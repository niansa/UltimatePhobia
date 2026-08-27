#include "whisper_voice.hpp"

#include "game_hook.hpp"
#include "global_instance_manager.hpp"
#include "il2cpp_api_cpp.hpp"
#include "misc_utils.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <imgui.h>
#include <spdlog/spdlog.h>

using namespace Il2Cpp::API;

namespace {

constexpr int kWhisperSampleRate = 16000;
constexpr std::size_t kVadFrameSamples = 160;       // 10 ms at 16 kHz
constexpr std::size_t kVadPreRollSamples = 3200;    // 200 ms
constexpr std::size_t kVadEndSilenceSamples = 6400; // 400 ms
constexpr std::size_t kMinimumSpeechSamples = 1280; // 80 ms of voiced audio
constexpr std::size_t kMaximumUtteranceSamples = 30 * kWhisperSampleRate;
constexpr std::size_t kMaximumQueuedAudioJobs = 2;
constexpr std::size_t kMaximumQueuedPhrases = 16;

std::string NormalizePhrase(std::string_view input) {
    std::string result;
    result.reserve(input.size());
    bool previousWasSpace = true;

    for (std::size_t i = 0; i < input.size(); ++i) {
        const unsigned char ch = input[i];

        if (ch == '\'')
            continue;

        if (ch == '&') {
            if (!previousWasSpace)
                result.push_back(' ');
            result.append("and ");
            previousWasSpace = true;
            continue;
        }

        if (ch >= 0x80) {
            result.push_back(static_cast<char>(ch));
            previousWasSpace = false;
            continue;
        }

        if (std::isalnum(ch)) {
            result.push_back(static_cast<char>(std::tolower(ch)));
            previousWasSpace = false;
        } else if (!previousWasSpace) {
            result.push_back(' ');
            previousWasSpace = true;
        }
    }

    while (!result.empty() && result.back() == ' ')
        result.pop_back();

    return result;
}

bool ContainsWholePhrase(std::string_view normalizedText, std::string_view normalizedPhrase) {
    if (normalizedText.empty() || normalizedPhrase.empty())
        return false;

    std::string paddedText;
    paddedText.reserve(normalizedText.size() + 2);
    paddedText.push_back(' ');
    paddedText.append(normalizedText);
    paddedText.push_back(' ');

    std::string paddedPhrase;
    paddedPhrase.reserve(normalizedPhrase.size() + 2);
    paddedPhrase.push_back(' ');
    paddedPhrase.append(normalizedPhrase);
    paddedPhrase.push_back(' ');

    return paddedText.find(paddedPhrase) != std::string::npos;
}

float CalculatePhraseSimilarity(const std::string& text, const std::string& phrase) {
    if (text == phrase)
        return 1.0f;
    if (ContainsWholePhrase(text, phrase))
        return 0.9f;

    // Split words and calculate token overlap
    auto splitWords = [](const std::string& str) {
        std::vector<std::string> words;
        std::string word;
        std::stringstream ss(str);
        while (ss >> word)
            words.push_back(word);
        return words;
    };

    auto textWords = splitWords(text);
    auto phraseWords = splitWords(phrase);

    if (phraseWords.empty())
        return 0.0f;

    std::size_t matches = 0;
    for (const auto& pw : phraseWords) {
        if (std::find(textWords.begin(), textWords.end(), pw) != textWords.end()) {
            matches++;
        }
    }

    return static_cast<float>(matches) / static_cast<float>(phraseWords.size());
}

std::vector<std::string> ExtractKeywordsFromArray(Il2CppObject *keywordsArrayPtr) {
    std::vector<std::string> keywords;

    if (!keywordsArrayPtr)
        return keywords;

    Array keywordArray{reinterpret_cast<Il2CppArray *>(keywordsArrayPtr)};

    const uint32_t count = keywordArray.length();
    keywords.reserve(count);

    for (uint32_t i = 0; i < count; ++i) {
        Object item = keywordArray.object_at(i);
        if (!item)
            continue;

        String managedString{reinterpret_cast<Il2CppString *>(item.ptr)};

        std::string normalized = NormalizePhrase(managedString.to_utf8());
        if (!normalized.empty())
            keywords.emplace_back(std::move(normalized));
    }

    return keywords;
}

std::vector<float> ResampleTo16k(const std::vector<float>& input, int sourceRate) {
    if (input.empty() || sourceRate <= 0)
        return {};

    if (sourceRate == kWhisperSampleRate)
        return input;

    const double sourcePerOutput = static_cast<double>(sourceRate) / static_cast<double>(kWhisperSampleRate);

    const std::size_t outputSize = static_cast<std::size_t>(static_cast<double>(input.size()) / sourcePerOutput);

    if (outputSize == 0)
        return {};

    std::vector<float> output(outputSize);

    for (std::size_t i = 0; i < outputSize; ++i) {
        const double sourceIndex = static_cast<double>(i) * sourcePerOutput;
        const std::size_t index0 = static_cast<std::size_t>(sourceIndex);
        const std::size_t index1 = (std::min)(index0 + 1, input.size() - 1);

        const double fraction = sourceIndex - static_cast<double>(index0);
        output[i] = static_cast<float>(static_cast<double>(input[index0]) * (1.0 - fraction) + static_cast<double>(input[index1]) * fraction);
    }

    return output;
}

bool DeviceNamesMatch(Object lhs, Il2CppObject *rhsPtr) {
    if (!lhs && !rhsPtr)
        return true;

    if (!lhs || !rhsPtr)
        return false;

    if (lhs.ptr == rhsPtr)
        return true;

    String lhsString{reinterpret_cast<Il2CppString *>(lhs.ptr)};
    String rhsString{reinterpret_cast<Il2CppString *>(rhsPtr)};
    return lhsString.to_utf8() == rhsString.to_utf8();
}

} // namespace

