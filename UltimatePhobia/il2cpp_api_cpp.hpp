#pragma once

#include "il2cpp_api.hpp"
#include "deobfuscations.hpp"

#include <string>
#include <memory>
#include <vector>
#include <span>
#include <source_location>
#include <format>
#include <utility>
#include <spdlog/spdlog.h>

namespace Il2Cpp::API {
// ======================== Errors ========================

struct Error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct ManagedException : Error {
    Il2CppException *ex{};
    explicit ManagedException(std::string msg, Il2CppException *e = nullptr) : Error(std::move(msg)), ex(e) {}
};

// Unique pointer for il2cpp_alloc/il2cpp_free buffers
struct Il2CppDeleter {
    void operator()(void *p) const noexcept {
        if (p)
            il2cpp_free(p);
    }
};

template <class T> using unique_il2cpp = std::unique_ptr<T, Il2CppDeleter>;

// Helper to format managed exceptions
std::string format_exception(Il2CppException *ex);
std::string format_stacktrace(Il2CppException *ex);

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
    Il2CppObject *ptr;
    std::shared_ptr<GcHandle> gc_handle;

    Object() : ptr(nullptr) {}
    Object(Il2CppObject *object) : ptr(object) {}
    explicit operator bool() const { return ptr != nullptr; }

    Class klass() const;
    uint32_t size() const { return il2cpp_object_get_size(ptr); }
    const MethodInfo *get_virtual_method(const MethodInfo *m) const { return il2cpp_object_get_virtual_method(ptr, m); }
    void *unbox_if_value();

    void lock_gc(bool pinned = false) { gc_handle = std::make_shared<GcHandle>(ptr, pinned); }
};

struct String : Object {
    String() = default;
    explicit String(Il2CppString *s) : Object(reinterpret_cast<Il2CppObject *>(s)) {}
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
    std::string to_utf8() const;

    Object object() const { return Object{*this}; }
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
    Class get_class(std::string_view namespaze, std::string_view name);
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

    bool is_compatible_with(Class klass) const;

    std::string name_owned() const {
        unique_il2cpp<char> p{il2cpp_type_get_name(ptr)};
        return p ? std::string(apply_name_deobfuscations(p.get())) : std::string{};
    }
    std::string assembly_qualified_name_owned() const {
        unique_il2cpp<char> p{il2cpp_type_get_assembly_qualified_name(ptr)};
        return p ? std::string(apply_name_deobfuscations(p.get())) : std::string{};
    }
    Class class_or_element() const;
    bool equals(Type other) const { return il2cpp_type_equals(ptr, other.ptr); }
};

struct Class {
    Il2CppClass *ptr{nullptr};
    explicit operator bool() const { return ptr != nullptr; }

    static Class from_type(const Il2CppType *t) { return Class{il2cpp_class_from_type(t)}; }
    static Class from_name(const Image& img, const char *ns, const char *name) {
        return Class{il2cpp_class_from_name(img.ptr, apply_name_obfuscations(ns), apply_name_obfuscations(name))};
    }

    std::string_view name() const { return apply_name_deobfuscations(il2cpp_class_get_name(ptr)); }
    std::string_view namespaze() const { return apply_name_deobfuscations(il2cpp_class_get_namespace(ptr)); }
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
    Field get_field(Class klass) const;
    Method get_method(const char *name, std::span<Class> args) const;
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

    std::string_view name() const { return apply_name_deobfuscations(il2cpp_method_get_name(ptr)); }
    Class declaring_type() const { return Class{il2cpp_method_get_declaring_type(ptr)}; }
    Class klass() const { return Class{il2cpp_method_get_class(ptr)}; }
    bool is_generic() const { return il2cpp_method_is_generic(ptr); }
    bool is_inflated() const { return il2cpp_method_is_inflated(ptr); }
    bool is_instance() const { return il2cpp_method_is_instance(ptr); }
    uint32_t param_count() const { return il2cpp_method_get_param_count(ptr); }
    Type param(uint32_t index) const { return Type{il2cpp_method_get_param(ptr, index)}; }
    std::string_view param_name(uint32_t index) const { return apply_name_deobfuscations(il2cpp_method_get_param_name(ptr, index)); }
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

    std::string_view name() const { return apply_name_deobfuscations(il2cpp_field_get_name(ptr)); }
    uint32_t flags() const { return il2cpp_field_get_flags(ptr); }
    Class parent() const { return Class{il2cpp_field_get_parent(ptr)}; }
    size_t offset() const { return il2cpp_field_get_offset(ptr); }
    Type type() const { return Type{il2cpp_field_get_type(ptr)}; }
    bool has_attribute(Class attr_class) const { return il2cpp_field_has_attribute(ptr, attr_class.ptr); }
    bool is_literal() const { return il2cpp_field_is_literal(ptr); }

