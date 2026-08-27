#pragma once

#include "mods/base.hpp"
#include "game_hook.hpp"
#include "il2cpp_api_cpp.hpp"
#include "whisper.h"

#include <fvad.h>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class WhisperVoice final : public Mod {
public:
    struct RecognizerRegistration {
        Il2Cpp::API::Object recognizer;
        std::unordered_set<std::string> keywords;
        std::vector<Il2Cpp::API::Object> delegates;
    };

    GameHookPool hooks;

    whisper_context *wctx = nullptr;
    Fvad *fvad = nullptr;

    Il2Cpp::API::Object activeMicrophoneClip;
    Il2Cpp::API::Object activeMicrophoneDevice;

    int lastMicPosition = 0;
    bool isSpeaking = false;

    // Main-thread VAD/audio state.
    std::vector<float> vadInputBuffer;
    std::vector<float> preRollBuffer;
    std::vector<float> accumulationBuffer;
    std::size_t utteranceSilenceSamples = 0;
    std::size_t utteranceSpeechSamples = 0;

    // Recognizers and delegates may be touched by hooks.
    std::mutex recognizersMutex;
    std::unordered_map<Il2CppObject *, RecognizerRegistration> recognizers;

    // Native keyword data is also read by the Whisper worker.
    std::mutex keywordsMutex;
    std::unordered_set<std::string> dynamicGameKeywords;

    // Background speech-recognition state.
    std::thread workerThread;
    std::mutex jobMutex;
    std::condition_variable jobCv;
    std::queue<std::vector<float>> audioJobQueue;
    std::atomic<bool> stopWorker{false};

    // Recognized phrases are consumed on Unity's main thread.
    std::mutex phraseMutex;
    std::queue<std::string> pendingPhrasesQueue;
    std::string lastRawRecognizedText = "None";

    WhisperVoice();
    ~WhisperVoice() override;

    void uiUpdate() override;

    void ProcessAudioAndRecognize();
    void ProcessVadSamples(const std::vector<float>& samples);
    void SubmitCurrentUtterance();
    void ResetAudioState();

    void BeginMicrophoneCapture(Il2CppObject *clip, Il2CppObject *deviceName);
    void EndMicrophoneCapture(Il2CppObject *deviceName);

    void RegisterRecognizer(Il2CppObject *recognizer, const std::vector<std::string>& keywords);

    void AddRecognizerDelegate(Il2CppObject *recognizer, Il2CppObject *delegateInstance);

    void RemoveRecognizerDelegate(Il2CppObject *recognizer, Il2CppObject *delegateInstance);

    void FireGameDelegates(std::string_view recognizedText);
    void WorkerThreadLoop();
};

extern ModInfo whisperVoiceInfo;