static void Hooked_KeywordRecognizer_ctor_1(Il2CppObject *__this, Il2CppObject *keywordsArrayPtr, const ::MethodInfo *method) {
    const auto self = whisperVoiceInfo.get<WhisperVoice>();
    const auto ctorMethod = get_method_cached<"UnityEngine.CoreModule", "UnityEngine.Windows.Speech", "KeywordRecognizer", ".ctor", 1>();

    if (auto hook = self->hooks.get(ctorMethod.function_pointer())) {
        GameHookRelease release(*hook);
        hook->getFunction<decltype(Hooked_KeywordRecognizer_ctor_1)>()(__this, keywordsArrayPtr, method);
    }

    self->RegisterRecognizer(__this, ExtractKeywordsFromArray(keywordsArrayPtr));
}

static void Hooked_KeywordRecognizer_ctor_2(Il2CppObject *__this, Il2CppObject *keywordsArrayPtr, int32_t minimumConfidence, const ::MethodInfo *method) {
    const auto self = whisperVoiceInfo.get<WhisperVoice>();
    const auto ctorMethod = get_method_cached<"UnityEngine.CoreModule", "UnityEngine.Windows.Speech", "KeywordRecognizer", ".ctor", 2>();

    if (auto hook = self->hooks.get(ctorMethod.function_pointer())) {
        GameHookRelease release(*hook);
        hook->getFunction<decltype(Hooked_KeywordRecognizer_ctor_2)>()(__this, keywordsArrayPtr, minimumConfidence, method);
    }

    self->RegisterRecognizer(__this, ExtractKeywordsFromArray(keywordsArrayPtr));
}

static void Hooked_add_OnPhraseRecognized(Il2CppObject *__this, Il2CppObject *delegateInstance, const ::MethodInfo *method) {
    const auto self = whisperVoiceInfo.get<WhisperVoice>();
    const auto addMethod = get_method_cached<"UnityEngine.CoreModule", "UnityEngine.Windows.Speech", "PhraseRecognizer", "add_OnPhraseRecognized", 1>();

    if (auto hook = self->hooks.get(addMethod.function_pointer())) {
        GameHookRelease release(*hook);
        hook->getFunction<decltype(Hooked_add_OnPhraseRecognized)>()(__this, delegateInstance, method);
    }

    self->AddRecognizerDelegate(__this, delegateInstance);
}