    template <class T> T get_value(Object obj) const {
        T v{};
        il2cpp_field_get_value(obj.ptr, ptr, &v);
        return v;
    }
    Object get_value_object(Object obj) const { return Object{il2cpp_field_get_value_object(ptr, obj.ptr)}; }
    template <class T> void set_value(Object obj, const T& v) const {
        T copy = v;
        il2cpp_field_set_value(obj.ptr, ptr, &copy);
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
    void set_value_object(Object obj, Object value) const { il2cpp_field_set_value_object(obj.ptr, ptr, value.ptr); }
};

struct Property {
    PropertyInfo *ptr{nullptr};
    explicit operator bool() const { return ptr != nullptr; }

    std::string_view name() const { return apply_name_deobfuscations(il2cpp_property_get_name(ptr)); }
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
    static Array New(Class klass, std::size_t length) { return Array{il2cpp_array_new(klass.ptr, static_cast<uint32_t>(length))}; }
    static Array NewSpecific(Il2CppClass *arrayType, std::size_t length) { return Array{il2cpp_array_new_specific(arrayType, static_cast<uint32_t>(length))}; }
    uint32_t length() const { return il2cpp_array_length(ptr); }
    uint32_t byte_length() const { return il2cpp_array_get_byte_length(ptr); }

    Object object() const { return Object{reinterpret_cast<Il2CppObject *>(ptr)}; }
    Object object_at(size_t idx) const { return Object{reinterpret_cast<Il2CppObject **>(reinterpret_cast<System_Byte_array *>(ptr)->m_Items)[idx]}; }
};

// Implementations dependent on other wrappers

inline Class Type::class_or_element() const { return Class{il2cpp_type_get_class_or_element_class(ptr)}; }
inline Class Object::klass() const { return Class{il2cpp_object_get_class(ptr)}; }

inline Assembly Domain::open_assembly(const char *name) { return Assembly{il2cpp_domain_assembly_open(ptr, name)}; }

inline Image Assembly::image() const { return Image{il2cpp_assembly_get_image(ptr)}; }

inline Class Image::get_class(size_t index) const { return Class{const_cast<Il2CppClass *>(il2cpp_image_get_class(ptr, index))}; }

inline Field Class::get_field(const char *name) const { return Field{il2cpp_class_get_field_from_name(ptr, apply_name_obfuscations(name))}; }

inline Method Class::get_method(const char *name, int args) const {
    return Method{il2cpp_class_get_method_from_name(ptr, apply_name_obfuscations(name), args)};
}

inline Property Class::get_property(const char *name) const {
    return Property{const_cast<PropertyInfo *>(il2cpp_class_get_property_from_name(ptr, apply_name_obfuscations(name)))};
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
    return il2cpp_exception_from_name_msg(img.ptr, apply_name_obfuscations(ns), apply_name_obfuscations(name), msg);
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

inline Class class_from_name(const Image& img, const char *ns, const char *name) {
    return Class{il2cpp_class_from_name(img.ptr, apply_name_obfuscations(ns), apply_name_obfuscations(name))};
}

inline Class class_from_type(const Type& t) { return Class{il2cpp_class_from_type(t.ptr)}; }

// ======================== Domain initialization ========================

inline void set_config_dir(const char *path) { il2cpp_set_config_dir(path); }
inline void set_data_dir(const char *path) { il2cpp_set_data_dir(path); }
inline void set_temp_dir(const char *path) { il2cpp_set_temp_dir(path); }
inline void set_command_line(int argc, const char *const argv[], const char *basedir) { il2cpp_set_commandline_arguments(argc, argv, basedir); }
inline int init(const char *domain_name) { return il2cpp_init(domain_name); }
inline void shutdown() { il2cpp_shutdown(); }

// ======================== Method calling helpers ========================

template <size_t N> struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        for (unsigned it = 0; it != N; ++it)
            value[it] = str[it];
    }

    char value[N];
    const size_t length = N - 1;

    operator const char *() const { return value; }
    operator std::string_view() const { return {value, length}; }
};

template <StringLiteral AssemblyName> Image get_image_cached() {
    char full_name[AssemblyName.length + 5];
    size_t idx = 0;
    for (; idx != AssemblyName.length; ++idx)
        full_name[idx] = AssemblyName.value[idx];
    full_name[idx++] = '.';
    full_name[idx++] = 'd';
    full_name[idx++] = 'l';
    full_name[idx++] = 'l';
    full_name[idx++] = '\0';
    static Image fres = Domain::get().open_assembly(full_name).image();
    return fres;
}

template <StringLiteral AssemblyName, StringLiteral NamespaceName, StringLiteral ClassName> Class get_class_cached() {
    static Class fres = get_image_cached<AssemblyName>().get_class(NamespaceName, ClassName);
    return fres;
}

template <StringLiteral AssemblyName, StringLiteral NamespaceName, StringLiteral ClassName, StringLiteral MethodName, size_t ArgCount>
Method get_method_cached() {
    static Method fres = get_class_cached<AssemblyName, NamespaceName, ClassName>().get_method(MethodName, ArgCount);
    return fres;
}

