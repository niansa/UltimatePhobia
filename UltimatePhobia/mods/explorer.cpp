#include "explorer.hpp"

#include <unordered_map>
#include <memory>
#include <algorithm>
#include <charconv>
#include <limits>
#include <sstream>
#include <cstring>
#include <imgui.h>

using namespace Il2Cpp::API;

static inline bool icontains(std::string_view hay, std::string_view needle) {
    auto tolower = [](unsigned char c) { return static_cast<char>(::tolower(c)); };
    auto it = std::search(hay.begin(), hay.end(), needle.begin(), needle.end(), [&](char a, char b) { return tolower(a) == tolower(b); });
    return it != hay.end();
}

std::string Explorer::InstanceEntry::getLabel() const {
    return fullClassName(Object{handle.target()}.klass()) + " -> " + objectToString(Object{handle.target()});
}

Explorer::Explorer() {
    corlib_ = Image::get_corlib();
    sysBoolean_ = class_from_name(corlib_, "System", "Boolean");
    sysInt32_ = class_from_name(corlib_, "System", "Int32");
    sysInt64_ = class_from_name(corlib_, "System", "Int64");
    sysDouble_ = class_from_name(corlib_, "System", "Double");
    sysSingle_ = class_from_name(corlib_, "System", "Single");
    sysString_ = class_from_name(corlib_, "System", "String");
}

void Explorer::uiUpdate() { drawWindow(); }

void Explorer::refreshAssemblies() {
    assemblies_.clear();
    auto d = Domain::get();
    assemblies_ = d.get_assemblies();
}

void Explorer::rebuildClassList() {
    classList_.clear();
    selectedClass_ = -1;
    selectedMethod_ = Method{};
    selectedProperty_ = Property{};
    callState_ = CallState{};

    if (selectedAssembly_ < 0)
        return;
    if (selectedAssembly_ >= static_cast<int>(assemblies_.size())) {
        selectedAssembly_ = -1;
        return;
    }

    auto img = assemblies_[selectedAssembly_].image();
    const auto classCount = img.class_count();
    classList_.reserve(classCount);
    for (size_t i = 0; i < classCount; ++i) {
        Class c = img.get_class(i);
        if (!c)
            continue;
        ClassEntry e;
        e.klass = c;
        e.fullname = fullClassName(c);
        classList_.push_back(std::move(e));
    }

    std::sort(classList_.begin(), classList_.end(), [](const ClassEntry& a, const ClassEntry& b) { return a.fullname < b.fullname; });
}

void Explorer::drawWindow() {
    if (!open_)
        return;

    if (assemblies_.empty())
        refreshAssemblies();

    if (!ImGui::Begin("IL2CPP Explorer", &open_)) {
        ImGui::End();
        return;
    }

    // Layout: 3 panes horizontally
    ImGui::BeginChild("AssembliesPane", ImVec2(260, 0), true);
    drawAssembliesPane();
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("ClassesPane", ImVec2(360, 0), true);
    drawClassesPane();
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("DetailsPane", ImVec2(0, 0), true);
    drawClassDetailPane();
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::BeginChild("InstancesPane", ImVec2(0, 180), true);
    drawInstancesPane();
    ImGui::EndChild();

    ImGui::End();
}