static void Hooked_remove_OnPhraseRecognized(Il2CppObject *__this, Il2CppObject *delegateInstance, const ::MethodInfo *method) {
    const auto self = whisperVoiceInfo.get<WhisperVoice>();
    const auto removeMethod = get_method_cached<"UnityEngine.CoreModule", "UnityEngine.Windows.Speech", "PhraseRecognizer", "remove_OnPhraseRecognized", 1>();

    if (auto hook = self->hooks.get(removeMethod.function_pointer())) {
        GameHookRelease release(*hook);
        hook->getFunction<decltype(Hooked_remove_OnPhraseRecognized)>()(__this, delegateInstance, method);
    }

    self->RemoveRecognizerDelegate(__this, delegateInstance);
}

static Il2CppObject *Hooked_Microphone_Start(Il2CppObject *deviceName, bool loop, int32_t lengthSec, int32_t frequency, const ::MethodInfo *method) {
    const auto self = whisperVoiceInfo.get<WhisperVoice>();
    const auto startMethod = get_method_cached<"UnityEngine.AudioModule", "UnityEngine", "Microphone", "Start", 4>();

    Il2CppObject *clip = nullptr;

    if (auto hook = self->hooks.get(startMethod.function_pointer())) {
        GameHookRelease release(*hook);
        clip = hook->getFunction<decltype(Hooked_Microphone_Start)>()(deviceName, loop, lengthSec, frequency, method);
    }

    self->BeginMicrophoneCapture(clip, deviceName);
    return clip;
}

static void Hooked_Microphone_End(Il2CppObject *deviceName, const ::MethodInfo *method) {
    const auto self = whisperVoiceInfo.get<WhisperVoice>();
    const auto endMethod = get_method_cached<"UnityEngine.AudioModule", "UnityEngine", "Microphone", "End", 1>();

    if (auto hook = self->hooks.get(endMethod.function_pointer())) {
        GameHookRelease release(*hook);
        hook->getFunction<decltype(Hooked_Microphone_End)>()(deviceName, method);
    }

    self->EndMicrophoneCapture(deviceName);
}

WhisperVoice::WhisperVoice() {
    whisper_context_params contextParams = whisper_context_default_params();

    wctx = whisper_init_from_file_with_params("whisper.bin", contextParams);
    if (!wctx)
        g.logger->error("WhisperVoice: failed to load whisper.bin");

    fvad = fvad_new();
    if (fvad) {
        if (fvad_set_sample_rate(fvad, kWhisperSampleRate) != 0 || fvad_set_mode(fvad, 3) != 0) {

            g.logger->error("WhisperVoice: failed to configure FVAD");
            fvad_free(fvad);
            fvad = nullptr;
        }
    } else {
        g.logger->error("WhisperVoice: failed to create FVAD instance");
    }

    const auto ctorOneMethod = get_method_cached<"UnityEngine.CoreModule", "UnityEngine.Windows.Speech", "KeywordRecognizer", ".ctor", 1>();
    if (ctorOneMethod && ctorOneMethod.function_pointer())
        hooks.add(ctorOneMethod.function_pointer(), reinterpret_cast<void *>(Hooked_KeywordRecognizer_ctor_1));

    const auto ctorTwoMethod = get_method_cached<"UnityEngine.CoreModule", "UnityEngine.Windows.Speech", "KeywordRecognizer", ".ctor", 2>();
    if (ctorTwoMethod && ctorTwoMethod.function_pointer())
        hooks.add(ctorTwoMethod.function_pointer(), reinterpret_cast<void *>(Hooked_KeywordRecognizer_ctor_2));

    const auto addPhraseMethod = get_method_cached<"UnityEngine.CoreModule", "UnityEngine.Windows.Speech", "PhraseRecognizer", "add_OnPhraseRecognized", 1>();
    if (addPhraseMethod && addPhraseMethod.function_pointer())
        hooks.add(addPhraseMethod.function_pointer(), reinterpret_cast<void *>(Hooked_add_OnPhraseRecognized));

    const auto removePhraseMethod =
        get_method_cached<"UnityEngine.CoreModule", "UnityEngine.Windows.Speech", "PhraseRecognizer", "remove_OnPhraseRecognized", 1>();
    if (removePhraseMethod && removePhraseMethod.function_pointer())
        hooks.add(removePhraseMethod.function_pointer(), reinterpret_cast<void *>(Hooked_remove_OnPhraseRecognized));

    const auto microphoneStartMethod = get_method_cached<"UnityEngine.AudioModule", "UnityEngine", "Microphone", "Start", 4>();
    if (microphoneStartMethod && microphoneStartMethod.function_pointer())
        hooks.add(microphoneStartMethod.function_pointer(), reinterpret_cast<void *>(Hooked_Microphone_Start));

    const auto microphoneEndMethod = get_method_cached<"UnityEngine.AudioModule", "UnityEngine", "Microphone", "End", 1>();
    if (microphoneEndMethod && microphoneEndMethod.function_pointer())
        hooks.add(microphoneEndMethod.function_pointer(), reinterpret_cast<void *>(Hooked_Microphone_End));

    workerThread = std::thread(&WhisperVoice::WorkerThreadLoop, this);
}

