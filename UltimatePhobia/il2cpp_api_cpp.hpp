#pragma once

#include "il2cpp_api.hpp"

#include <string>
#include <memory>
#include <vector>
#include <span>
#include <optional>
#include <utility>

namespace Il2Cpp::API {
// ======================== Errors ========================

struct Error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct ManagedException : Error {
    Il2CppException *ex{};
    explicit ManagedException(std::string msg, Il2CppException *e = nullptr) : Error(std::move(msg)), ex(e) {}
};

// ======================== Utils =========================

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

// Unique pointer for il2cpp_alloc/il2cpp_free buffers
struct Il2CppDeleter {
    void operator()(void *p) const noexcept {
        if (p)
            il2cpp_free(p);
    }
};

template <class T> using unique_il2cpp = std::unique_ptr<T, Il2CppDeleter>;

// Helper to format managed exceptions
inline std::string format_exception(Il2CppException *ex) {
    if (!ex)
        return {};
    char buf[4096]{};
    il2cpp_format_exception(ex, buf, static_cast<int>(sizeof(buf)));
    return std::string(buf);
}

inline std::string format_stacktrace(Il2CppException *ex) {
    if (!ex)
        return {};
    char buf[16384]{};
    il2cpp_format_stack_trace(ex, buf, static_cast<int>(sizeof(buf)));
    return std::string(buf);
}

// ======================== RAII: Thread attach/detach ========================

struct ThreadAttach {
    Il2CppThread *thread{nullptr};
    explicit ThreadAttach(Il2CppDomain *domain = nullptr) {
        if (!domain)
            domain = il2cpp_domain_get();
        thread = il2cpp_thread_attach(domain);
    }
    ~ThreadAttach() {
        if (thread)
            il2cpp_thread_detach(thread);
    }
    ThreadAttach(const ThreadAttach&) = delete;
    ThreadAttach& operator=(const ThreadAttach&) = delete;
    ThreadAttach(ThreadAttach&& o) noexcept : thread(o.thread) { o.thread = nullptr; }
    ThreadAttach& operator=(ThreadAttach&& o) noexcept {
        if (this != &o) {
            std::swap(thread, o.thread);
        }
        return *this;
    }
};

// ======================== RAII: GCHandle ========================

struct GcHandle {
    uint32_t handle{0};
    GcHandle() = default;
    explicit GcHandle(Il2CppObject *obj, bool pinned = false) {
        if (obj)
            handle = il2cpp_gchandle_new(obj, pinned);
    }
    static GcHandle weak(Il2CppObject *obj, bool track_resurrection = false) {
        GcHandle g;
        if (obj)
            g.handle = il2cpp_gchandle_new_weakref(obj, track_resurrection);
        return g;
    }
    ~GcHandle() {
        if (handle)
            il2cpp_gchandle_free(handle);
    }
    GcHandle(const GcHandle&) = delete;
    GcHandle& operator=(const GcHandle&) = delete;
    GcHandle(GcHandle&& o) noexcept : handle(o.handle) { o.handle = 0; }
    GcHandle& operator=(GcHandle&& o) noexcept {
        if (this != &o)
            std::swap(handle, o.handle);
        return *this;
    }
    Il2CppObject *target() const { return handle ? il2cpp_gchandle_get_target(handle) : nullptr; }
    explicit operator bool() const noexcept { return handle != 0; }
};

// ======================== Thin wrappers ========================

struct Object;
struct String;
struct Type;
struct Image;
struct Class;
struct Method;
struct Field;
struct Property;
struct Array;
struct Domain;
struct Assembly;

struct Object {
    Il2CppObject *ptr{nullptr};
    explicit operator bool() const { return ptr != nullptr; }