void Explorer::drawAssembliesPane() {
    ImGui::Text("Assemblies");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##AsmFilter", asmFilter_, sizeof(asmFilter_));

    if (ImGui::Button("Refresh")) {
        refreshAssemblies();
        rebuildClassList();
    }

    ImGui::Separator();
    if (ImGui::BeginListBox("##AsmList", ImVec2(-1, -1))) {
        for (int i = 0; i < static_cast<int>(assemblies_.size()); ++i) {
            ImGui::PushID(i);

            auto img = assemblies_[i].image();
            std::string name = std::string(img.name());
            std::string file = std::string(img.filename());
            if (asmFilter_[0] != '\0') {
                if (!icontains(name, asmFilter_) && !icontains(file, asmFilter_))
                    continue;
            }
            bool sel = (i == selectedAssembly_);
            if (ImGui::Selectable(name.c_str(), sel)) {
                selectedAssembly_ = i;
                rebuildClassList();
            }

            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
}

void Explorer::drawClassesPane() {
    ImGui::Text("Classes");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##ClassFilter", classFilter_, sizeof(classFilter_));

    ImGui::Separator();
    if (ImGui::BeginListBox("##ClassList", ImVec2(-1, -1))) {
        for (int i = 0; i < static_cast<int>(classList_.size()); ++i) {
            ImGui::PushID(i);

            const auto& e = classList_[i];
            if (classFilter_[0] != '\0') {
                if (!icontains(e.fullname, classFilter_)) {
                    ImGui::PopID();
                    continue;
                }
            }
            bool sel = (i == selectedClass_);
            if (ImGui::Selectable(e.fullname.c_str(), sel)) {
                selectedClass_ = i;
                selectedMethod_ = Method{};
                callState_ = CallState{};
            }

            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
}

static inline const char *yesno(bool v) { return v ? "Yes" : "No"; }

void Explorer::drawClassDetailPane() {
    ImGui::Text("Details");
    ImGui::Separator();

    if (selectedClass_ < 0 || selectedClass_ >= static_cast<int>(classList_.size())) {
        ImGui::TextUnformatted("Select a class to inspect.");
        return;
    }

    Class c = classList_[selectedClass_].klass;

    ImGui::Text("Name: %s", c.name().data());
    ImGui::Text("Namespace: %s", c.namespaze().data());
    ImGui::Text("Image: %s", c.image().name().data());
    ImGui::Text("Is ValueType: %s  Is Interface: %s  Is Abstract: %s  Is Enum: %s", yesno(c.is_value_type()), yesno(c.is_interface()), yesno(c.is_abstract()),
                yesno(c.is_enum()));
    if (auto p = c.parent()) {
        std::string pfn = fullClassName(p);
        ImGui::Text("Parent: %s", pfn.c_str());
    }
    ImGui::Separator();

    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("Filter Members", memberFilter_, sizeof(memberFilter_));

    if (ImGui::CollapsingHeader("Fields", 0)) {
        auto fields = c.fields();
        for (auto& f : fields) {
            auto ft = typeName(f.type());
            std::string line = ft + " " + std::string(f.name());
            if (memberFilter_[0] != '\0' && !icontains(line, memberFilter_))
                continue;

            ImGui::PushID(std::string(f.name()).c_str());
            ImGui::TextUnformatted(line.c_str());

            // Try read value (static only here; instance via Instances pane)
            Il2CppObject *fv = nullptr;
            if (selectedInstance_ >= 0 && instances_[selectedInstance_].handle.target()->klass == c.ptr)
                fv = f.get_value_object(instances_[selectedInstance_].handle.target());
            try {
                std::string s = fv ? objectToString(Object{fv}) : std::string("null");
                s = fmt::format("{} = {}{}", f.name().data(), s, (f.flags() & 0x0010) ? " [static]" : "");
                ImGui::SameLine();
                ImGui::TextUnformatted(s.c_str());
            } catch (const ManagedException& e) {
                g.logger->error("Field get threw: {}", e.what());
            }

            ImGui::PopID();
        }
    }

    if (ImGui::CollapsingHeader("Properties", 0)) {
        for (auto& p : c.properties()) {
            if (memberFilter_[0] != '\0' && !icontains(p.name(), memberFilter_))
                continue;

            ImGui::PushID(std::string(p.name()).c_str());

            bool sel = (p.ptr == selectedProperty_.ptr);
            if (ImGui::Selectable(std::string(p.name()).c_str(), sel))
                setSelectedProperty(p);

            ImGui::PopID();
        }
    }

    if (ImGui::CollapsingHeader("Methods", 0)) {
        for (auto& m : c.methods()) {
            if (memberFilter_[0] != '\0' && !icontains(m.name(), memberFilter_))
                continue;

            std::string sig = methodSignature(m);
            ImGui::PushID(sig.c_str());

            bool sel = (m.ptr == selectedMethod_.ptr);
            if (ImGui::Selectable(sig.c_str(), sel))
                setSelectedMethod(m);

            ImGui::PopID();
        }
    }

    if (ImGui::Button("Create instance (default .ctor)"))
        createDefaultInstance();

    ImGui::SeparatorText("Invoke");
    if (selectedMethod_) {
        // Instance selection for instance methods
        if (selectedMethod_.is_instance()) {
            ImGui::TextUnformatted("This method is an instance method.");
            ImGui::Text("Selected instance: %s", (selectedInstance_ >= 0 && selectedInstance_ < static_cast<int>(instances_.size()))
                                                     ? instances_[selectedInstance_].getLabel().c_str()
                                                     : "(none)");
            ImGui::SameLine();
            if (ImGui::Button("Clear selection"))
                selectedInstance_ = -1;
        }

        // Argument inputs
        uint32_t argc = selectedMethod_.param_count();
        while (callState_.argTexts.size() < argc)
            callState_.argTexts.emplace_back();

        for (uint32_t i = 0; i < argc; ++i) {
            ImGui::PushID(i);

            auto pt = selectedMethod_.param(i);
            auto pname = selectedMethod_.param_name(i);
            std::string label = std::string(pname) + " : " + typeName(pt);

            std::array<char, 512> input{};
            std::strncpy(input.data(), callState_.argTexts[i].c_str(), input.size() - 1);

            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText(label.c_str(), input.data(), input.size()))
                callState_.argTexts[i] = input.data();

            ImGui::PopID();
        }

        if (ImGui::Button("Invoke"))
            invokeSelectedMethod();
        if (!callState_.lastError.empty())
            ImGui::Text("Last error: %s", callState_.lastError.c_str());
        else if (!callState_.lastReturn.empty())
            ImGui::Text("Return: %s", callState_.lastReturn.c_str());
    } else {
        ImGui::TextUnformatted("Select a method to prepare invocation.");
    }

    ImGui::SeparatorText("Inspect Property");
    if (selectedProperty_) {
        ImGui::Text("Type: %s", selectedProperty_.getter().return_type().name_owned().c_str());
        if (auto m = selectedProperty_.getter())
            if (ImGui::Button("Select getter"))
                selectedMethod_ = m;

        if (auto m = selectedProperty_.setter())
            if (ImGui::Button("Select setter"))
                selectedMethod_ = m;
    } else {
        ImGui::TextUnformatted("Select a property to inspect.");
    }
}

void Explorer::drawInstancesPane() {
    ImGui::TextUnformatted("Instances (created here)");
    ImGui::Separator();

    // List instances
    for (int i = 0; i < static_cast<int>(instances_.size()); ++i) {
        ImGui::PushID(i);

        auto& e = instances_[i];
        bool selected = (i == selectedInstance_);
        if (ImGui::Selectable(e.getLabel().c_str(), selected)) {
            selectedInstance_ = i;
        }
        ImGui::SameLine();
        if (ImGui::Button("Release")) {
            instances_.erase(instances_.begin() + i);
            if (selectedInstance_ == i)
                selectedInstance_ = -1;
            if (selectedInstance_ > i)
                selectedInstance_--;
            --i;
        }

        ImGui::PopID();
    }
}

std::string Explorer::fullClassName(const Class& c) {
    if (!c)
        return "<null>";

    auto ns = c.namespaze();
    auto n = c.name();
    if (ns.empty())
        return std::string(n);
    std::string out;
    out.reserve(ns.size() + n.size() + 1);
    out.append(ns);
    out.push_back('.');
    out.append(n);
    return out;
}

std::string Explorer::typeName(const Type& t) {
    if (!t)
        return std::string("<null>");
    return t.name_owned();
}

std::string Explorer::methodSignature(const Method& m) {
    std::ostringstream oss;
    auto decl = m.declaring_type();
    oss << (decl.namespaze().empty() ? "" : std::string(decl.namespaze()) + ".") << decl.name() << "$$" << m.name() << "(";
    for (uint32_t i = 0; i < m.param_count(); ++i) {
        if (i)
            oss << ", ";
        auto p = m.param(i);
        oss << p.name_owned();
    }
    oss << ") : " << m.return_type().name_owned();
    uint32_t flags = m.flags(nullptr);
    bool isStatic = flags & 0x0010; // MethodAttributes.Static
    oss << (isStatic ? " [static]" : "");
    return oss.str();
}

std::string Explorer::objectToString(Object obj) {
    if (!obj)
        return std::string("null");
    try {
        auto toStr = obj.klass().get_method("ToString", 0);
        if (toStr) {
            Object s = toStr.invoke(obj.ptr, {});
            if (s) {
                String ss(reinterpret_cast<Il2CppString *>(s.ptr));
                return ss.to_utf8();
            }
        }
    } catch (const ManagedException& e) {
        return std::string("[ToString threw] ") + e.what();
    }
    return std::string("[object] ") + std::string(obj.klass().name());
}

void Explorer::setSelectedMethod(Method m) {
    selectedMethod_ = m;
    callState_ = CallState{m};
    callState_.argTexts.resize(m.param_count());
}

void Explorer::setSelectedProperty(Il2Cpp::API::Property p) {
    selectedProperty_ = p;
    propertyState_ = PropertyState{p};
}

void Explorer::createDefaultInstance() {
    if (selectedClass_ < 0 || selectedClass_ >= static_cast<int>(classList_.size()))
        return;
    Class c = classList_[selectedClass_].klass;

    try {
        Object o = object_new(c);
        // Try call default .ctor if present
        auto ctor = c.get_method(".ctor", 0);
        if (ctor)
            ctor.invoke(o.ptr, {});
        InstanceEntry e;
        e.handle = GcHandle(o.ptr);
        instances_.push_back(std::move(e));
        selectedInstance_ = static_cast<int>(instances_.size() - 1);
    } catch (const ManagedException& e) {
        callState_.lastError = std::string("Creating instance failed: ") + e.what();
    } catch (const std::exception& e) {
        callState_.lastError = std::string("Creating instance failed: ") + e.what();
    }
}

bool Explorer::parseAndBoxArg(const Type& paramType, const std::string& text, Il2CppObject *& outObj, std::string& err) {
    outObj = nullptr;
    const Class pc = paramType.class_or_element();
    if (!pc) {
        err = "Invalid parameter type.";
        return false;
    }

    // "null" literal
    if (text == "null" || text == "(null)" || text.empty()) {
        // allow null for reference types and string; for value types this will likely be invalid but runtime may allow default
        if (!pc.is_value_type() || isStringClass(pc)) {
            outObj = nullptr;
            return true;
        } else {
            // Provide default(T) via boxing zero
            if (pc.is_enum()) {
                const int32_t v = 0;
                outObj = value_box<int32_t>(pc, v);
                return true;
            }
            // Try zero for numeric primitives and floats
            // We'll just box Int32 zero and let runtime convert if needed
            const int32_t v = 0;
            outObj = value_box<int32_t>(sysInt32_, v);
            return true;
        }
    }

    // If expected string
    if (isStringClass(pc)) {
        outObj = make_string(text).ptr;
        return true;
    }

    // Try boolean literals
    if (pc.namespaze() == std::string_view("System") && pc.name() == std::string_view("Boolean")) {
        bool v = false;
        if (text == "true" || text == "True" || text == "1")
            v = true;
        else if (text == "false" || text == "False" || text == "0")
            v = false;
        else {
            err = "Expected Boolean (true/false).";
            return false;
        }
        outObj = value_box<bool>(sysBoolean_, v);
        return true;
    }

    // Numbers: detect hex, float, int
    const auto isHex = [](const std::string& s) { return s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'); };
    const auto isFloatLike = [](const std::string& s) {
        return s.find('.') != std::string::npos || s.find('e') != std::string::npos || s.find('E') != std::string::npos;
    };

    if (isHex(text)) {
        // Parse as unsigned 64 then box Int64
        uint64_t val = 0;
        const auto sv = std::string_view(text).substr(2);
        for (char ch : sv) {
            val <<= 4;
            if (ch >= '0' && ch <= '9')
                val |= static_cast<uint64_t>(ch - '0');
            else if (ch >= 'a' && ch <= 'f')
                val |= static_cast<uint64_t>(ch - 'a' + 10);
            else if (ch >= 'A' && ch <= 'F')
                val |= static_cast<uint64_t>(ch - 'A' + 10);
            else {
                err = "Invalid hex number.";
                return false;
            }
        }
        const int64_t i64 = static_cast<int64_t>(val);
        outObj = value_box<int64_t>(sysInt64_, i64);
        return true;
    }

    if (isFloatLike(text)) {
        char *end = nullptr;
        const double dv = std::strtod(text.c_str(), &end);
        if (!end || *end != '\0') {
            err = "Invalid floating number.";
            return false;
        }
        outObj = value_box<double>(sysDouble_, dv);
        return true;
    } else {
        // Integer
        char *end = nullptr;
        const long long iv = std::strtoll(text.c_str(), &end, 10);
        if (!end || *end != '\0') {
            err = "Invalid integer.";
            return false;
        }
        if (iv >= std::numeric_limits<int32_t>::min() && iv <= std::numeric_limits<int32_t>::max())
            outObj = value_box<int32_t>(sysInt32_, static_cast<int32_t>(iv));
        else
            outObj = value_box<int64_t>(sysInt64_, static_cast<int64_t>(iv));
        return true;
    }
}

void Explorer::invokeSelectedMethod() {
    callState_.lastError.clear();
    callState_.lastReturn.clear();
    callState_.lastReturnObject = GcHandle();

    if (!selectedMethod_) {
        callState_.lastError = "No method selected.";
        return;
    }

    try {
        auto& instance = instances_[selectedInstance_];

        // Prepare args
        uint32_t argc = selectedMethod_.param_count();
        std::vector<Il2CppObject *> args(argc, nullptr);
        for (uint32_t i = 0; i < argc; ++i) {
            std::string err;
            if (!parseAndBoxArg(selectedMethod_.param(i), callState_.argTexts[i], args[i], err)) {
                callState_.lastError = "Arg " + std::to_string(i) + ": " + err;
                return;
            }
        }

        Il2CppObject *thisObj = nullptr;
        if (selectedMethod_.is_instance()) {
            if (selectedInstance_ < 0 || selectedInstance_ >= static_cast<int>(instances_.size())) {
                callState_.lastError = "No instance selected for instance method.";
                return;
            }
            thisObj = instance.handle.target();
            if (!thisObj) {
                callState_.lastError = "Selected instance was collected or invalid.";
                return;
            }

            // Make sure the instance type is compatible with declaring type
            Class instK = Object{thisObj}.klass();
            Class declK = selectedMethod_.declaring_type();
            if (!declK.is_assignable_from(instK)) {
                callState_.lastError = "Selected instance is not assignable to method's declaring type.";
                return;
            }
        }

        Object ret = selectedMethod_.invoke_convert(thisObj, args);
        if (ret) {
            callState_.lastReturn = objectToString(ret);
            callState_.lastReturnObject = GcHandle(ret.ptr); // keep alive for inspection if needed

            // Set new object if called object is constructor
            if (selectedMethod_.name() == ".ctor")
                instance.handle = GcHandle(ret.ptr);
        } else {
            callState_.lastReturn = "null";
        }
    } catch (const ManagedException& e) {
        callState_.lastError = std::string("Managed exception:\n") + e.what();
    } catch (const std::exception& e) {
        callState_.lastError = std::string("Native exception: ") + e.what();
    }
}

bool Explorer::isStringClass(const Class& c) { return c.namespaze() == std::string_view("System") && c.name() == std::string_view("String"); }

ModInfo explorerInfo{"Explorer", false, []() { return std::make_unique<Explorer>(); }};