WhisperVoice::~WhisperVoice() {
    stopWorker.store(true, std::memory_order_release);
    jobCv.notify_all();

    if (workerThread.joinable())
        workerThread.join();

    {
        std::lock_guard lock(recognizersMutex);
        recognizers.clear();
    }

    activeMicrophoneClip = {};
    activeMicrophoneDevice = {};

    if (fvad) {
        fvad_free(fvad);
        fvad = nullptr;
    }

    if (wctx) {
        whisper_free(wctx);
        wctx = nullptr;
    }
}

void WhisperVoice::RegisterRecognizer(Il2CppObject *recognizer, const std::vector<std::string>& keywords) {
    if (!recognizer)
        return;

    for (const auto& keyword : keywords)
        g.logger->info("Added global dynamic keyword: {}", keyword);

    {
        std::lock_guard lock(keywordsMutex);

        for (const std::string& keyword : keywords)
            if (!keyword.empty())
                dynamicGameKeywords.insert(keyword);
    }

    std::lock_guard lock(recognizersMutex);

    RecognizerRegistration& registration = recognizers[recognizer];

    if (!registration.recognizer) {
        registration.recognizer = Object{recognizer};
        registration.recognizer.lock_gc();
    }

    registration.keywords.clear();

    for (const std::string& keyword : keywords)
        if (!keyword.empty())
            registration.keywords.insert(keyword);
}

void WhisperVoice::AddRecognizerDelegate(Il2CppObject *recognizer, Il2CppObject *delegateInstance) {
    if (!recognizer || !delegateInstance)
        return;

    std::lock_guard lock(recognizersMutex);

    RecognizerRegistration& registration = recognizers[recognizer];

    if (!registration.recognizer) {
        registration.recognizer = Object{recognizer};
        registration.recognizer.lock_gc();
    }

    Object delegateObject{delegateInstance};
    delegateObject.lock_gc();

    registration.delegates.emplace_back(std::move(delegateObject));
}

void WhisperVoice::RemoveRecognizerDelegate(Il2CppObject *recognizer, Il2CppObject *delegateInstance) {
    if (!recognizer || !delegateInstance)
        return;

    std::lock_guard lock(recognizersMutex);

    const auto registrationIt = recognizers.find(recognizer);
    if (registrationIt == recognizers.end())
        return;

    auto& delegates = registrationIt->second.delegates;

    for (auto it = delegates.rbegin(); it != delegates.rend(); ++it) {
        if (it->ptr == delegateInstance) {
            delegates.erase(std::next(it).base());
            break;
        }
    }
}

