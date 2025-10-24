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
    corlib = Image::get_corlib();
    sysObject = class_from_name(corlib, "System", "Object");
    sysBoolean = class_from_name(corlib, "System", "Boolean");
    sysInt32 = class_from_name(corlib, "System", "Int32");
    sysInt64 = class_from_name(corlib, "System", "Int64");
    sysDouble = class_from_name(corlib, "System", "Double");
    sysSingle = class_from_name(corlib, "System", "Single");
    sysString = class_from_name(corlib, "System", "String");
}

void Explorer::uiUpdate() { drawWindow(); }

void Explorer::refreshAssemblies() {
    assemblies.clear();
    auto d = Domain::get();
    assemblies = d.get_assemblies();
}

void Explorer::rebuildClassList() {
    classList.clear();
    selectedClass = -1;
    selectedMethod = Method{};
    selectedProperty = Property{};
    callState = CallState{};

    if (selectedAssembly < 0)
        return;
    if (selectedAssembly >= static_cast<int>(assemblies.size())) {
        selectedAssembly = -1;
        return;
    }

    auto img = assemblies[selectedAssembly].image();
    const auto classCount = img.class_count();
    classList.reserve(classCount);
    for (size_t i = 0; i < classCount; ++i) {
        Class c = img.get_class(i);
        if (!c)
            continue;
        ClassEntry e;
        e.klass = c;
        e.fullname = fullClassName(c);
        classList.push_back(std::move(e));
    }

    std::sort(classList.begin(), classList.end(), [](const ClassEntry& a, const ClassEntry& b) { return a.fullname < b.fullname; });
}

