#include "il2cpp_api_cpp.hpp"
#include "il2cpp_api.hpp"

namespace Il2Cpp::API {

// ======================== Utils =========================

namespace {
namespace Utils {
inline std::string utf16_to_utf8(std::u16string_view s) {
    std::string out;
    out.reserve(s.size()); // approximate

    auto append = [&](char c) { out.push_back(c); };

    for (size_t i = 0; i < s.size(); ++i) {
        uint32_t code = s[i];
        if (code >= 0xD800 && code <= 0xDBFF) { // high surrogate
            if (i + 1 < s.size()) {
                uint32_t low = s[i + 1];
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    code = ((code - 0xD800) << 10) + (low - 0xDC00) + 0x10000;
                    ++i;
                }
            }
        }
        if (code <= 0x7F) {
            append(static_cast<char>(code));
        } else if (code <= 0x7FF) {
            append(static_cast<char>(0xC0 | ((code >> 6) & 0x1F)));
            append(static_cast<char>(0x80 | (code & 0x3F)));
        } else if (code <= 0xFFFF) {
            append(static_cast<char>(0xE0 | ((code >> 12) & 0x0F)));
            append(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
            append(static_cast<char>(0x80 | (code & 0x3F)));
        } else {
            append(static_cast<char>(0xF0 | ((code >> 18) & 0x07)));
            append(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
            append(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
            append(static_cast<char>(0x80 | (code & 0x3F)));
        }
    }
    return out;
}

inline std::u16string utf8_to_utf16(std::string_view s) {
    std::u16string out;
    out.reserve(s.size());

    auto next = [&](size_t& i) -> unsigned char {
        if (i >= s.size())
            return 0;
        return static_cast<unsigned char>(s[i++]);
    };

    for (size_t i = 0; i < s.size();) {
        uint32_t code;
        unsigned char c0 = next(i);
        if (c0 < 0x80) {
            code = c0;
        } else if ((c0 >> 5) == 0x6) {
            unsigned char c1 = next(i);
            code = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
        } else if ((c0 >> 4) == 0xE) {
            unsigned char c1 = next(i), c2 = next(i);
            code = ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
        } else if ((c0 >> 3) == 0x1E) {
            unsigned char c1 = next(i), c2 = next(i), c3 = next(i);
            code = ((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        } else {
            code = 0xFFFD;
        }

        if (code <= 0xFFFF) {
            out.push_back(static_cast<char16_t>(code));
        } else {
            code -= 0x10000;
            out.push_back(static_cast<char16_t>(0xD800 + ((code >> 10) & 0x3FF)));
            out.push_back(static_cast<char16_t>(0xDC00 + (code & 0x3FF)));
        }
    }
    return out;
}
} // namespace Utils
} // namespace

std::string format_exception(Il2CppException *ex) {
    if (!ex)
        return {};
    char buf[4096]{};
    il2cpp_format_exception(ex, buf, static_cast<int>(sizeof(buf)));
    return std::string(buf);
}

std::string format_stacktrace(Il2CppException *ex) {
    if (!ex)
        return {};
    char buf[16384]{};
    il2cpp_format_stack_trace(ex, buf, static_cast<int>(sizeof(buf)));
    return std::string(buf);
}

std::string String::to_utf8() const { return Utils::utf16_to_utf8(to_u16string()); }

bool Il2Cpp::API::Type::is_compatible_with(Class klass) const {
    if ((ptr == nullptr || kind() == IL2CPP_TYPE_VOID) && klass.ptr == nullptr)
        return true;

    const auto this_kind = static_cast<Il2CppTypeEnum>(kind());
    const auto target_kind = klass ? klass.type().kind() : IL2CPP_TYPE_VOID;

    switch (this_kind) {
    case IL2CPP_TYPE_VOID:
    case IL2CPP_TYPE_BOOLEAN:
    case IL2CPP_TYPE_CHAR:
    case IL2CPP_TYPE_I1:
    case IL2CPP_TYPE_U1:
    case IL2CPP_TYPE_I2:
    case IL2CPP_TYPE_U2:
    case IL2CPP_TYPE_I4:
    case IL2CPP_TYPE_U4:
    case IL2CPP_TYPE_I8:
    case IL2CPP_TYPE_U8:
    case IL2CPP_TYPE_R4:
    case IL2CPP_TYPE_R8:
    case IL2CPP_TYPE_STRING:
    case IL2CPP_TYPE_MVAR:
    case IL2CPP_TYPE_VAR:
    case IL2CPP_TYPE_TYPEDBYREF:
    case IL2CPP_TYPE_CLASS:
    case IL2CPP_TYPE_I:
    case IL2CPP_TYPE_U: {
        const bool result = kind() == target_kind;
        return result;
    }
    case IL2CPP_TYPE_VALUETYPE: {
        Class thisClass = class_or_element();
        const bool result = thisClass.ptr == klass.ptr || (thisClass.is_enum() && klass.type().kind() == IL2CPP_TYPE_I4);
        return result;
    }
    default: {
        const bool result = !klass || ptr == klass.type().ptr || name_owned() == klass.type().name_owned();
        return result;
    }
    }
}

void *Il2Cpp::API::Object::unboxIfValue() {
    if (ptr == nullptr)
        return nullptr;

    if (klass().is_value_type())
        return il2cpp_object_unbox(ptr);

    return reinterpret_cast<void *>(ptr);
}

std::vector<Assembly> Il2Cpp::API::Domain::get_assemblies() const {
    size_t n = 0;
    auto arr = il2cpp_domain_get_assemblies(ptr, &n);
    std::vector<Assembly> out;
    out.reserve(n);
    for (size_t i = 0; i < n; ++i)
        out.push_back(Assembly{arr[i]});
    return out;
}

Class Il2Cpp::API::Image::get_class(std::string_view namespaze, std::string_view name) {
    const size_t max_idx = class_count();
    for (size_t idx = 0; idx != max_idx; ++idx) {
        Class klass = get_class(idx);
        if (klass.namespaze() == namespaze && klass.name() == name)
            return klass;
    }
    return {};
}

Method Il2Cpp::API::Class::get_method(const char *name, std::span<Class> args) const {
    void *it = nullptr;
    while (auto m = Method{il2cpp_class_get_methods(ptr, &it)}) {
        if (m.name() != name)
            continue;

        // Check argument count first
        if (m.param_count() != args.size()) {
            continue;
        }

        // Compare argument types
        bool matches = true;
        for (size_t idx = 0; idx != args.size(); ++idx) {
            auto param_type = m.param(idx);
            auto arg_class = args[idx];

            if (!param_type.is_compatible_with(arg_class)) {
                matches = false;
                break;
            }
        }
        if (!matches)
            continue;

        // This is the one!
        return m;
    }

    // No match, try parent
    if (auto p = parent())
        return p.get_method(name, args);
    return {};
}

std::vector<Field> Il2Cpp::API::Class::fields() const {
    std::vector<Field> out;
    void *it = nullptr;
    while (auto f = il2cpp_class_get_fields(ptr, &it))
        out.push_back(Field{f});
    return out;
}

std::vector<Method> Il2Cpp::API::Class::methods() const {
    std::vector<Method> out;
    void *it = nullptr;
    while (auto m = il2cpp_class_get_methods(ptr, &it))
        out.push_back(Method{m});
    return out;
}

std::vector<Class> Il2Cpp::API::Class::nested_types() const {
    std::vector<Class> out;
    void *it = nullptr;
    while (auto c = il2cpp_class_get_nested_types(ptr, &it))
        out.push_back(Class{c});
    return out;
}

std::vector<Property> Il2Cpp::API::Class::properties() const {
    std::vector<Property> out;
    void *it = nullptr;
    while (auto c = il2cpp_class_get_properties(ptr, &it))
        out.push_back(Property{const_cast<PropertyInfo *>(c)});
    return out;
}

Object Il2Cpp::API::Method::invoke(Object obj, std::span<void *> args) const {
    Il2CppException *exc = nullptr;
    Il2CppObject *ret = il2cpp_runtime_invoke(ptr, obj.unboxIfValue(), const_cast<void **>(args.data()), &exc);
    if (exc)
        throw ManagedException(format_exception(exc) + "\n" + format_stacktrace(exc), exc);
    return Object{ret};
}

Object Il2Cpp::API::Method::invoke_convert(Object obj, std::span<Il2CppObject *> args) const {
    Il2CppException *exc = nullptr;
    Il2CppObject *ret = il2cpp_runtime_invoke_convert_args(ptr, obj.unboxIfValue(), args.data(), static_cast<int>(args.size()), &exc);
    if (exc)
        throw ManagedException(format_exception(exc) + "\n" + format_stacktrace(exc), exc);
    return Object{ret};
}
} // namespace Il2Cpp::API