void WhisperVoice::BeginMicrophoneCapture(Il2CppObject *clip, Il2CppObject *deviceName) {
    if (isSpeaking)
        SubmitCurrentUtterance();

    activeMicrophoneClip = {};
    activeMicrophoneDevice = {};
    ResetAudioState();

    if (!clip)
        return;

    activeMicrophoneClip = Object{clip};
    activeMicrophoneClip.lock_gc();

    if (deviceName) {
        activeMicrophoneDevice = Object{deviceName};
        activeMicrophoneDevice.lock_gc();
    }
}

void WhisperVoice::EndMicrophoneCapture(Il2CppObject *deviceName) {
    if (!DeviceNamesMatch(activeMicrophoneDevice, deviceName))
        return;

    if (isSpeaking)
        SubmitCurrentUtterance();

    activeMicrophoneClip = {};
    activeMicrophoneDevice = {};
    ResetAudioState();
}

void WhisperVoice::ResetAudioState() {
    lastMicPosition = 0;
    isSpeaking = false;

    utteranceSilenceSamples = 0;
    utteranceSpeechSamples = 0;

    vadInputBuffer.clear();
    preRollBuffer.clear();
    accumulationBuffer.clear();
}

void WhisperVoice::SubmitCurrentUtterance() {
    isSpeaking = false;
    utteranceSilenceSamples = 0;

    const bool hasEnoughSpeech = utteranceSpeechSamples >= kMinimumSpeechSamples;

    utteranceSpeechSamples = 0;

    if (!hasEnoughSpeech || accumulationBuffer.empty()) {
        accumulationBuffer.clear();
        return;
    }

    std::vector<float> completedUtterance = std::move(accumulationBuffer);

    accumulationBuffer.clear();
    {
        std::lock_guard lock(jobMutex);

        while (audioJobQueue.size() >= kMaximumQueuedAudioJobs)
            audioJobQueue.pop();

        audioJobQueue.emplace(std::move(completedUtterance));
    }
    jobCv.notify_one();
}

void WhisperVoice::ProcessVadSamples(const std::vector<float>& samples) {
    if (!fvad || samples.empty())
        return;

    vadInputBuffer.insert(vadInputBuffer.end(), samples.begin(), samples.end());

    std::size_t processedSamples = 0;

    while (vadInputBuffer.size() - processedSamples >= kVadFrameSamples) {
        const float *frame = vadInputBuffer.data() + processedSamples;

        int16_t pcm16[kVadFrameSamples];

        for (std::size_t i = 0; i < kVadFrameSamples; ++i) {
            float sample = frame[i];

            if (!std::isfinite(sample))
                sample = 0.0f;

            sample = std::clamp(sample, -1.0f, 1.0f);
            pcm16[i] = static_cast<int16_t>(sample * 32767.0f);
        }

        const bool frameHasSpeech = fvad_process(fvad, pcm16, static_cast<int>(kVadFrameSamples)) == 1;

        if (!isSpeaking) {
            preRollBuffer.insert(preRollBuffer.end(), frame, frame + kVadFrameSamples);

            if (preRollBuffer.size() > kVadPreRollSamples) {
                const std::size_t excess = preRollBuffer.size() - kVadPreRollSamples;
                preRollBuffer.erase(preRollBuffer.begin(), preRollBuffer.begin() + static_cast<std::ptrdiff_t>(excess));
            }

            if (frameHasSpeech) {
                isSpeaking = true;
                utteranceSilenceSamples = 0;
                utteranceSpeechSamples = kVadFrameSamples;

                accumulationBuffer = std::move(preRollBuffer);

                preRollBuffer.clear();
            }
        } else {
            accumulationBuffer.insert(accumulationBuffer.end(), frame, frame + kVadFrameSamples);

            if (frameHasSpeech) {
                utteranceSilenceSamples = 0;
                utteranceSpeechSamples += kVadFrameSamples;
            } else {
                utteranceSilenceSamples += kVadFrameSamples;
            }

            if (utteranceSilenceSamples >= kVadEndSilenceSamples || accumulationBuffer.size() >= kMaximumUtteranceSamples)
                SubmitCurrentUtterance();
        }

        processedSamples += kVadFrameSamples;
    }

    if (processedSamples != 0) {
        vadInputBuffer.erase(vadInputBuffer.begin(), vadInputBuffer.begin() + static_cast<std::ptrdiff_t>(processedSamples));
    }
}