    Class klass() const;
    uint32_t size() const { return il2cpp_object_get_size(ptr); }
    const MethodInfo *get_virtual_method(const MethodInfo *m) const { return il2cpp_object_get_virtual_method(ptr, m); }
    void *unboxIfValue();
};

struct String : Object {
    String() = default;
    explicit String(Il2CppString *s) { ptr = reinterpret_cast<Il2CppObject *>(s); }
    static String New(std::string_view s) { return String(il2cpp_string_new_len(s.data(), static_cast<uint32_t>(s.size()))); }
    static String NewUtf16(std::u16string_view s) {
        return String(il2cpp_string_new_utf16(reinterpret_cast<const Il2CppChar *>(s.data()), static_cast<int32_t>(s.size())));
    }
    int32_t length() const { return il2cpp_string_length(reinterpret_cast<Il2CppString *>(ptr)); }
    const Il2CppChar *chars() const { return il2cpp_string_chars(reinterpret_cast<Il2CppString *>(ptr)); }
    std::u16string to_u16string() const {
        auto len = length();
        auto c = chars();
        return std::u16string(reinterpret_cast<const char16_t *>(c), reinterpret_cast<const char16_t *>(c) + len);
    }
    std::string to_utf8() const { return Utils::utf16_to_utf8(to_u16string()); }
};

struct Domain {
    Il2CppDomain *ptr{nullptr};
    static Domain get() {
        Domain d{};
        d.ptr = il2cpp_domain_get();
        return d;
    }
    explicit operator bool() const { return ptr != nullptr; }

    Assembly open_assembly(const char *name);
    std::vector<Assembly> get_assemblies() const;
};

struct Assembly {
    const Il2CppAssembly *ptr{nullptr};
    explicit operator bool() const { return ptr != nullptr; }
    Image image() const;
};

struct Image {
    const Il2CppImage *ptr{nullptr};
    explicit operator bool() const { return ptr != nullptr; }

    static Image get_corlib() { return Image{il2cpp_get_corlib()}; };

    std::string_view name() const { return ptr ? std::string_view(il2cpp_image_get_name(ptr)) : std::string_view{}; }
    std::string_view filename() const { return ptr ? std::string_view(il2cpp_image_get_filename(ptr)) : std::string_view{}; }
    const MethodInfo *entry_point() const { return ptr ? il2cpp_image_get_entry_point(ptr) : nullptr; }
    size_t class_count() const { return ptr ? il2cpp_image_get_class_count(ptr) : 0; }
    Class get_class(size_t index) const;
    std::optional<Class> get_class(std::string_view namespaze, std::string_view name);
    Assembly get_assembly() const { return Assembly{il2cpp_image_get_assembly(ptr)}; }
};

struct Type {
    const Il2CppType *ptr{nullptr};
    explicit operator bool() const { return ptr != nullptr; }

    Object object() const { return Object{il2cpp_type_get_object(ptr)}; }
    bool is_byref() const { return il2cpp_type_is_byref(ptr); }
    bool is_static() const { return il2cpp_type_is_static(ptr); }
    bool is_pointer() const { return il2cpp_type_is_pointer_type(ptr); }
    uint32_t attrs() const { return il2cpp_type_get_attrs(ptr); }
    int kind() const { return il2cpp_type_get_type(ptr); }

    std::string name_owned() const {
        unique_il2cpp<char> p{il2cpp_type_get_name(ptr)};
        return p ? std::string(p.get()) : std::string{};
    }
    std::string assembly_qualified_name_owned() const {
        unique_il2cpp<char> p{il2cpp_type_get_assembly_qualified_name(ptr)};
        return p ? std::string(p.get()) : std::string{};
    }
    Class class_or_element() const;
    bool equals(Type other) const { return il2cpp_type_equals(ptr, other.ptr); }
};

struct Class {
    Il2CppClass *ptr{nullptr};
    explicit operator bool() const { return ptr != nullptr; }

    static Class from_type(const Il2CppType *t) { return Class{il2cpp_class_from_type(t)}; }
    static Class from_name(const Image& img, const char *ns, const char *name) { return Class{il2cpp_class_from_name(img.ptr, ns, name)}; }