template <typename T> Class value_class(T value) {
    if constexpr (std::is_same_v<T, std::nullptr_t>)
        return Class{};
    else if constexpr (std::is_same_v<T, Object>)
        return value.klass();
    else if constexpr (std::is_same_v<T, String> || std::is_same_v<T, Array>)
        return value.object().klass();
    else if constexpr (std::is_same_v<T, Il2CppObject *>)
        return Object{value}.klass();
    else if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string> || std::is_same_v<T, const char *>)
        return get_class_cached<"mscorlib", "System", "String">();
    else if constexpr (std::is_same_v<T, bool>)
        return get_class_cached<"mscorlib", "System", "Boolean">();
    else if constexpr (std::is_same_v<T, int8_t>)
        return get_class_cached<"mscorlib", "System", "SByte">();
    else if constexpr (std::is_same_v<T, uint8_t>)
        return get_class_cached<"mscorlib", "System", "Byte">();
    else if constexpr (std::is_same_v<T, int16_t>)
        return get_class_cached<"mscorlib", "System", "Int16">();
    else if constexpr (std::is_same_v<T, uint16_t>)
        return get_class_cached<"mscorlib", "System", "UInt16">();
    else if constexpr (std::is_same_v<T, int32_t>)
        return get_class_cached<"mscorlib", "System", "Int32">();
    else if constexpr (std::is_same_v<T, uint32_t>)
        return get_class_cached<"mscorlib", "System", "UInt32">();
    else if constexpr (std::is_same_v<T, int64_t>)
        return get_class_cached<"mscorlib", "System", "Int64">();
    else if constexpr (std::is_same_v<T, uint64_t>)
        return get_class_cached<"mscorlib", "System", "UInt64">();
    else if constexpr (std::is_pointer_v<T>)
        return Object{reinterpret_cast<Il2CppObject *>(value)}.klass();
    else
        static_assert(false, "Value is not boxable");
}

template <typename T> Object value_box(T value) {
    if constexpr (std::is_same_v<T, std::nullptr_t>)
        return Object{};
    else if constexpr (std::is_same_v<T, Object>)
        return value;
    else if constexpr (std::is_same_v<T, String> || std::is_same_v<T, Array>)
        return value.object();
    else if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string> || std::is_same_v<T, const char *>)
        return String::New(value);
    else if constexpr (std::is_pointer_v<T>)
        return Object{reinterpret_cast<Il2CppObject *>(value)};
    else
        return value_box(value_class<T>(value), value);
}

struct CachedMethodLookup {
    CachedMethodLookup(const char *methodName) : methodName(methodName) {}

    CachedMethodLookup(const CachedMethodLookup&) = delete;
    CachedMethodLookup& operator=(const CachedMethodLookup&) = delete;

    operator const char *() const { return methodName; }

    const char *const methodName;
    class Method cachedMethod;

};

template <typename NameT, typename... ArgsT> Object call(Class klass, Object __this, NameT& methodSel, ArgsT... args) {
    constexpr size_t ArgCount = sizeof...(ArgsT);
    using NameD = std::decay_t<NameT>;

    std::array<Il2CppObject *, ArgCount> csArgs{value_box(args).ptr...};
    std::array<Class, ArgCount> csArgClasses = {value_class(args)...};

    auto getMethodHandle = [&]() -> Method {
        if constexpr (std::is_same_v<NameD, CachedMethodLookup>) {
            auto& s = methodSel;
            if (!s.cachedMethod)
                s.cachedMethod = klass.get_method(methodSel.methodName, csArgClasses);
            return s.cachedMethod;
        } else if constexpr (std::is_convertible_v<NameD, const char *>) {
            const char *methodName = methodSel;
            return klass.get_method(methodSel, csArgClasses);
        } else {
            static_assert(false, "First argument must be const char* or CachedMethodLookup&");
        }
    };
    Method method = getMethodHandle();

    if (!method)
        throw Error(std::format("Attempted to call non-existent method '{}' on class '{}' using function '{}' with {} arguments",
                                static_cast<const char *>(methodSel), klass.name(), std::source_location::current().function_name(), ArgCount));

    return method.invoke_convert(__this, csArgs);
}

template <typename NameT, typename... ArgsT> Object call(Class klass, NameT&& methodSel, ArgsT... args) {
    return call(klass, {}, methodSel, std::forward<ArgsT>(args)...);
}

template <typename NameT, typename... ArgsT> Object call(Object __this, NameT&& methodSel, ArgsT... args) {
    constexpr size_t ArgCount = sizeof...(ArgsT);
    if (!__this)
        throw Error(std::format("Attempted to call method '{}' on null using function '{}' with {} arguments", static_cast<const char *>(methodSel),
                                std::source_location::current().function_name(), ArgCount));

    return call(__this.klass(), __this, methodSel, std::forward<ArgsT>(args)...);
}
} // namespace Il2Cpp::API