void WhisperVoice::WorkerThreadLoop() {
    for (;;) {
        std::vector<float> audioData;

        {
            std::unique_lock lock(jobMutex);

            jobCv.wait(lock, [this] { return stopWorker.load(std::memory_order_acquire) || !audioJobQueue.empty(); });

            if (stopWorker.load(std::memory_order_acquire))
                break;

            audioData = std::move(audioJobQueue.front());
            audioJobQueue.pop();
        }

        if (audioData.empty() || !wctx)
            continue;

        float maxPeak = 0.0f;
        for (const float sample : audioData)
            maxPeak = (std::max)(maxPeak, std::abs(sample));

        if (maxPeak > 0.001f && maxPeak < 0.85f) {
            const float scaleFactor = (std::min)(0.85f / maxPeak, 15.0f);
            for (float& sample : audioData)
                sample = std::clamp(sample * scaleFactor, -1.0f, 1.0f);
        }

        whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

        params.language = "en";
        params.no_timestamps = true;
        params.single_segment = true;
        params.print_special = false;
        params.print_progress = false;

        const unsigned int hardwareThreads = std::thread::hardware_concurrency();
        const int availableThreads = hardwareThreads > 2 ? static_cast<int>(hardwareThreads) - 2 : 1;
        params.n_threads = std::clamp(availableThreads, 1, 8);

        const int result = whisper_full(wctx, params, audioData.data(), static_cast<int>(audioData.size()));

        if (result != 0) {
            g.logger->warn("WhisperVoice: whisper_full failed with code {}", result);
            continue;
        }

        std::string transcribedText;

        const int segmentCount = whisper_full_n_segments(wctx);

        for (int i = 0; i < segmentCount; ++i) {
            const char *segment = whisper_full_get_segment_text(wctx, i);

            if (segment)
                transcribedText.append(segment);
        }

        const std::string normalizedText = NormalizePhrase(transcribedText);
        g.logger->info("Recognized text: {}", normalizedText);

        {
            std::lock_guard lock(phraseMutex);
            lastRawRecognizedText = normalizedText;
        }

        if (normalizedText.empty())
            continue;

        std::string matchedPhrase;
        float highestScore = 0.0f;

        {
            std::lock_guard lock(keywordsMutex);
            for (const std::string& phrase : dynamicGameKeywords) {
                float score = CalculatePhraseSimilarity(normalizedText, phrase);

                // Require at least 75% word overlap match
                if (score >= 0.75f && score > highestScore) {
                    highestScore = score;
                    matchedPhrase = phrase;
                }
            }
        }

        if (matchedPhrase.empty())
            continue;

        {
            std::lock_guard lock(phraseMutex);

            while (pendingPhrasesQueue.size() >= kMaximumQueuedPhrases)
                pendingPhrasesQueue.pop();

            pendingPhrasesQueue.emplace(std::move(matchedPhrase));
        }
    }
}

