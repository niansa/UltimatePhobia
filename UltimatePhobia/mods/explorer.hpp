#pragma once

#include "base.hpp"
#include "il2cpp_api_cpp.hpp"

#include <string>
#include <vector>
#include <imgui.h>

class Explorer final : public Mod {
public:
    Explorer();
    void uiUpdate() override;

private:
    // UI state
    bool open_ = true;
    char asmFilter_[128]{};
    char classFilter_[256]{};
    char memberFilter_[256]{};

    struct ClassEntry {
        Il2Cpp::API::Class klass{};
        std::string fullname;
    };

    struct InstanceEntry {
        Il2Cpp::API::GcHandle handle{};

        std::string getLabel() const;
    };

    struct CallState {
        Il2Cpp::API::Method method{};
        std::vector<std::string> argTexts;
        std::string lastError;
        std::string lastReturn;
        Il2Cpp::API::GcHandle lastReturnObject; // keep last return alive if it's a reference type
    };

    struct PropertyState {
        Il2Cpp::API::Property property{};
        std::string valueText;
    };

    // Data caches
    std::vector<Il2Cpp::API::Assembly> assemblies_;
    int selectedAssembly_ = -1;

    std::vector<ClassEntry> classList_; // for selected assembly
    int selectedClass_ = -1;            // index in classList_

    Il2Cpp::API::Method selectedMethod_{};
    Il2Cpp::API::Property selectedProperty_{};
    CallState callState_{};
    PropertyState propertyState_{};

    std::vector<InstanceEntry> instances_; // user-created instances
    int selectedInstance_ = -1;

    // Helpers for corlib boxing
    Il2Cpp::API::Image corlib_{};
    Il2Cpp::API::Class sysBoolean_{};
    Il2Cpp::API::Class sysInt32_{};
    Il2Cpp::API::Class sysInt64_{};
    Il2Cpp::API::Class sysDouble_{};
    Il2Cpp::API::Class sysSingle_{};
    Il2Cpp::API::Class sysString_{};

    // Build/refresh
    void refreshAssemblies();
    void rebuildClassList();

    // Drawing
    void drawWindow();
    void drawAssembliesPane();
    void drawClassesPane();
    void drawClassDetailPane();
    void drawInstancesPane();

    // Utils
    static std::string fullClassName(const Il2Cpp::API::Class& c);
    static std::string typeName(const Il2Cpp::API::Type& t);
    static std::string methodSignature(const Il2Cpp::API::Method& m);
    static std::string objectToString(Il2Cpp::API::Object obj);
    void setSelectedMethod(Il2Cpp::API::Method m);
    void setSelectedProperty(Il2Cpp::API::Property p);

    // Creation/Invocation
    void createDefaultInstance();
    bool parseAndBoxArg(const Il2Cpp::API::Type& paramType, const std::string& text, Il2CppObject *& outObj, std::string& err);
    void invokeSelectedMethod();
    static bool isStringClass(const Il2Cpp::API::Class& c);
};

extern ModInfo explorerInfo;