    std::string_view name() const { return il2cpp_class_get_name(ptr); }
    std::string_view namespaze() const { return il2cpp_class_get_namespace(ptr); }
    Image image() const { return Image{il2cpp_class_get_image(ptr)}; }
    Class parent() const { return Class{il2cpp_class_get_parent(ptr)}; }
    Class declaring_type() const { return Class{il2cpp_class_get_declaring_type(ptr)}; }
    bool is_value_type() const { return il2cpp_class_is_valuetype(ptr); }
    bool is_enum() const { return il2cpp_class_is_enum(ptr); }
    bool is_abstract() const { return il2cpp_class_is_abstract(ptr); }
    bool is_interface() const { return il2cpp_class_is_interface(ptr); }
    bool is_generic() const { return il2cpp_class_is_generic(ptr); }
    bool is_inflated() const { return il2cpp_class_is_inflated(ptr); }
    bool has_references() const { return il2cpp_class_has_references(ptr); }
    int rank() const { return il2cpp_class_get_rank(ptr); }
    int32_t instance_size() const { return il2cpp_class_instance_size(ptr); }
    int array_element_size() const { return il2cpp_class_array_element_size(ptr); }
    int flags() const { return il2cpp_class_get_flags(ptr); }
    uint32_t type_token() const { return il2cpp_class_get_type_token(ptr); }
    uint32_t data_size() const { return il2cpp_class_get_data_size(ptr); }

    Type type() const { return Type{il2cpp_class_get_type(ptr)}; }
    Class element_class() const { return Class{il2cpp_class_get_element_class(ptr)}; }

    Field get_field(const char *name) const;
    Method get_method(const char *name, int args) const;
    Property get_property(const char *name) const;

    std::vector<Field> fields() const;
    std::vector<Method> methods() const;
    std::vector<Class> nested_types() const;
    std::vector<Property> properties() const;

    bool has_attribute(Class attr_class) const { return il2cpp_class_has_attribute(ptr, attr_class.ptr); }
    bool is_assignable_from(Class other) const { return il2cpp_class_is_assignable_from(ptr, other.ptr); }
    bool is_subclass_of(Class klassc, bool check_interfaces) const { return il2cpp_class_is_subclass_of(ptr, klassc.ptr, check_interfaces); }
    bool has_parent(Class klassc) const { return il2cpp_class_has_parent(ptr, klassc.ptr); }
};

struct Method {    
    const MethodInfo *ptr{nullptr};
    explicit operator bool() const { return ptr != nullptr; }

    std::string_view name() const { return il2cpp_method_get_name(ptr); }
    Class declaring_type() const { return Class{il2cpp_method_get_declaring_type(ptr)}; }
    Class klass() const { return Class{il2cpp_method_get_class(ptr)}; }
    bool is_generic() const { return il2cpp_method_is_generic(ptr); }
    bool is_inflated() const { return il2cpp_method_is_inflated(ptr); }
    bool is_instance() const { return il2cpp_method_is_instance(ptr); }
    uint32_t param_count() const { return il2cpp_method_get_param_count(ptr); }
    Type param(uint32_t index) const { return Type{il2cpp_method_get_param(ptr, index)}; }
    std::string_view param_name(uint32_t index) const { return il2cpp_method_get_param_name(ptr, index); }
    Type return_type() const { return Type{il2cpp_method_get_return_type(ptr)}; }
    uint32_t flags(uint32_t *iflags = nullptr) const { return il2cpp_method_get_flags(ptr, iflags); }
    uint32_t token() const { return il2cpp_method_get_token(ptr); }
    bool has_attribute(Class attr_class) const { return il2cpp_method_has_attribute(ptr, attr_class.ptr); }

    void *function_pointer() const { return ptr->methodPointer; }

    // Invoke using void** args (pointers to values/objects depending on signature).
    // Throws ManagedException if the target throws.
    Object invoke(Object obj, std::span<void *> args) const;