void WhisperVoice::uiUpdate() {
    std::queue<std::string> phrasesToFire;

    {
        std::lock_guard lock(phraseMutex);
        phrasesToFire.swap(pendingPhrasesQueue);
    }

    while (!phrasesToFire.empty()) {
        FireGameDelegates(phrasesToFire.front());
        phrasesToFire.pop();
    }

    std::size_t keywordCount = 0;
    std::size_t recognizerCount = 0;
    std::size_t queuedAudioJobs = 0;

    {
        std::lock_guard lock(keywordsMutex);
        keywordCount = dynamicGameKeywords.size();
    }
    {
        std::lock_guard lock(recognizersMutex);
        recognizerCount = recognizers.size();
    }
    {
        std::lock_guard lock(jobMutex);
        queuedAudioJobs = audioJobQueue.size();
    }

    ImGui::Begin("Whisper Voice");
    ImGui::Text("Whisper Engine: %s", wctx ? "Active" : "Failed");
    ImGui::Text("VAD Engine: %s", fvad ? "Active" : "Failed");
    ImGui::Text("Microphone Hooked: %s", activeMicrophoneClip ? "Yes" : "No");
    ImGui::Text("Voice Activity: %s", isSpeaking ? "Speaking" : "Idle");
    ImGui::Text("Recognizers: %zu", recognizerCount);
    ImGui::Text("Keywords: %zu", keywordCount);
    ImGui::Text("Queued Audio Jobs: %zu", queuedAudioJobs);
    ImGui::Text("Last recognized: %s", lastRawRecognizedText.c_str());
    ImGui::End();

    if (!activeMicrophoneClip || !wctx || !fvad)
        return;

    try {
        ProcessAudioAndRecognize();
    } catch (const ManagedException& exception) {
        g.logger->warn("WhisperVoice: managed audio-processing failure: {}", exception.what());
    } catch (const Error& exception) {
        g.logger->warn("WhisperVoice: IL2CPP audio-processing failure: {}", exception.what());
    } catch (const std::exception& exception) {
        g.logger->warn("WhisperVoice: audio-processing failure: {}", exception.what());
    }
}

void WhisperVoice::ProcessAudioAndRecognize() {
    const Class microphoneClass = get_class_cached<"UnityEngine.AudioModule", "UnityEngine", "Microphone">();

    const Method getPositionMethod = microphoneClass.get_method("GetPosition", 1);

    if (!getPositionMethod)
        return;

    void *microphoneArgs[] = {activeMicrophoneDevice.ptr};

    int currentPosition = invoke_unbox<int>(getPositionMethod, Object{}, microphoneArgs);

    if (currentPosition < 0 || currentPosition == lastMicPosition)
        return;

    const Class audioClipClass = get_class_cached<"UnityEngine.AudioModule", "UnityEngine", "AudioClip">();
    const Method getFrequencyMethod = audioClipClass.get_method("get_frequency", 0);
    const Method getSamplesMethod = audioClipClass.get_method("get_samples", 0);
    const Method getChannelsMethod = audioClipClass.get_method("get_channels", 0);

    if (!getFrequencyMethod || !getSamplesMethod || !getChannelsMethod)
        return;

    const int nativeFrequency = invoke_unbox<int>(getFrequencyMethod, activeMicrophoneClip, {});
    const int clipSampleFrames = invoke_unbox<int>(getSamplesMethod, activeMicrophoneClip, {});
    const int channelCount = invoke_unbox<int>(getChannelsMethod, activeMicrophoneClip, {});

    if (nativeFrequency <= 0 || clipSampleFrames <= 0 || channelCount <= 0)
        return;

    currentPosition %= clipSampleFrames;

    if (lastMicPosition < 0 || lastMicPosition >= clipSampleFrames) {
        lastMicPosition = currentPosition;
        return;
    }

    const int sampleFramesToRead = currentPosition > lastMicPosition ? currentPosition - lastMicPosition : clipSampleFrames - lastMicPosition + currentPosition;

    // Wait until at least 10 ms is available
    if (sampleFramesToRead < nativeFrequency / 100)
        return;

    const std::size_t valueCount = static_cast<std::size_t>(sampleFramesToRead) * static_cast<std::size_t>(channelCount);

    if (valueCount == 0 || valueCount > static_cast<std::size_t>(std::numeric_limits<uint32_t>::max()))
        return;

    const Class floatClass = get_class_cached<"mscorlib", "System", "Single">();
    Array managedArray = Array::New(floatClass, valueCount);

    Object getDataResult = call(activeMicrophoneClip, "GetData", managedArray, lastMicPosition);

    if (getDataResult) {
        const bool *success = object_unbox<bool>(getDataResult);

        if (success && !*success)
            return;
    }

    if (managedArray.byte_length() < valueCount * sizeof(float)) {
        g.logger->warn("WhisperVoice: AudioClip.GetData returned a short array");
        return;
    }

    Il2CppObject *rawArrayObj = managedArray.object().ptr;
    size_t headerOffset = il2cpp_object_get_size(rawArrayObj) - il2cpp_array_get_byte_length(managedArray.ptr);
    const float *rawSamples = reinterpret_cast<const float *>(reinterpret_cast<const uint8_t *>(rawArrayObj) + headerOffset);

    if (!rawSamples)
        return;

    lastMicPosition = currentPosition;

    std::vector<float> monoPcm(static_cast<std::size_t>(sampleFramesToRead));

    if (channelCount == 1) {
        std::copy_n(rawSamples, monoPcm.size(), monoPcm.begin());
    } else {
        for (int frame = 0; frame < sampleFramesToRead; ++frame) {
            double sum = 0.0;

            const std::size_t frameOffset = static_cast<std::size_t>(frame) * static_cast<std::size_t>(channelCount);

            for (int channel = 0; channel < channelCount; ++channel) {
                const float sample = rawSamples[frameOffset + static_cast<std::size_t>(channel)];
                if (std::isfinite(sample))
                    sum += sample;
            }

            monoPcm[static_cast<std::size_t>(frame)] = static_cast<float>(sum / static_cast<double>(channelCount));
        }
    }

    std::vector<float> pcm16k = ResampleTo16k(monoPcm, nativeFrequency);

    if (!pcm16k.empty())
        ProcessVadSamples(pcm16k);
}