void Explorer::drawWindow() {
    if (!open_)
        return;

    if (assemblies.empty())
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
        for (int i = 0; i < static_cast<int>(assemblies.size()); ++i) {
            ImGui::PushID(i);

            auto img = assemblies[i].image();
            std::string name = std::string(img.name());
            std::string file = std::string(img.filename());
            if (asmFilter_[0] != '\0') {
                if (!icontains(name, asmFilter_) && !icontains(file, asmFilter_))
                    continue;
            }
            bool sel = (i == selectedAssembly);
            if (ImGui::Selectable(name.c_str(), sel)) {
                selectedAssembly = i;
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
        for (int i = 0; i < static_cast<int>(classList.size()); ++i) {
            ImGui::PushID(i);

            const auto& e = classList[i];
            if (classFilter_[0] != '\0') {
                if (!icontains(e.fullname, classFilter_)) {
                    ImGui::PopID();
                    continue;
                }
            }
            bool sel = (i == selectedClass);
            if (ImGui::Selectable(e.fullname.c_str(), sel)) {
                selectedClass = i;
                selectedMethod = Method{};
                callState = CallState{};
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

    if (selectedClass < 0 || selectedClass >= static_cast<int>(classList.size())) {
        ImGui::TextUnformatted("Select a class to inspect.");
        return;
    }

    Class c = classList[selectedClass].klass;

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
            if (selectedInstance >= 0 && instances[selectedInstance].handle.target()->klass == c.ptr)
                fv = f.get_value_object(instances[selectedInstance].handle.target());
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

            bool sel = (p.ptr == selectedProperty.ptr);
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

            bool sel = (m.ptr == selectedMethod.ptr);
            if (ImGui::Selectable(sig.c_str(), sel))
                setSelectedMethod(m);

            ImGui::PopID();
        }
    }

    if (ImGui::Button("Create instance (default .ctor)"))
        createDefaultInstance();

    ImGui::SeparatorText("Invoke");
    if (selectedMethod) {
        ImGui::Text("Method: %s", methodSignature(selectedMethod).c_str());

        // Instance selection for instance methods
        if (selectedMethod.is_instance()) {
            const bool hasValidSelection = (selectedInstance >= 0 && selectedInstance < static_cast<int>(instances.size()));
            ImGui::TextUnformatted("This method is an instance method.");
            ImGui::Text("Selected instance: %s", hasValidSelection ? instances[selectedInstance].getLabel().c_str() : "(none)");
            if (hasValidSelection) {
                ImGui::SameLine();
                if (ImGui::Button("Clear selection"))
                    selectedInstance = -1;
            }
        }

        // Argument inputs
        uint32_t argc = selectedMethod.param_count();
        while (callState.argTexts.size() < argc)
            callState.argTexts.emplace_back();

        for (uint32_t i = 0; i < argc; ++i) {
            ImGui::PushID(i);

            auto pt = selectedMethod.param(i);
            auto pname = selectedMethod.param_name(i);
            std::string label = std::string(pname) + " : " + typeName(pt);

            std::array<char, 512> input{};
            std::strncpy(input.data(), callState.argTexts[i].c_str(), input.size() - 1);

            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText(label.c_str(), input.data(), input.size()))
                callState.argTexts[i] = input.data();

            ImGui::PopID();
        }

        if (ImGui::Button("Invoke"))
            invokeSelectedMethod();
        if (!callState.lastError.empty()) {
            ImGui::Text("Last error: %s", callState.lastError.c_str());
        } else if (!callState.lastReturn.empty()) {
            ImGui::Text("Return: %s", callState.lastReturn.c_str());
            if (callState.lastReturnObject) {
                if (ImGui::Button("Take")) {
                    selectedInstance = instances.size();
                    instances.emplace_back(InstanceEntry{std::move(callState.lastReturnObject)});
                }
            }
        }
    } else {
        ImGui::TextUnformatted("Select a method to prepare invocation.");
    }

    ImGui::SeparatorText("Inspect Property");
    if (selectedProperty) {
        ImGui::Text("Type: %s", selectedProperty.getter().return_type().name_owned().c_str());
        if (auto m = selectedProperty.getter())
            if (ImGui::Button("Select getter"))
                selectedMethod = m;

        if (auto m = selectedProperty.setter())
            if (ImGui::Button("Select setter"))
                selectedMethod = m;
    } else {
        ImGui::TextUnformatted("Select a property to inspect.");
    }
}

void Explorer::drawInstancesPane() {
    ImGui::TextUnformatted("Instances (created here)");
    ImGui::Separator();

    // List instances
    for (int i = 0; i < static_cast<int>(instances.size()); ++i) {
        ImGui::PushID(i);

        auto& e = instances[i];
        bool selected = (i == selectedInstance);
        if (ImGui::Selectable(e.getLabel().c_str(), selected)) {
            selectedInstance = i;
        }
        ImGui::SameLine();
        if (ImGui::Button("Release")) {
            instances.erase(instances.begin() + i);
            if (selectedInstance == i)
                selectedInstance = -1;
            if (selectedInstance > i)
                selectedInstance--;
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
            Object s = toStr.invoke(obj, {});
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
    selectedMethod = m;
    callState = CallState{m};
    callState.argTexts.resize(m.param_count());
}

void Explorer::setSelectedProperty(Il2Cpp::API::Property p) {
    selectedProperty = p;
    propertyState = PropertyState{p};
}

void Explorer::createDefaultInstance() {
    if (selectedClass < 0 || selectedClass >= static_cast<int>(classList.size()))
        return;
    Class c = classList[selectedClass].klass;

    try {
        Object o = object_new(c);
        // Try call default .ctor if present
        auto ctor = c.get_method(".ctor", 0);
        if (ctor)
            ctor.invoke(o, {});
        InstanceEntry e;
        e.handle = GcHandle(o.ptr);
        instances.push_back(std::move(e));
        selectedInstance = static_cast<int>(instances.size() - 1);
    } catch (const ManagedException& e) {
        callState.lastError = std::string("Creating instance failed: ") + e.what();
    } catch (const std::exception& e) {
        callState.lastError = std::string("Creating instance failed: ") + e.what();
    }
}

bool Explorer::parseAndBoxArg(const Type& paramType, const std::string& text, Il2Cpp::API::Object& outObj, std::string& err) {
    outObj.ptr = nullptr;
    const Class pc = paramType.class_or_element();
    if (!pc) {
        err = "Invalid parameter type.";
        return false;
    }

    // Handle null/empty input
    if (text == "null" || text == "(null)" || text.empty()) {
        if (!pc.is_value_type() || isStringClass(pc)) {
            return true; // Null is valid for reference types and strings
        } else {
            // Box default value for value types
            if (pc.is_enum())
                outObj = value_box<int32_t>(pc, 0);
            else
                outObj = value_box<int32_t>(sysInt32, 0);
            return true;
        }
    }

    // Define parsing lambdas
    auto parseString = [&]() -> bool {
        outObj = make_string(text);
        return true;
    };

    auto parseBoolean = [&]() -> bool {
        if (text == "true" || text == "True" || text == "1") {
            outObj = value_box<bool>(sysBoolean, true);
        } else if (text == "false" || text == "False" || text == "0") {
            outObj = value_box<bool>(sysBoolean, false);
        } else {
            err = "Expected Boolean (true/false).";
            return false;
        }
        return true;
    };

    auto parseHex = [&]() -> bool {
        uint64_t val = 0;
        auto sv = std::string_view(text).substr(2);
        for (char ch : sv) {
            val <<= 4;
            if (ch >= '0' && ch <= '9') {
                val |= (ch - '0');
            } else if (ch >= 'a' && ch <= 'f') {
                val |= (ch - 'a' + 10);
            } else if (ch >= 'A' && ch <= 'F') {
                val |= (ch - 'A' + 10);
            } else {
                err = "Invalid hex number.";
                return false;
            }
        }
        outObj = value_box<int64_t>(sysInt64, static_cast<int64_t>(val));
        return true;
    };

    auto parseFloat = [&]() -> bool {
        char *end = nullptr;
        double dv = std::strtod(text.c_str(), &end);
        if (end == text.c_str() || *end != '\0') {
            err = "Invalid floating number.";
            return false;
        }
        if (pc.ptr == sysSingle.ptr)
            outObj = value_box<float>(sysSingle, static_cast<float>(dv));
        else
            outObj = value_box<double>(sysDouble, dv);
        return true;
    };

    auto parseInteger = [&]() -> bool {
        char *end = nullptr;
        long long iv = std::strtoll(text.c_str(), &end, 10);
        if (end == text.c_str() || *end != '\0') {
            err = "Invalid integer.";
            return false;
        }
        if (pc.ptr == sysInt32.ptr)
            outObj = value_box<int32_t>(sysInt32, static_cast<int32_t>(iv));
        else
            outObj = value_box<int64_t>(sysInt64, static_cast<int64_t>(iv));
        return true;
    };

    // Type-specific parsing
    if (isStringClass(pc))
        return parseString();

    if (pc.ptr == sysBoolean.ptr)
        return parseBoolean();

    // For sysObject_, try parsers in specified order
    if (pc.ptr == sysObject.ptr) {
        if (parseInteger())
            return true;
        if (parseFloat())
            return true;
        if (parseHex())
            return true;
        if (parseString())
            return true;
        err = "Failed to parse input as Integer, Float, Hex, or String.";
        return false;
    }

    // For other types, use original logic
    auto isHex = [](const std::string& s) { return s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'); };
    auto isFloatLike = [](const std::string& s) {
        return s.find('.') != std::string::npos || s.find('e') != std::string::npos || s.find('E') != std::string::npos;
    };

    if (isHex(text))
        return parseHex();
    else if (isFloatLike(text))
        return parseFloat();
    else
        return parseInteger();
}

void Explorer::invokeSelectedMethod() {
    callState.lastError.clear();
    callState.lastReturn.clear();
    callState.lastReturnObject = GcHandle();

    if (!selectedMethod) {
        callState.lastError = "No method selected.";
        return;
    }

    try {
        auto& instance = instances[selectedInstance];

        // Prepare args
        uint32_t argc = selectedMethod.param_count();
        std::vector<Object> args(argc);
        for (uint32_t i = 0; i < argc; ++i) {
            std::string err;
            if (!parseAndBoxArg(selectedMethod.param(i), callState.argTexts[i], args[i], err)) {
                callState.lastError = "Arg " + std::to_string(i) + ": " + err;
                return;
            }
        }

        Object thisObj;
        if (selectedMethod.is_instance()) {
            if (selectedInstance < 0 || selectedInstance >= static_cast<int>(instances.size())) {
                callState.lastError = "No instance selected for instance method.";
                return;
            }
            thisObj = Object{instance.handle.target()};
            if (!thisObj) {
                callState.lastError = "Selected instance was collected or invalid.";
                return;
            }

            // Make sure the instance type is compatible with declaring type
            Class instK = Object{thisObj}.klass();
            Class declK = selectedMethod.declaring_type();
            if (!declK.is_assignable_from(instK)) {
                callState.lastError = "Selected instance is not assignable to method's declaring type.";
                return;
            }
        }

        Object ret = selectedMethod.invoke_convert(thisObj, [args]() mutable {
            // Little bit of a hack, but it's really fast
            std::span<Object> ospan(args);
            std::span<Il2CppObject *>& fres = *reinterpret_cast<std::span<Il2CppObject *> *>(&ospan);
            static_assert(sizeof(ospan[0]) == sizeof(fres[0]));
            return fres;
        }());
        if (ret) {
            callState.lastReturn = objectToString(ret);
            callState.lastReturnObject = GcHandle(ret.ptr); // keep alive for inspection if needed

            // Set new object if called object is constructor
            if (selectedMethod.name() == ".ctor")
                instance.handle = GcHandle(ret.ptr);
        } else {
            callState.lastReturn = "null";
            callState.lastReturnObject = {};
        }
    } catch (const ManagedException& e) {
        callState.lastError = std::string("Managed exception:\n") + e.what();
    } catch (const std::exception& e) {
        callState.lastError = std::string("Native exception: ") + e.what();
    }
}

bool Explorer::isStringClass(const Class& c) { return c.namespaze() == std::string_view("System") && c.name() == std::string_view("String"); }

ModInfo explorerInfo{"Explorer", false, []() { return std::make_unique<Explorer>(); }};