    // Invoke with argument conversion (Il2CppObject** params).
    Object invoke_convert(Object obj, std::span<Il2CppObject *> args) const;
};

struct Field {
    FieldInfo *ptr{nullptr};
    explicit operator bool() const { return ptr != nullptr; }

    std::string_view name() const { return il2cpp_field_get_name(ptr); }
    uint32_t flags() const { return il2cpp_field_get_flags(ptr); }
    Class parent() const { return Class{il2cpp_field_get_parent(ptr)}; }
    size_t offset() const { return il2cpp_field_get_offset(ptr); }
    Type type() const { return Type{il2cpp_field_get_type(ptr)}; }
    bool has_attribute(Class attr_class) const { return il2cpp_field_has_attribute(ptr, attr_class.ptr); }
    bool is_literal() const { return il2cpp_field_is_literal(ptr); }

    template <class T> T get_value(Il2CppObject *obj) const {
        T v{};
        il2cpp_field_get_value(obj, ptr, &v);
        return v;
    }
    Il2CppObject *get_value_object(Il2CppObject *obj) const { return il2cpp_field_get_value_object(ptr, obj); }
    template <class T> void set_value(Il2CppObject *obj, const T& v) const {
        T copy = v;
        il2cpp_field_set_value(obj, ptr, &copy);
    }
    template <class T> T static_get_value() const {
        T v{};
        il2cpp_field_static_get_value(ptr, &v);
        return v;
    }
    template <class T> void static_set_value(const T& v) const {
        T copy = v;
        il2cpp_field_static_set_value(ptr, &copy);
    }
    void set_value_object(Il2CppObject *obj, Il2CppObject *value) const { il2cpp_field_set_value_object(obj, ptr, value); }
};

struct Property {
    PropertyInfo *ptr{nullptr};
    explicit operator bool() const { return ptr != nullptr; }

    std::string_view name() const { return il2cpp_property_get_name(ptr); }
    Class parent() const { return Class{il2cpp_property_get_parent(ptr)}; }
    uint32_t flags() const { return il2cpp_property_get_flags(ptr); }
    Method getter() const { return Method{il2cpp_property_get_get_method(ptr)}; }
    Method setter() const { return Method{il2cpp_property_get_set_method(ptr)}; }
};

struct Array {
    Il2CppArray *ptr{nullptr};
    explicit operator bool() const { return ptr != nullptr; }

    static Array New(Class klass, std::span<Object> elements) {
        auto fres = Array{il2cpp_array_new(klass.ptr, elements.size())};
        for (size_t idx = 0; idx != elements.size(); ++idx)
            reinterpret_cast<Il2CppObject **>(reinterpret_cast<System_Byte_array *>(fres.ptr)->m_Items)[idx] = elements[idx].ptr;
        return fres;
    }
    static Array New(Il2CppClass *element, std::size_t length) { return Array{il2cpp_array_new(element, static_cast<uint32_t>(length))}; }
    static Array NewSpecific(Il2CppClass *arrayType, std::size_t length) { return Array{il2cpp_array_new_specific(arrayType, static_cast<uint32_t>(length))}; }
    uint32_t length() const { return il2cpp_array_length(ptr); }
    uint32_t byte_length() const { return il2cpp_array_get_byte_length(ptr); }