void WhisperVoice::FireGameDelegates(std::string_view recognizedText) {
    g.logger->info("Matched text: {}", recognizedText);

    const std::string normalizedPhrase = NormalizePhrase(recognizedText);

    if (normalizedPhrase.empty())
        return;

    std::vector<Object> delegatesToInvoke;

    {
        std::lock_guard lock(recognizersMutex);
        for (const auto& [recognizer, registration] : recognizers) {
            (void)recognizer;

            if (!registration.keywords.contains(normalizedPhrase))
                continue;

            delegatesToInvoke.insert(delegatesToInvoke.end(), registration.delegates.begin(), registration.delegates.end());
        }
    }

    if (delegatesToInvoke.empty())
        return;

    const Class argsClass = get_class_cached<"UnityEngine.CoreModule", "UnityEngine.Windows.Speech", "PhraseRecognizedEventArgs">();
    if (!argsClass)
        return;

    Object spoofedArgs = object_new(argsClass);
    if (!spoofedArgs)
        return;

    const Field textField = argsClass.get_field("text");
    const Field confidenceField = argsClass.get_field("confidence");
    const Field semanticMeaningsField = argsClass.get_field("semanticMeanings");

    if (textField) {
        textField.set_value_object(spoofedArgs, make_string(normalizedPhrase));
    }

    if (confidenceField) {
        // UnityEngine.Windows.Speech.ConfidenceLevel.High == 0
        confidenceField.set_value<int32_t>(spoofedArgs, 0);
    }

    if (semanticMeaningsField) {
        const Class semanticMeaningClass = get_class_cached<"UnityEngine.CoreModule", "UnityEngine.Windows.Speech", "SemanticMeaning">();

        if (semanticMeaningClass) {
            Array emptyMeanings = Array::New(semanticMeaningClass, static_cast<std::size_t>(0));
            semanticMeaningsField.set_value_object(spoofedArgs, emptyMeanings.object());
        }
    }

    // Invoke a snapshot. Event handlers are allowed to subscribe or unsubscribe while they are running
    for (Object& delegateObject : delegatesToInvoke) {
        if (!delegateObject)
            continue;

        try {
            call(delegateObject, "Invoke", spoofedArgs);
        } catch (const ManagedException& exception) {
            g.logger->warn("WhisperVoice: delegate execution failure: {}", exception.what());
        } catch (const Error& exception) {
            g.logger->warn("WhisperVoice: delegate invocation failure: {}", exception.what());
        }
    }
}

ModInfo whisperVoiceInfo{"WhisperVoice", false, []() { return std::make_unique<WhisperVoice>(); }};
