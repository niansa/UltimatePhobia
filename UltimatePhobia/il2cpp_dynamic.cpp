#include "il2cpp_dynamic.hpp"
#include "il2cpp_api_cpp.hpp"
#include "json.hpp"

#include <chrono>
#include <list>
#include <fstream>
#include <cstdint>

namespace Il2Cpp::Dynamic {
namespace {
std::vector<Method> methods;
std::mutex methods_mutex; // Intentionally used only very loosely
std::list<std::string> string_arena;

using namespace Il2Cpp::API;

inline std::string_view intern(std::string s) {
    string_arena.push_back(std::move(s));
    return std::string_view(string_arena.back());
}

std::string replace_non_ascii_printable(const std::string& str) {
    std::string result;
    result.reserve(str.length());

    for (size_t i = 0; i < str.length();) {
        unsigned char c = str[i];

        // Pure ASCII character (high bit is 0)
        if (c < 128) {
            if (std::isprint(c)) {
                result += c;
            } else {
                result += '_';
            }
            i++;
        } else {
            // UTF-8 multi-byte character
            int char_len = 0;

            // Determine character length from first byte
            if ((c & 0xE0) == 0xC0) {
                char_len = 2;
            } else if ((c & 0xF0) == 0xE0) {
                char_len = 3;
            } else if ((c & 0xF8) == 0xF0) {
                char_len = 4;
            } else {
                // Invalid UTF-8 starting byte, skip just this byte
                result += '_';
                i++;
                continue;
            }

            // Check if we have enough bytes remaining
            if (i + char_len > str.length()) {
                // Incomplete character at end of string
                result += '_';
                break;
            }

            // Verify continuation bytes (should be 10xxxxxx)
            bool valid = true;
            for (int j = 1; j < char_len; j++) {
                if ((str[i + j] & 0xC0) != 0x80) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                // Valid multi-byte character - replace with single '_'
                result += '_';
                i += char_len;
            } else {
                // Invalid UTF-8 sequence, skip just the first byte
                result += '_';
                i++;
            }
        }
    }

    return result;
}

std::string to_symbol_name(std::string_view ns, std::string_view name) {
    // Dumper style: Namespace_ClassName
    if (ns.empty())
        return std::string(name);
    std::string out;
    out.reserve(ns.size() + 1 + name.size());
    out.append(ns.data(), ns.size());
    std::replace(out.begin(), out.end(), '.', '_');
    out.push_back('_');
    out.append(name.data(), name.size());
    return out;
}

bool is_array_type(const Type& t) {
    // il2cpp_type_get_name returns names like "System.Int32[]" or "Foo.Bar[,]"
    auto n = t.name_owned();
    return n.find('[') != std::string::npos;
}

std::string primitive_to_c(std::string_view sysName) {
    // sysName: "System.Int32" etc.
    if (sysName == "System.Boolean")
        return "bool";
    if (sysName == "System.SByte")
        return "int8_t";
    if (sysName == "System.Byte")
        return "uint8_t";
    if (sysName == "System.Int16")
        return "int16_t";
    if (sysName == "System.UInt16")
        return "uint16_t";
    if (sysName == "System.Int32")
        return "int32_t";
    if (sysName == "System.UInt32")
        return "uint32_t";
    if (sysName == "System.Int64")
        return "int64_t";
    if (sysName == "System.UInt64")
        return "uint64_t";
    if (sysName == "System.Char")
        return "char16_t";
    if (sysName == "System.Single")
        return "float";
    if (sysName == "System.Double")
        return "double";
    if (sysName == "System.IntPtr")
        return "intptr_t";
    if (sysName == "System.UIntPtr")
        return "uintptr_t";
    return {};
}

std::string class_to_obj_c(const Class& c) {
    // e.g., "UnityEngine_Component_o*"
    auto sym = to_symbol_name(c.namespaze(), c.name());
    std::replace(sym.begin(), sym.end(), '.', '_');
    sym += "_o*";
    return sym;
}
std::string type_to_obj_c(const Type& c, bool ptr) {
    // e.g., "UnityEngine_Component_o*"
    std::string sym = c.name_owned();
    if (sym.back() == '*')
        sym.pop_back();
    std::replace(sym.begin(), sym.end(), '.', '_');
    sym += "_o";
    if (ptr)
        sym.push_back('*');
    return sym;
}

std::string class_to_val_c(const Class& c) {
    // value type by value: "Namespace_Class_o"
    auto sym = to_symbol_name(c.namespaze(), c.name());
    std::replace(sym.begin(), sym.end(), '.', '_');
    sym += "_o";
    return sym;
}

std::string array_to_c(const Class& elem) {
    // e.g., "UnityEngine_Component_array*"
    auto sym = to_symbol_name(elem.namespaze(), elem.name());
    std::replace(sym.begin(), sym.end(), '.', '_');
    sym += "_array*";
    return sym;
}

std::string type_to_cdecl(const Type& t) {
    if (!t.ptr)
        return "void*";

    switch (static_cast<Il2CppTypeEnum>(t.kind())) {
    case IL2CPP_TYPE_VOID:
        return "void";
    case IL2CPP_TYPE_BOOLEAN:
        return "bool";
    case IL2CPP_TYPE_CHAR:
        return "char";
    case IL2CPP_TYPE_I1:
        return "int8_t";
    case IL2CPP_TYPE_U1:
        return "uint8_t";
    case IL2CPP_TYPE_I2:
        return "int16_t";
    case IL2CPP_TYPE_U2:
        return "uint16_t";
    case IL2CPP_TYPE_I4:
        return "int32_t";
    case IL2CPP_TYPE_U4:
        return "uint32_t";
    case IL2CPP_TYPE_I8:
        return "int64_t";
    case IL2CPP_TYPE_U8:
        return "uint64_t";
    case IL2CPP_TYPE_R4:
        return "float";
    case IL2CPP_TYPE_R8:
        return "double";
    case IL2CPP_TYPE_STRING:
        return "System_String_o*";
    case IL2CPP_TYPE_PTR:
        return type_to_obj_c(t, true);
    case IL2CPP_TYPE_BYREF:
        return "void *"; // Probably correct?
    case IL2CPP_TYPE_VALUETYPE: {
        if (t.class_or_element().is_enum())
            return "int32_t";
        return type_to_obj_c(t, false);
    }
    case IL2CPP_TYPE_CLASS:
        return type_to_obj_c(t, true);
    case IL2CPP_TYPE_MVAR:
    case IL2CPP_TYPE_VAR:
        return "T";
    case IL2CPP_TYPE_SZARRAY:
    case IL2CPP_TYPE_ARRAY: {
        Class elem = t.class_or_element();
        if (elem)
            return array_to_c(elem);
        return "void_array*";
    }
    case IL2CPP_TYPE_GENERICINST:
        return type_to_obj_c(t, false); // TODO (System_RuntimeType_ListBuilder_MethodInfo__o for System.RuntimeType.ListBuilder<System.Reflection.MethodInfo>)
    case IL2CPP_TYPE_TYPEDBYREF:
        return "Il2CppObject*";
    case IL2CPP_TYPE_I:
        return "intptr_t";
    case IL2CPP_TYPE_U:
        return "uintptr_t";
    case IL2CPP_TYPE_OBJECT:
        return "Il2CppObject*";
    default: {
        g.logger->warn("Can not represent {} (attrs: {:b}, kind: {})", t.name_owned(), t.attrs(), t.kind());
        return "void *"; // Unable to represent
    }
    }
}

char type_to_sig(const Type& t) {
    if (!t.ptr)
        return 'i';
    if (t.kind() == IL2CPP_TYPE_VOID)
        return 'v';

    // 64-bit ints
    if (t.kind() == IL2CPP_TYPE_I8 || t.kind() == IL2CPP_TYPE_U8)
        return 'j';

    // float/double
    if (t.kind() == IL2CPP_TYPE_R4)
        return 'f';
    if (t.kind() == IL2CPP_TYPE_R8)
        return 'd';

    // Everything else treated as 'i' (pointers, int32, bool, structs, arrays, byref, etc.)
    return 'i';
}

std::string make_method_display_name(const Class& c, const ApiMethod& m) {
    // Namespace.Class$$Method[<...>]
    std::string out;
    if (!c.namespaze().empty()) {
        out.append(c.namespaze().data(), c.namespaze().size());
        out.push_back('.');
    }
    out.append(c.name().data(), c.name().size());
    out += "$$";
    out.append(m.name().data(), m.name().size());

    // Show generic marker if generic
    if (m.is_generic() || m.is_inflated()) {
        out += "<>";
    }
    return out;
}

std::string make_c_symbol_name(const Class& c, const ApiMethod& m) {
    // Namespace_Class__Method
    auto sym = to_symbol_name(c.namespaze(), c.name());
    std::replace(sym.begin(), sym.end(), '.', '_');
    sym += "__";
    auto mn = std::string(m.name());
    std::replace(mn.begin(), mn.end(), '.', '_');
    sym += mn;
    return sym;
}

std::string make_signature(const Class& c, const ApiMethod& m) {
    std::ostringstream oss;
    // Return type
    oss << type_to_cdecl(m.return_type()) << ' ';

    // Function symbol
    oss << make_c_symbol_name(c, m) << " (";

    bool first = true;

    // Instance "this"
    if (m.is_instance()) {
        if (!first)
            oss << ", ";
        first = false;
        oss << class_to_obj_c(c) << " __this";
    }

    // Parameters
    const auto pcount = m.param_count();
    for (uint32_t i = 0; i < pcount; ++i) {
        if (!first)
            oss << ", ";
        first = false;
        auto ptype = m.param(i);
        auto pname = m.param_name(i);
        std::string argName = pname.empty() ? ("arg" + std::to_string(i)) : std::string(pname);
        if (argName == "klass")
            argName = "_" + argName;
        // Replace illegal chars in argName if any
        std::replace(argName.begin(), argName.end(), '.', '_');
        oss << type_to_cdecl(ptype) << ' ' << argName;
    }

    // MethodInfo*
    if (!first)
        oss << ", ";
    oss << "const MethodInfo* method";
    oss << ");";
    return replace_non_ascii_printable(oss.str());
}

std::string make_type_signature(const Class& c, const ApiMethod& m) {
    (void)c;
    std::string sig;
    sig.reserve(1 + m.param_count() + 2);

    // Return
    sig.push_back(type_to_sig(m.return_type()));

    // this
    if (m.is_instance())
        sig.push_back('i');

    // args
    const auto pcount = m.param_count();
    for (uint32_t i = 0; i < pcount; ++i)
        sig.push_back(type_to_sig(m.param(i)));

    // MethodInfo*
    sig.push_back('i');
    return sig;
}
} // namespace

void init() {
    using std::chrono::high_resolution_clock;

    methods.clear();
    string_arena.clear();

    auto domain = Domain::get();
    if (!domain)
        return;

    unsigned runningIndex = 0;
    unsigned errors = 0;

    const auto time_start = high_resolution_clock::now();
    for (const Assembly& asmbl : domain.get_assemblies()) {
        Image img = asmbl.image();
        if (!img)
            continue;

        const size_t klassCount = img.class_count();
        for (size_t ci = 0; ci < klassCount; ++ci) {
            Class klass = img.get_class(ci);
            if (!klass) {
                ++errors;
                continue;
            }

            for (const ApiMethod& mm : klass.methods()) {
                if (!mm) {
                    ++errors;
                    continue;
                }

                Method out{};
                // Method
                out.method = mm;
                // Name
                out.name = intern(make_method_display_name(klass, mm));
                // Signature
                out.signature = intern(make_signature(klass, mm));
                // Type signature
                out.typeSignature = intern(make_type_signature(klass, mm));
                // Index
                out.index = runningIndex++;

                methods.emplace_back(std::move(out));
            }
        }
    }
    const auto time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(high_resolution_clock::now() - time_start).count();

    // Log out function count
    if (errors)
        g.logger->info("Processed {} methods with {} errors"
                       "from il2cpp runtime",
                       methods.size(), errors);
    else
        g.logger->info("Processed {} methods from il2cpp runtime", methods.size());
    g.logger->info("Processing reflection data from il2cpp runtime took {} ms ({} methods/s)", time_taken,
                   unsigned(double(methods.size()) / (double(time_taken) * 0.001) + 0.5));

    // Validate some pointers
    bool valid = true;
    const auto validate = [&valid](std::string_view name, void *ptr) {
        if (getMethod(name).getFullAddress() != ptr)
            valid = false;
    };
    validate("void UnityEngine_Application__Quit (const MethodInfo* method);", Il2Cpp::UnityEngine::Application::Quit_getPtr());
    validate("Player$$Update", Il2Cpp::Player::Update_getPtr());
    validate("GhostAI$$Appear", Il2Cpp::GhostAI::Appear_getPtr());
    if (!valid)
        g.logger->warn("Loaded il2cpp runtime doesn't match script.json "
                       "UltimatePhobia was compiled with! Expect serious issues.");
}

std::string dump() {
    using json = nlohmann::json;
    json root = json::object();
    auto& methodsJson = root["ScriptMethod"] = json::array();
    for (const auto& method : methods) {
        const uintptr_t addr = reinterpret_cast<uintptr_t>(method.getFullAddress()) - g.base;
        if (addr >= 2000000000)
            continue;
        json& methodJson = methodsJson.emplace_back(json::object());
        methodJson["Address"] = addr;
        methodJson["Name"] = method.name;
        methodJson["Signature"] = method.signature;
        methodJson["TypeSignature"] = method.typeSignature;
    }
    return root.dump(2, ' ', true);
}

bool isLoaded() {
    std::scoped_lock L(methods_mutex);
    return !methods.empty();
}

Method getMethod(std::string_view identifier, bool noError) {
    for (const Method& method : methods)
        if (method.signature == identifier)
            return method;

    for (const Method& method : methods)
        if (method.name == identifier)
            return method;

    if (!noError)
        g.logger->error("Failed to find method by identifier: {}", identifier);
    return {nullptr};
}

Method getMethod(void *addr, bool noError) {
    for (Method method : methods)
        if (method.getFullAddress() == addr)
            return method;

    if (!noError)
        g.logger->error("Failed to find method by address: {}", addr);
    return {nullptr};
}

Method getMethod(unsigned idx, bool noError) {
    if (idx > methods.size()) {
        if (!noError)
            g.logger->error("Failed to find method by index: {}", idx);
        return {nullptr};
    }

    return methods[idx];
}

std::vector<Method> searchMethods(std::string_view identifier) {
    std::vector<Method> fres;

    const auto isDup = [&fres](const Method& method) {
        for (const auto& other_method : fres)
            if (method.signature == other_method.signature)
                return true;
        return false;
    };

    // Try exact search first
    {
        const auto result = getMethod(identifier, true);
        if (result.isValid())
            fres.emplace_back(std::move(result));
    }

    // Then try name search
    for (const Method& method : methods) {
        if (isDup(method))
            continue;
        if (method.name.find(identifier) != std::string_view::npos)
            fres.emplace_back(method);
    }

    // Finally try signature search
    for (const Method& method : methods) {
        if (isDup(method))
            continue;
        if (method.signature.find(identifier) != std::string_view::npos)
            fres.emplace_back(method);
    }

    return fres;
}

const std::vector<Method>& getMethods() { return methods; }

bool registerICall(const char *name, std::string_view typeSignature) {
    // Validate typeSignature
    const std::string_view validChars = "ijfd";
    for (size_t idx = 0; idx != typeSignature.size(); ++idx) {
        const char c = typeSignature[idx];
        if (validChars.find(c) == validChars.npos && !(idx == 0 && c == 'v'))
            return false;
    }

    // Replace $ with : for icall name syntax
    std::string iname(name);
    for (char& c : iname)
        if (c == '$')
            c = ':';

    // Get function pointer
    void *fptr = reinterpret_cast<void *>(Il2Cpp::API::il2cpp_resolve_icall(iname.c_str()));
    if (fptr == nullptr)
        return false;

    // Register method
    methods.emplace_back(Method{fptr, string_arena.emplace_back(name), "", string_arena.emplace_back(typeSignature), static_cast<int>(methods.size())});
    return true;
}
} // namespace Il2Cpp::Dynamic