    Object objectAt(size_t idx) { return Object{reinterpret_cast<Il2CppObject **>(reinterpret_cast<System_Byte_array *>(ptr)->m_Items)[idx]}; }
};

inline Class Type::class_or_element() const { return Class{il2cpp_type_get_class_or_element_class(ptr)}; }

// Implementations dependent on other wrappers

inline Class Object::klass() const { return Class{il2cpp_object_get_class(ptr)}; }

inline void *Object::unboxIfValue() {
    if (ptr == nullptr)
        return nullptr;

    if (klass().is_value_type())
        return il2cpp_object_unbox(ptr);

    return reinterpret_cast<void *>(ptr);
}

inline Assembly Domain::open_assembly(const char *name) { return Assembly{il2cpp_domain_assembly_open(ptr, name)}; }

inline std::vector<Assembly> Domain::get_assemblies() const {
    size_t n = 0;
    auto arr = il2cpp_domain_get_assemblies(ptr, &n);
    std::vector<Assembly> out;
    out.reserve(n);
    for (size_t i = 0; i < n; ++i)
        out.push_back(Assembly{arr[i]});
    return out;
}

inline Image Assembly::image() const { return Image{il2cpp_assembly_get_image(ptr)}; }

inline Class Image::get_class(size_t index) const { return Class{const_cast<Il2CppClass *>(il2cpp_image_get_class(ptr, index))}; }

inline std::optional<Class> Image::get_class(std::string_view namespaze, std::string_view name) {
    const size_t max_idx = class_count();
    for (size_t idx = 0; idx != max_idx; ++idx) {
        Class klass = get_class(idx);
        if (klass.namespaze() == namespaze && klass.name() == name)
            return klass;
    }
    return std::nullopt;
}

inline Field Class::get_field(const char *n) const { return Field{il2cpp_class_get_field_from_name(ptr, n)}; }

inline Method Class::get_method(const char *n, int args) const { return Method{il2cpp_class_get_method_from_name(ptr, n, args)}; }

inline Property Class::get_property(const char *n) const { return Property{const_cast<PropertyInfo *>(il2cpp_class_get_property_from_name(ptr, n))}; }

inline std::vector<Field> Class::fields() const {
    std::vector<Field> out;
    void *it = nullptr;
    while (auto f = il2cpp_class_get_fields(ptr, &it))
        out.push_back(Field{f});
    return out;
}
inline std::vector<Method> Class::methods() const {
    std::vector<Method> out;
    void *it = nullptr;
    while (auto m = il2cpp_class_get_methods(ptr, &it))
        out.push_back(Method{m});
    return out;
}
inline std::vector<Class> Class::nested_types() const {
    std::vector<Class> out;
    void *it = nullptr;
    while (auto c = il2cpp_class_get_nested_types(ptr, &it))
        out.push_back(Class{c});
    return out;
}

inline std::vector<Property> Class::properties() const {
    std::vector<Property> out;
    void *it = nullptr;
    while (auto c = il2cpp_class_get_properties(ptr, &it))
        out.push_back(Property{const_cast<PropertyInfo *>(c)});
    return out;
}

inline Object Method::invoke(Object obj, std::span<void *> args) const {
    Il2CppException *exc = nullptr;
    Il2CppObject *ret = il2cpp_runtime_invoke(ptr, obj.unboxIfValue(), const_cast<void **>(args.data()), &exc);
    if (exc)
        throw ManagedException(format_exception(exc) + "\n" + format_stacktrace(exc), exc);
    return Object{ret};
}

inline Object Method::invoke_convert(Object obj, std::span<Il2CppObject *> args) const {
    Il2CppException *exc = nullptr;
    Il2CppObject *ret = il2cpp_runtime_invoke_convert_args(ptr, obj.unboxIfValue(), args.data(), static_cast<int>(args.size()), &exc);
    if (exc)
        throw ManagedException(format_exception(exc) + "\n" + format_stacktrace(exc), exc);
    return Object{ret};
}

// ======================== Monitor RAII ========================

struct MonitorLock {
    Il2CppObject *obj{};
    explicit MonitorLock(Il2CppObject *o) : obj(o) { il2cpp_monitor_enter(obj); }
    ~MonitorLock() {
        if (obj)
            il2cpp_monitor_exit(obj);
    }
    MonitorLock(const MonitorLock&) = delete;
    MonitorLock& operator=(const MonitorLock&) = delete;
    MonitorLock(MonitorLock&& o) noexcept : obj(o.obj) { o.obj = nullptr; }
    MonitorLock& operator=(MonitorLock&& o) noexcept {
        if (this != &o)
            std::swap(obj, o.obj);
        return *this;
    }
    static bool try_enter(Il2CppObject *o, uint32_t timeout_ms = 0) { return il2cpp_monitor_try_enter(o, timeout_ms); }
    static void pulse(Il2CppObject *o) { il2cpp_monitor_pulse(o); }
    static void pulse_all(Il2CppObject *o) { il2cpp_monitor_pulse_all(o); }
    static bool wait(Il2CppObject *o, uint32_t timeout_ms = 0) {
        if (timeout_ms == 0) {
            il2cpp_monitor_wait(o);
            return true;
        }
        return il2cpp_monitor_try_wait(o, timeout_ms);
    }
};

// ======================== GC helpers ========================

namespace gc {
inline void collect(int maxGenerations) { il2cpp_gc_collect(maxGenerations); }
inline int32_t collect_a_little() { return il2cpp_gc_collect_a_little(); }
inline void enable() { il2cpp_gc_enable(); }
inline void disable() { il2cpp_gc_disable(); }
inline bool is_disabled() { return il2cpp_gc_is_disabled(); }
inline int64_t used_size() { return il2cpp_gc_get_used_size(); }
inline int64_t heap_size() { return il2cpp_gc_get_heap_size(); }
inline bool is_incremental() { return il2cpp_gc_is_incremental(); }
inline int64_t max_time_slice_ns() { return il2cpp_gc_get_max_time_slice_ns(); }
inline void set_max_time_slice_ns(int64_t ns) { il2cpp_gc_set_max_time_slice_ns(ns); }
} // namespace gc

// ======================== Strings ========================

inline String make_string(std::string_view s) { return String::New(s); }
inline String make_string(std::u16string_view s) { return String::NewUtf16(s); }

// ======================== Runtime helpers ========================

inline void class_init(Class c) { il2cpp_runtime_class_init(c.ptr); }

inline Object object_new(Class c) { return Object{il2cpp_object_new(c.ptr)}; }

template <class T> inline Object value_box(Class c, const T& value) { return Object{il2cpp_value_box(c.ptr, const_cast<T *>(&value))}; }

template <class T> inline T *object_unbox(Object o) { return static_cast<T *>(il2cpp_object_unbox(o.ptr)); }

// Raise a managed exception (no return)
[[noreturn]]
inline void raise_exception(Il2CppException *ex) {
    il2cpp_raise_exception(ex);
    std::terminate();
}

inline Il2CppException *exception_from_name(const Image& img, const char *ns, const char *name, const char *msg) {
    return il2cpp_exception_from_name_msg(img.ptr, ns, name, msg);
}

template <class T> inline T invoke_unbox(Method m, Object instance, std::span<void *> args) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    Object ret = m.invoke(instance, args);
    if (!ret.ptr)
        return T{};
    auto *val = object_unbox<T>(ret.ptr);
    return val ? *val : T{};
}

// ======================== Image/Class helpers ========================

inline Class class_from_name(const Image& img, const char *ns, const char *name) { return Class{il2cpp_class_from_name(img.ptr, ns, name)}; }

inline Class class_from_type(const Type& t) { return Class{il2cpp_class_from_type(t.ptr)}; }

// ======================== Domain initialization ========================

inline void set_config_dir(const char *path) { il2cpp_set_config_dir(path); }
inline void set_data_dir(const char *path) { il2cpp_set_data_dir(path); }
inline void set_temp_dir(const char *path) { il2cpp_set_temp_dir(path); }
inline void set_command_line(int argc, const char *const argv[], const char *basedir) { il2cpp_set_commandline_arguments(argc, argv, basedir); }
inline int init(const char *domain_name) { return il2cpp_init(domain_name); }
inline void shutdown() { il2cpp_shutdown(); }
} // namespace Il2Cpp::API
