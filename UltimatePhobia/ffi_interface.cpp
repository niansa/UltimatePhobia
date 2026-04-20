#include "ffi_interface.hpp"
#include "global_state.hpp"
#include "il2cpp_dynamic.hpp"
#include "il2cpp_cppinterop.hpp"
#include "il2cpp_api.hpp"
#include "il2cpp_api_cpp.hpp"
#include "deobfuscations.hpp"
#include "game_hook.hpp"
#include "ffi_loader.hpp"
#include "anycall.hpp"
#include "mods/base.hpp"

#include <format>
#include <map>
#include <exception>
#include <limits>
#include <imgui.h>

using namespace AnyCall;

namespace FFIInterface {
int32_t getFtableItemCount() { return getLocalFtableItemCount(); }

namespace {
template <std::integral HandleT, typename PtrT> class HandleCollection {
    HandleT last_id = 0;
    std::map<HandleT, PtrT *> objects;

public:
    HandleT add(PtrT *ptr) {
        if (ptr == nullptr)
            return 0;
        // Find empty handle ID
        HandleT fres = ++last_id;
        while (objects.find(fres) != objects.end())
            ++fres;
        if (fres == std::numeric_limits<HandleT>::max() - 1) {
            g.logger->warn("Ran out of object handles! Please consider dropping "
                           "some handles.");
            return -1;
        }
        objects[fres] = ptr;
        return fres;
    }
    PtrT *get(HandleT id) const {
        if (id == 0)
            return nullptr;
        auto res = objects.find(id);
        if (res == objects.end())
            return nullptr;
        return res->second;
    }

    void drop(HandleT id) {
        if (id > 0)
            objects.erase(id);
        if (id == last_id)
            --last_id;
    }

    WIBool isValid(HandleT id) const { return id == 0 || objects.find(id) != objects.end(); }
    int64_t getAddress(HandleT id) const { return reinterpret_cast<uintptr_t>(get(id)); }
    static inline HandleT getNull() { return 0; }
};

HandleCollection<ObjectHandle, void /*Il2Cpp::API::Il2CppObject*/> objectHandles;
HandleCollection<ValueHandle, void> valueHandles;
HandleCollection<ImageHandle, Il2Cpp::API::Il2CppImage> imageHandles;
HandleCollection<ClassHandle, Il2Cpp::API::Il2CppClass> classHandles;
HandleCollection<DomainHandle, Il2Cpp::API::Il2CppDomain> domainHandles;
HandleCollection<AssemblyHandle, Il2Cpp::API::Il2CppAssembly> assemblyHandles;
HandleCollection<TypeHandle, Il2Cpp::API::Il2CppType> typeHandles;
HandleCollection<MethodInfoHandle, Il2Cpp::API::MethodInfo> methodInfoHandles;
HandleCollection<FieldHandle, Il2Cpp::API::FieldInfo> fieldHandles;
HandleCollection<PropertyHandle, Il2Cpp::API::PropertyInfo> propertyHandles;
HandleCollection<EventHandle, Il2Cpp::API::EventInfo> eventHandles;

// Argument stack
std::vector<void *> call_args;
void *return_value;
std::string call_error;

void *getValue(int32_t index) {
    if (index < 0)
        return return_value;
    if (index > call_args.size())
        return nullptr;
    return call_args[index];
}
} // namespace

ObjectHandle getNull() { return 0; }

// Object handle management
void dropObject(ObjectHandle id) { objectHandles.drop(id); }
WIBool isValidObject(ObjectHandle id) { return objectHandles.isValid(id); }
int64_t getObjectAddress(ObjectHandle id) { return objectHandles.getAddress(id); }

// Misc handle management
void dropValue(ValueHandle id) { valueHandles.drop(id); }
void dropImage(ImageHandle id) { imageHandles.drop(id); }
void dropClass(ClassHandle id) { classHandles.drop(id); }
void dropDomain(DomainHandle id) { domainHandles.drop(id); }
void dropAssembly(AssemblyHandle id) { assemblyHandles.drop(id); }
void dropType(TypeHandle id) { typeHandles.drop(id); }
void dropMethodInfo(MethodInfoHandle id) { methodInfoHandles.drop(id); }
void dropField(FieldHandle id) { fieldHandles.drop(id); }
void dropProperty(PropertyHandle id) { propertyHandles.drop(id); }
void dropEvent(EventHandle id) { eventHandles.drop(id); }

// String creation
ObjectHandle toCsString(const char *str) { return objectHandles.add(Il2Cpp::CppInterop::ToCsString(str)); }
ObjectHandle toCsStringWithLength(const char *str, int32_t length) {
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString({str, static_cast<size_t>(length)}));
}
void toCString(ObjectHandle str, char *buf, int32_t maxlen) {
    Il2Cpp::CppInterop::ToCString(reinterpret_cast<System_String_o *>(objectHandles.get(str)), buf, maxlen);
}

// C# runtime wrappers
ImageHandle getImageCorlib() {
    return imageHandles.add(const_cast<Il2Cpp::API::Il2CppImage *>(
        reinterpret_cast<const Il2Cpp::API::Il2CppImage *>(reinterpret_cast<const void *>(Il2Cpp::API::il2cpp_get_corlib()))));
}
ClassHandle getClassFromName(ImageHandle image, const char *namespaze, const char *name) {
    return classHandles.add(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(Il2Cpp::API::il2cpp_class_from_name(
        reinterpret_cast<Il2Cpp::API::Il2CppImage *>(imageHandles.get(image)), apply_name_obfuscations(namespaze), apply_name_obfuscations(name))));
}

// Arrays
namespace {
template <typename RetT, typename Fn> RetT *iterateNth(Il2Cpp::API::Il2CppClass *klass, int32_t index, Fn fn) {
    if (!klass || index < 0)
        return nullptr;
    void *iter = nullptr;
    RetT *res = nullptr;
    for (int32_t i = 0; i <= index; ++i) {
        res = const_cast<RetT *>(fn(klass, &iter));
        if (!res)
            return nullptr;
    }
    return res;
}
} // namespace
ClassHandle getArrayFromClass(ClassHandle elementClass, int32_t rank) {
    return classHandles.add(Il2Cpp::API::il2cpp_array_class_get(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(elementClass)), rank));
}
ObjectHandle createArray(ClassHandle elementClass, int32_t length) {
    return objectHandles.add(
        reinterpret_cast<void *>(Il2Cpp::API::il2cpp_array_new(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(elementClass)), length)));
}
void copyArrayBytes(ObjectHandle array, int32_t offset, int32_t length, void *to) {
    auto csArray = reinterpret_cast<System_Byte_array *>(objectHandles.get(array));
    if (csArray)
        memcpy(to, csArray->m_Items + offset, length);
}
int32_t arrayGetLength(ObjectHandle array) {
    return static_cast<int32_t>(Il2Cpp::API::il2cpp_array_length(reinterpret_cast<Il2Cpp::API::Il2CppArray *>(objectHandles.get(array))));
}
int32_t arrayGetByteLength(ObjectHandle array) {
    return static_cast<int32_t>(Il2Cpp::API::il2cpp_array_get_byte_length(reinterpret_cast<Il2Cpp::API::Il2CppArray *>(objectHandles.get(array))));
}
ObjectHandle createArraySpecific(ClassHandle arrayTypeInfo, int32_t length) {
    auto *arr = Il2Cpp::API::il2cpp_array_new_specific(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(arrayTypeInfo)), length);
    return objectHandles.add(arr);
}
ClassHandle getBoundedArrayClass(ClassHandle elementClass, int32_t rank, WIBool bounded) {
    return classHandles.add(
        Il2Cpp::API::il2cpp_bounded_array_class_get(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(elementClass)), rank, bounded));
}
int32_t arrayElementSize(ClassHandle arrayClass) {
    return Il2Cpp::API::il2cpp_array_element_size(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(arrayClass)));
}

// Domains and Assemblies
DomainHandle domainGet() { return domainHandles.add(Il2Cpp::API::il2cpp_domain_get()); }
AssemblyHandle domainAssemblyOpen(DomainHandle domain, const char *name) {
    return assemblyHandles.add(const_cast<Il2Cpp::API::Il2CppAssembly *>(reinterpret_cast<const Il2Cpp::API::Il2CppAssembly *>(
        Il2Cpp::API::il2cpp_domain_assembly_open(reinterpret_cast<Il2Cpp::API::Il2CppDomain *>(domainHandles.get(domain)), name))));
}
int32_t domainGetAssemblyCount(DomainHandle domain) {
    size_t size = 0;
    (void)Il2Cpp::API::il2cpp_domain_get_assemblies(reinterpret_cast<const Il2Cpp::API::Il2CppDomain *>(domainHandles.get(domain)), &size);
    return static_cast<int32_t>(size);
}
AssemblyHandle domainGetAssemblyAt(DomainHandle domain, int32_t index) {
    size_t size = 0;
    auto assemblies = Il2Cpp::API::il2cpp_domain_get_assemblies(reinterpret_cast<const Il2Cpp::API::Il2CppDomain *>(domainHandles.get(domain)), &size);
    if (!assemblies || index < 0 || static_cast<size_t>(index) >= size)
        return 0;
    return assemblyHandles.add(const_cast<Il2Cpp::API::Il2CppAssembly *>(assemblies[index]));
}
ImageHandle assemblyGetImage(AssemblyHandle assembly) {
    return imageHandles.add(const_cast<Il2Cpp::API::Il2CppImage *>(
        Il2Cpp::API::il2cpp_assembly_get_image(reinterpret_cast<const Il2Cpp::API::Il2CppAssembly *>(assemblyHandles.get(assembly)))));
}

// Classes
TypeHandle classEnumBaseType(ClassHandle klass) {
    return typeHandles.add(const_cast<Il2Cpp::API::Il2CppType *>(Il2Cpp::API::il2cpp_class_enum_basetype(
        const_cast<Il2Cpp::API::Il2CppClass *>(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass))))));
}
WIBool classIsGeneric(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_is_generic(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
WIBool classIsInflated(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_is_inflated(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
WIBool classIsAssignableFrom(ClassHandle klass, ClassHandle oklass) {
    return Il2Cpp::API::il2cpp_class_is_assignable_from(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)),
                                                        reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(oklass)));
}
WIBool classIsSubclassOf(ClassHandle klass, ClassHandle klassc, WIBool checkInterfaces) {
    return Il2Cpp::API::il2cpp_class_is_subclass_of(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)),
                                                    reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klassc)), checkInterfaces);
}
WIBool classHasParent(ClassHandle klass, ClassHandle klassc) {
    return Il2Cpp::API::il2cpp_class_has_parent(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)),
                                                reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klassc)));
}
ClassHandle getClassFromType(TypeHandle type) {
    return classHandles.add(Il2Cpp::API::il2cpp_class_from_il2cpp_type(reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(type))));
}
ClassHandle getClassFromSystemType(ObjectHandle reflectionType) {
    return classHandles.add(
        Il2Cpp::API::il2cpp_class_from_system_type(reinterpret_cast<Il2Cpp::API::Il2CppReflectionType *>(objectHandles.get(reflectionType))));
}
ClassHandle classGetElementClass(ClassHandle klass) {
    return classHandles.add(Il2Cpp::API::il2cpp_class_get_element_class(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass))));
}
ObjectHandle classGetName(ClassHandle klass) {
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(
        apply_name_deobfuscations(Il2Cpp::API::il2cpp_class_get_name(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass))))));
}
ObjectHandle classGetNamespace(ClassHandle klass) {
    return objectHandles.add(
        Il2Cpp::CppInterop::ToCsString(Il2Cpp::API::il2cpp_class_get_namespace(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)))));
}
ClassHandle classGetParent(ClassHandle klass) {
    return classHandles.add(Il2Cpp::API::il2cpp_class_get_parent(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass))));
}
ClassHandle classGetDeclaringType(ClassHandle klass) {
    return classHandles.add(Il2Cpp::API::il2cpp_class_get_declaring_type(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass))));
}
int32_t classInstanceSize(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_instance_size(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
int32_t classNumFields(ClassHandle klass) {
    return static_cast<int32_t>(Il2Cpp::API::il2cpp_class_num_fields(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass))));
}
WIBool classIsValueType(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_is_valuetype(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
WIBool classIsBlittable(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_is_blittable(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
int32_t classGetFlags(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_get_flags(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
WIBool classIsAbstract(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_is_abstract(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
WIBool classIsInterface(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_is_interface(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
int32_t classArrayElementSize(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_array_element_size(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
TypeHandle classGetType(ClassHandle klass) {
    return typeHandles.add(
        const_cast<Il2Cpp::API::Il2CppType *>(Il2Cpp::API::il2cpp_class_get_type(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)))));
}
int32_t classGetTypeToken(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_get_type_token(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
WIBool classHasAttribute(ClassHandle klass, ClassHandle attrClass) {
    return Il2Cpp::API::il2cpp_class_has_attribute(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)),
                                                   reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(attrClass)));
}
WIBool classHasReferences(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_has_references(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
WIBool classIsEnum(ClassHandle klass) { return Il2Cpp::API::il2cpp_class_is_enum(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass))); }
ImageHandle classGetImage(ClassHandle klass) {
    return imageHandles.add(
        const_cast<Il2Cpp::API::Il2CppImage *>(Il2Cpp::API::il2cpp_class_get_image(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)))));
}
ObjectHandle classGetAssemblyName(ClassHandle klass) {
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(
        Il2Cpp::API::il2cpp_class_get_assemblyname(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)))));
}
int32_t classGetRank(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_get_rank(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
int32_t classGetDataSize(ClassHandle klass) {
    return Il2Cpp::API::il2cpp_class_get_data_size(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)));
}
EventHandle classGetEventAt(ClassHandle klass, int32_t index) {
    auto *res = iterateNth<Il2Cpp::API::EventInfo>(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), index,
                                                   [](auto *k, void **it) { return Il2Cpp::API::il2cpp_class_get_events(k, it); });
    return eventHandles.add(const_cast<Il2Cpp::API::EventInfo *>(res));
}
FieldHandle classGetFieldAt(ClassHandle klass, int32_t index) {
    auto *res = iterateNth<Il2Cpp::API::FieldInfo>(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), index,
                                                   [](auto *k, void **it) { return Il2Cpp::API::il2cpp_class_get_fields(k, it); });
    return fieldHandles.add(res);
}
ClassHandle classGetNestedTypeAt(ClassHandle klass, int32_t index) {
    auto *res = iterateNth<Il2Cpp::API::Il2CppClass>(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), index,
                                                     [](auto *k, void **it) { return Il2Cpp::API::il2cpp_class_get_nested_types(k, it); });
    return classHandles.add(res);
}
ClassHandle classGetInterfaceAt(ClassHandle klass, int32_t index) {
    auto *res = iterateNth<Il2Cpp::API::Il2CppClass>(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), index,
                                                     [](auto *k, void **it) { return Il2Cpp::API::il2cpp_class_get_interfaces(k, it); });
    return classHandles.add(res);
}
PropertyHandle classGetPropertyAt(ClassHandle klass, int32_t index) {
    auto *res = iterateNth<Il2Cpp::API::PropertyInfo>(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), index,
                                                      [](auto *k, void **it) { return Il2Cpp::API::il2cpp_class_get_properties(k, it); });
    return propertyHandles.add(const_cast<Il2Cpp::API::PropertyInfo *>(res));
}
MethodInfoHandle classGetMethodAt(ClassHandle klass, int32_t index) {
    auto *res = iterateNth<Il2Cpp::API::MethodInfo>(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), index,
                                                    [](auto *k, void **it) { return Il2Cpp::API::il2cpp_class_get_methods(k, it); });
    return methodInfoHandles.add(const_cast<Il2Cpp::API::MethodInfo *>(res));
}
PropertyHandle classGetPropertyFromName(ClassHandle klass, const char *name) {
    return propertyHandles.add(const_cast<Il2Cpp::API::PropertyInfo *>(Il2Cpp::API::il2cpp_class_get_property_from_name(
        reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), apply_name_obfuscations(name))));
}
FieldHandle classGetFieldFromName(ClassHandle klass, const char *name) {
    return fieldHandles.add(
        Il2Cpp::API::il2cpp_class_get_field_from_name(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), apply_name_obfuscations(name)));
}
MethodInfoHandle classGetMethodFromName(ClassHandle klass, const char *name, int32_t argCount) {
    return methodInfoHandles.add(const_cast<Il2Cpp::API::MethodInfo *>(Il2Cpp::API::il2cpp_class_get_method_from_name(
        reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), apply_name_obfuscations(name), argCount)));
}
MethodInfoHandle classGetMethodFromNameAndTypes(ClassHandle klass, const char *name, int32_t argCount) {
    // Handle unknown argument count
    if (argCount == unknownArgCount)
        argCount = call_args.size();

    // Make sure argCount matches argument count
    else if (argCount != call_args.size()) {
        call_error = std::format("Mismatched added ({}) vs. passed ({}) arg count", call_args.size(), argCount);
        g.logger->warn("FFI interface failed to find function '{}': {}", name, call_error);
        return -1;
    }

    // Build vector
    std::vector<Il2Cpp::API::Class> argClasses;
    for (size_t idx = 0; idx != argCount; ++idx)
        argClasses.emplace_back(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(call_args[idx]));

    // Get class pointer
    Il2Cpp::API::Class il2cppKlass{classHandles.get(klass)};
    if (!il2cppKlass) {
        call_error = std::format("null passed as parent class, don't know where to look", call_args.size(), argCount);
        g.logger->warn("FFI interface failed to find function '{}': {}", name, call_error);
        return -1;
    }

    // Reuse existing code from C++ wrapper
    auto fres = il2cppKlass.get_method(name, argClasses).ptr;

    // Return result
    return methodInfoHandles.add(const_cast<Il2Cpp::API::MethodInfo *>(fres));
}

// Exceptions
void exceptionRaise(ObjectHandle ex) { Il2Cpp::API::il2cpp_raise_exception(reinterpret_cast<Il2Cpp::API::Il2CppException *>(objectHandles.get(ex))); }
ObjectHandle exceptionFromNameMsg(ImageHandle image, const char *namespaze, const char *name, const char *msg) {
    return objectHandles.add(Il2Cpp::API::il2cpp_exception_from_name_msg(reinterpret_cast<const Il2Cpp::API::Il2CppImage *>(imageHandles.get(image)),
                                                                         apply_name_obfuscations(namespaze), apply_name_obfuscations(name), msg));
}
ObjectHandle exceptionGetArgumentNull(const char *arg) { return objectHandles.add(Il2Cpp::API::il2cpp_get_exception_argument_null(arg)); }
ObjectHandle exceptionFormat(ObjectHandle ex) {
    char buf[2048] = {};
    Il2Cpp::API::il2cpp_format_exception(reinterpret_cast<const Il2Cpp::API::Il2CppException *>(objectHandles.get(ex)), buf, sizeof(buf));
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(buf));
}
ObjectHandle exceptionFormatStackTrace(ObjectHandle ex) {
    char buf[4096] = {};
    Il2Cpp::API::il2cpp_format_stack_trace(reinterpret_cast<const Il2Cpp::API::Il2CppException *>(objectHandles.get(ex)), buf, sizeof(buf));
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(buf));
}
void exceptionUnhandled(ObjectHandle ex) { Il2Cpp::API::il2cpp_unhandled_exception(reinterpret_cast<Il2Cpp::API::Il2CppException *>(objectHandles.get(ex))); }

// Fields
int32_t fieldGetFlags(FieldHandle field) { return Il2Cpp::API::il2cpp_field_get_flags(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field))); }
ObjectHandle fieldGetName(FieldHandle field) {
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(
        apply_name_deobfuscations(Il2Cpp::API::il2cpp_field_get_name(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field))))));
}
ClassHandle fieldGetParent(FieldHandle field) {
    return classHandles.add(Il2Cpp::API::il2cpp_field_get_parent(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field))));
}
int32_t fieldGetOffset(FieldHandle field) {
    return static_cast<int32_t>(Il2Cpp::API::il2cpp_field_get_offset(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field))));
}
TypeHandle fieldGetType(FieldHandle field) {
    return typeHandles.add(
        const_cast<Il2Cpp::API::Il2CppType *>(Il2Cpp::API::il2cpp_field_get_type(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)))));
}
WIBool fieldHasAttribute(FieldHandle field, ClassHandle attrClass) {
    return Il2Cpp::API::il2cpp_field_has_attribute(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)),
                                                   reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(attrClass)));
}
void fieldSetValueI32(ObjectHandle obj, FieldHandle field, int32_t value) {
    Il2Cpp::API::il2cpp_field_set_value(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)),
                                        reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &value);
}
void fieldSetValueI64(ObjectHandle obj, FieldHandle field, int64_t value) {
    Il2Cpp::API::il2cpp_field_set_value(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)),
                                        reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &value);
}
void fieldSetValueFloat(ObjectHandle obj, FieldHandle field, float value) {
    Il2Cpp::API::il2cpp_field_set_value(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)),
                                        reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &value);
}
void fieldSetValueDouble(ObjectHandle obj, FieldHandle field, double value) {
    Il2Cpp::API::il2cpp_field_set_value(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)),
                                        reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &value);
}
void fieldSetValueObject(ObjectHandle obj, FieldHandle field, ObjectHandle value) {
    Il2Cpp::API::il2cpp_field_set_value_object(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)),
                                               reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)),
                                               reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(value)));
}
int32_t fieldGetValueI32(ObjectHandle obj, FieldHandle field) {
    int32_t v = 0;
    Il2Cpp::API::il2cpp_field_get_value(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)),
                                        reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
    return v;
}
int64_t fieldGetValueI64(ObjectHandle obj, FieldHandle field) {
    int64_t v = 0;
    Il2Cpp::API::il2cpp_field_get_value(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)),
                                        reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
    return v;
}
float fieldGetValueFloat(ObjectHandle obj, FieldHandle field) {
    float v = 0;
    Il2Cpp::API::il2cpp_field_get_value(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)),
                                        reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
    return v;
}
double fieldGetValueDouble(ObjectHandle obj, FieldHandle field) {
    double v = 0;
    Il2Cpp::API::il2cpp_field_get_value(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)),
                                        reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
    return v;
}
ObjectHandle fieldGetValueObject(ObjectHandle obj, FieldHandle field) {
    return objectHandles.add(Il2Cpp::API::il2cpp_field_get_value_object(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)),
                                                                        reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))));
}
int32_t fieldStaticGetValueI32(FieldHandle field) {
    int32_t v = 0;
    Il2Cpp::API::il2cpp_field_static_get_value(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
    return v;
}
int64_t fieldStaticGetValueI64(FieldHandle field) {
    int64_t v = 0;
    Il2Cpp::API::il2cpp_field_static_get_value(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
    return v;
}
float fieldStaticGetValueFloat(FieldHandle field) {
    float v = 0;
    Il2Cpp::API::il2cpp_field_static_get_value(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
    return v;
}
double fieldStaticGetValueDouble(FieldHandle field) {
    double v = 0;
    Il2Cpp::API::il2cpp_field_static_get_value(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
    return v;
}
void fieldStaticSetValueI32(FieldHandle field, int32_t v) {
    Il2Cpp::API::il2cpp_field_static_set_value(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
}
void fieldStaticSetValueI64(FieldHandle field, int64_t v) {
    Il2Cpp::API::il2cpp_field_static_set_value(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
}
void fieldStaticSetValueFloat(FieldHandle field, float v) {
    Il2Cpp::API::il2cpp_field_static_set_value(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
}
void fieldStaticSetValueDouble(FieldHandle field, double v) {
    Il2Cpp::API::il2cpp_field_static_set_value(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field)), &v);
}
WIBool fieldIsLiteral(FieldHandle field) { return Il2Cpp::API::il2cpp_field_is_literal(reinterpret_cast<Il2Cpp::API::FieldInfo *>(fieldHandles.get(field))); }

// Garbage collection
GCHandle gcCreateHandle(ObjectHandle object, WIBool pinned) {
    auto ptr = objectHandles.get(object);
    if (ptr == nullptr)
        return -1;
    return Il2Cpp::API::il2cpp_gchandle_new(reinterpret_cast<Il2CppObject *>(ptr), pinned);
}
void gcDeleteHandle(GCHandle handle) { Il2Cpp::API::il2cpp_gchandle_free(handle); }
void gcCollect(int32_t maxGenerations) { Il2Cpp::API::il2cpp_gc_collect(maxGenerations); }
int32_t gcCollectALittle() { return Il2Cpp::API::il2cpp_gc_collect_a_little(); }
void gcDisable() { Il2Cpp::API::il2cpp_gc_disable(); }
void gcEnable() { Il2Cpp::API::il2cpp_gc_enable(); }
WIBool gcIsDisabled() { return Il2Cpp::API::il2cpp_gc_is_disabled(); }
int64_t gcGetMaxTimeSliceNs() { return Il2Cpp::API::il2cpp_gc_get_max_time_slice_ns(); }
void gcSetMaxTimeSliceNs(int64_t ns) { Il2Cpp::API::il2cpp_gc_set_max_time_slice_ns(ns); }
WIBool gcIsIncremental() { return Il2Cpp::API::il2cpp_gc_is_incremental(); }
int64_t gcGetUsedSize() { return Il2Cpp::API::il2cpp_gc_get_used_size(); }
int64_t gcGetHeapSize() { return Il2Cpp::API::il2cpp_gc_get_heap_size(); }
WIBool gcHasStrictWBarriers() { return Il2Cpp::API::il2cpp_gc_has_strict_wbarriers(); }
void gcStopWorld() { Il2Cpp::API::il2cpp_stop_gc_world(); }
void gcStartWorld() { Il2Cpp::API::il2cpp_start_gc_world(); }
GCHandle gcCreateWeakHandle(ObjectHandle object, WIBool trackResurrection) {
    auto *obj = reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(object));
    if (!obj)
        return -1;
    return Il2Cpp::API::il2cpp_gchandle_new_weakref(obj, trackResurrection);
}
ObjectHandle gcGetTarget(GCHandle handle) { return objectHandles.add(Il2Cpp::API::il2cpp_gchandle_get_target(handle)); }

// Method infos
TypeHandle methodInfoGetReturnType(MethodInfoHandle method) {
    return typeHandles.add(const_cast<Il2Cpp::API::Il2CppType *>(
        Il2Cpp::API::il2cpp_method_get_return_type(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)))));
}
ClassHandle methodInfoGetDeclaringType(MethodInfoHandle method) {
    return classHandles.add(Il2Cpp::API::il2cpp_method_get_declaring_type(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method))));
}
ObjectHandle methodInfoGetName(MethodInfoHandle method) {
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(
        apply_name_deobfuscations(Il2Cpp::API::il2cpp_method_get_name(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method))))));
}
MethodInfoHandle methodInfoGetFromReflection(ObjectHandle reflectionMethod) {
    return methodInfoHandles.add(const_cast<Il2Cpp::API::MethodInfo *>(
        Il2Cpp::API::il2cpp_method_get_from_reflection(reinterpret_cast<const Il2Cpp::API::Il2CppReflectionMethod *>(objectHandles.get(reflectionMethod)))));
}
ObjectHandle methodInfoGetObject(MethodInfoHandle method, ClassHandle refclass) {
    return objectHandles.add(Il2Cpp::API::il2cpp_method_get_object(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)),
                                                                   reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(refclass))));
}
WIBool methodInfoIsGeneric(MethodInfoHandle method) {
    return Il2Cpp::API::il2cpp_method_is_generic(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)));
}
WIBool methodInfoIsInflated(MethodInfoHandle method) {
    return Il2Cpp::API::il2cpp_method_is_inflated(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)));
}
WIBool methodInfoIsInstance(MethodInfoHandle method) {
    return Il2Cpp::API::il2cpp_method_is_instance(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)));
}
int32_t methodInfoGetParamCount(MethodInfoHandle method) {
    return static_cast<int32_t>(Il2Cpp::API::il2cpp_method_get_param_count(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method))));
}
TypeHandle methodInfoGetParamType(MethodInfoHandle method, int32_t index) {
    return typeHandles.add(const_cast<Il2Cpp::API::Il2CppType *>(
        Il2Cpp::API::il2cpp_method_get_param(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)), index)));
}
ClassHandle methodInfoGetClass(MethodInfoHandle method) {
    return classHandles.add(Il2Cpp::API::il2cpp_method_get_class(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method))));
}
WIBool methodInfoHasAttribute(MethodInfoHandle method, ClassHandle attrClass) {
    return Il2Cpp::API::il2cpp_method_has_attribute(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)),
                                                    reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(attrClass)));
}
int32_t methodInfoGetFlags(MethodInfoHandle method) {
    uint32_t impl = 0;
    return Il2Cpp::API::il2cpp_method_get_flags(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)), &impl);
}
int32_t methodInfoGetImplFlags(MethodInfoHandle method) {
    uint32_t impl = 0;
    (void)Il2Cpp::API::il2cpp_method_get_flags(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)), &impl);
    return impl;
}
int32_t methodInfoGetToken(MethodInfoHandle method) {
    return Il2Cpp::API::il2cpp_method_get_token(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)));
}
ObjectHandle methodInfoGetParamName(MethodInfoHandle method, int32_t index) {
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(
        Il2Cpp::API::il2cpp_method_get_param_name(reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)), index)));
}

// Properties
int32_t propertyGetFlags(PropertyHandle prop) {
    return Il2Cpp::API::il2cpp_property_get_flags(reinterpret_cast<Il2Cpp::API::PropertyInfo *>(propertyHandles.get(prop)));
}
MethodInfoHandle propertyGetGetMethod(PropertyHandle prop) {
    return methodInfoHandles.add(const_cast<Il2Cpp::API::MethodInfo *>(
        Il2Cpp::API::il2cpp_property_get_get_method(reinterpret_cast<Il2Cpp::API::PropertyInfo *>(propertyHandles.get(prop)))));
}
MethodInfoHandle propertyGetSetMethod(PropertyHandle prop) {
    return methodInfoHandles.add(const_cast<Il2Cpp::API::MethodInfo *>(
        Il2Cpp::API::il2cpp_property_get_set_method(reinterpret_cast<Il2Cpp::API::PropertyInfo *>(propertyHandles.get(prop)))));
}
ObjectHandle propertyGetName(PropertyHandle prop) {
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(
        apply_name_deobfuscations(Il2Cpp::API::il2cpp_property_get_name(reinterpret_cast<Il2Cpp::API::PropertyInfo *>(propertyHandles.get(prop))))));
}
ClassHandle propertyGetParent(PropertyHandle prop) {
    return classHandles.add(Il2Cpp::API::il2cpp_property_get_parent(reinterpret_cast<Il2Cpp::API::PropertyInfo *>(propertyHandles.get(prop))));
}

// Objects
ClassHandle objectGetClass(ObjectHandle obj) {
    return classHandles.add(Il2Cpp::API::il2cpp_object_get_class(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))));
}
int32_t objectGetSize(ObjectHandle obj) { return Il2Cpp::API::il2cpp_object_get_size(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))); }
MethodInfoHandle objectGetVirtualMethod(ObjectHandle obj, MethodInfoHandle method) {
    return methodInfoHandles.add(const_cast<Il2Cpp::API::MethodInfo *>(
        Il2Cpp::API::il2cpp_object_get_virtual_method(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)),
                                                      reinterpret_cast<const Il2Cpp::API::MethodInfo *>(methodInfoHandles.get(method)))));
}
ObjectHandle objectNew(ClassHandle klass) {
    return objectHandles.add(Il2Cpp::API::il2cpp_object_new(reinterpret_cast<const Il2Cpp::API::Il2CppClass *>(classHandles.get(klass))));
}
ValueHandle objectUnboxValue(ObjectHandle obj) {
    return valueHandles.add(Il2Cpp::API::il2cpp_object_unbox(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))));
}
int32_t objectUnboxI32(ObjectHandle obj) {
    int32_t v = 0;
    if (auto p = Il2Cpp::API::il2cpp_object_unbox(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))))
        std::memcpy(&v, p, sizeof(v));
    return v;
}
int64_t objectUnboxI64(ObjectHandle obj) {
    int64_t v = 0;
    if (auto p = Il2Cpp::API::il2cpp_object_unbox(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))))
        std::memcpy(&v, p, sizeof(v));
    return v;
}
float objectUnboxFloat(ObjectHandle obj) {
    float v = 0;
    if (auto p = Il2Cpp::API::il2cpp_object_unbox(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))))
        std::memcpy(&v, p, sizeof(v));
    return v;
}
double objectUnboxDouble(ObjectHandle obj) {
    double v = 0;
    if (auto p = Il2Cpp::API::il2cpp_object_unbox(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))))
        std::memcpy(&v, p, sizeof(v));
    return v;
}
ObjectHandle valueBox(ClassHandle klass, ValueHandle value) {
    return objectHandles.add(Il2Cpp::API::il2cpp_value_box(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), valueHandles.get(value)));
}
ObjectHandle valueBoxI32(ClassHandle klass, int32_t v) {
    return objectHandles.add(Il2Cpp::API::il2cpp_value_box(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), &v));
}
ObjectHandle valueBoxI64(ClassHandle klass, int64_t v) {
    return objectHandles.add(Il2Cpp::API::il2cpp_value_box(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), &v));
}
ObjectHandle valueBoxFloat(ClassHandle klass, float v) {
    return objectHandles.add(Il2Cpp::API::il2cpp_value_box(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), &v));
}
ObjectHandle valueBoxDouble(ClassHandle klass, double v) {
    return objectHandles.add(Il2Cpp::API::il2cpp_value_box(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass)), &v));
}

// Monitors
void monitorEnter(ObjectHandle obj) { Il2Cpp::API::il2cpp_monitor_enter(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))); }
WIBool monitorTryEnter(ObjectHandle obj, int32_t timeout) {
    return Il2Cpp::API::il2cpp_monitor_try_enter(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)), timeout);
}
void monitorExit(ObjectHandle obj) { Il2Cpp::API::il2cpp_monitor_exit(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))); }
void monitorPulse(ObjectHandle obj) { Il2Cpp::API::il2cpp_monitor_pulse(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))); }
void monitorPulseAll(ObjectHandle obj) { Il2Cpp::API::il2cpp_monitor_pulse_all(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))); }
void monitorWait(ObjectHandle obj) { Il2Cpp::API::il2cpp_monitor_wait(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))); }
WIBool monitorTryWait(ObjectHandle obj, int32_t timeout) {
    return Il2Cpp::API::il2cpp_monitor_try_wait(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)), timeout);
}

// Runtime
void runtimeClassInit(ClassHandle klass) { Il2Cpp::API::il2cpp_runtime_class_init(reinterpret_cast<Il2Cpp::API::Il2CppClass *>(classHandles.get(klass))); }
void runtimeObjectInit(ObjectHandle obj) { Il2Cpp::API::il2cpp_runtime_object_init(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj))); }
WIBool runtimeObjectInitException(ObjectHandle obj) {
    Il2Cpp::API::Il2CppException *exc = nullptr;
    Il2Cpp::API::il2cpp_runtime_object_init_exception(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(objectHandles.get(obj)), &exc);
    if (exc) {
        char buf[2048] = {};
        Il2Cpp::API::il2cpp_format_exception(exc, buf, sizeof(buf));
        g.logger->error("runtime_object_init_exception: {}", buf);
        return false;
    }
    return true;
}
void runtimeUnhandledExceptionPolicySet(int32_t value) {
    Il2Cpp::API::il2cpp_runtime_unhandled_exception_policy_set(static_cast<Il2Cpp::API::Il2CppRuntimeUnhandledExceptionPolicy>(value));
}

// Strings
int32_t stringGetLength(ObjectHandle str) { return Il2Cpp::API::il2cpp_string_length(reinterpret_cast<Il2Cpp::API::Il2CppString *>(objectHandles.get(str))); }
ObjectHandle stringIntern(ObjectHandle str) {
    return objectHandles.add(Il2Cpp::API::il2cpp_string_intern(reinterpret_cast<Il2Cpp::API::Il2CppString *>(objectHandles.get(str))));
}
ObjectHandle stringIsInterned(ObjectHandle str) {
    return objectHandles.add(Il2Cpp::API::il2cpp_string_is_interned(reinterpret_cast<Il2Cpp::API::Il2CppString *>(objectHandles.get(str))));
}

// Types
ObjectHandle typeGetObject(TypeHandle type) {
    return objectHandles.add(Il2Cpp::API::il2cpp_type_get_object(reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(type))));
}
int32_t typeGetType(TypeHandle type) { return Il2Cpp::API::il2cpp_type_get_type(reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(type))); }
ClassHandle typeGetClassOrElementClass(TypeHandle type) {
    return classHandles.add(Il2Cpp::API::il2cpp_type_get_class_or_element_class(reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(type))));
}
ObjectHandle typeGetName(TypeHandle type) {
    char *name = Il2Cpp::API::il2cpp_type_get_name(reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(type)));
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(name ? apply_name_deobfuscations(name) : ""));
}
WIBool typeIsByRef(TypeHandle type) { return Il2Cpp::API::il2cpp_type_is_byref(reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(type))); }
int32_t typeGetAttrs(TypeHandle type) { return Il2Cpp::API::il2cpp_type_get_attrs(reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(type))); }
WIBool typeEquals(TypeHandle a, TypeHandle b) {
    return Il2Cpp::API::il2cpp_type_equals(reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(a)),
                                           reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(b)));
}
ObjectHandle typeGetAssemblyQualifiedName(TypeHandle type) {
    char *name = Il2Cpp::API::il2cpp_type_get_assembly_qualified_name(reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(type)));
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(name ? name : ""));
}
WIBool typeIsStatic(TypeHandle type) { return Il2Cpp::API::il2cpp_type_is_static(reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(type))); }
WIBool typeIsPointerType(TypeHandle type) {
    return Il2Cpp::API::il2cpp_type_is_pointer_type(reinterpret_cast<const Il2Cpp::API::Il2CppType *>(typeHandles.get(type)));
}

// Images
AssemblyHandle imageGetAssembly(ImageHandle image) {
    return assemblyHandles.add(const_cast<Il2Cpp::API::Il2CppAssembly *>(
        Il2Cpp::API::il2cpp_image_get_assembly(reinterpret_cast<const Il2Cpp::API::Il2CppImage *>(imageHandles.get(image)))));
}
ObjectHandle imageGetName(ImageHandle image) {
    return objectHandles.add(
        Il2Cpp::CppInterop::ToCsString(Il2Cpp::API::il2cpp_image_get_name(reinterpret_cast<const Il2Cpp::API::Il2CppImage *>(imageHandles.get(image)))));
}
ObjectHandle imageGetFilename(ImageHandle image) {
    return objectHandles.add(
        Il2Cpp::CppInterop::ToCsString(Il2Cpp::API::il2cpp_image_get_filename(reinterpret_cast<const Il2Cpp::API::Il2CppImage *>(imageHandles.get(image)))));
}
MethodInfoHandle imageGetEntryPoint(ImageHandle image) {
    return methodInfoHandles.add(const_cast<Il2Cpp::API::MethodInfo *>(
        Il2Cpp::API::il2cpp_image_get_entry_point(reinterpret_cast<const Il2Cpp::API::Il2CppImage *>(imageHandles.get(image)))));
}
int32_t imageGetClassCount(ImageHandle image) {
    return static_cast<int32_t>(Il2Cpp::API::il2cpp_image_get_class_count(reinterpret_cast<const Il2Cpp::API::Il2CppImage *>(imageHandles.get(image))));
}
ClassHandle imageGetClassAt(ImageHandle image, int32_t index) {
    return classHandles.add(const_cast<Il2Cpp::API::Il2CppClass *>(
        Il2Cpp::API::il2cpp_image_get_class(reinterpret_cast<const Il2Cpp::API::Il2CppImage *>(imageHandles.get(image)), index)));
}

// Logging
namespace {
void log(spdlog::level::level_enum level, ObjectHandle message) {
    g.logger->log(level, "[{} (FFI)] {}", FFILoader::FFIMod::getCurrent()->name,
                  Il2Cpp::CppInterop::ToCppString(reinterpret_cast<System_String_o *>(objectHandles.get(message))));
}
} // namespace
void logTrace(ObjectHandle message) { log(spdlog::level::trace, message); }
void logDebug(ObjectHandle message) { log(spdlog::level::debug, message); }
void logInfo(ObjectHandle message) { log(spdlog::level::info, message); }
void logWarn(ObjectHandle message) { log(spdlog::level::warn, message); }
void logError(ObjectHandle message) { log(spdlog::level::err, message); }
void logCritical(ObjectHandle message) { log(spdlog::level::critical, message); }

// Methods
namespace Dynamic = Il2Cpp::Dynamic;
MethodHandle getMethodByIdentifier(const char *identifier) { return Dynamic::getMethod(identifier, true).index; }
MethodHandle getMethodByAddress(int64_t addr) { return Dynamic::getMethod(reinterpret_cast<void *>(addr), true).index; }
ObjectHandle getMethodName(MethodHandle index) {
    if (index < 0)
        return 0;
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(Dynamic::getMethod(index).name));
}
ObjectHandle getMethodSignature(MethodHandle index) {
    if (index < 0)
        return 0;
    return objectHandles.add(Il2Cpp::CppInterop::ToCsString(Dynamic::getMethod(index).signature));
}
int64_t getMethodAddresss(MethodHandle index) {
    if (index < 0)
        return 0;
    return reinterpret_cast<uintptr_t>(Dynamic::getMethod(index).getFullAddress());
}
WIBool registerICallMethod(const char *identifier, const char *typeSignature) {
    if (!Dynamic::getMethod(identifier, true).isValid())
        return Dynamic::registerICall(identifier, typeSignature);
    return true;
}

// Argument stack management
void addArgI32(int32_t v) { call_args.push_back(bit_cast<void *>(v)); }
void addArgI64(int64_t v) { call_args.push_back(bit_cast<void *>(v)); }
void addArgFloat(float v) { call_args.push_back(bit_cast<void *>(v)); }
void addArgDouble(double v) { call_args.push_back(bit_cast<void *>(v)); }
void addArgObject(ObjectHandle v) { call_args.push_back(objectHandles.get(v)); }
void addArgClass(ClassHandle v) { call_args.push_back(classHandles.get(v)); }
void addArgNull() { call_args.push_back(nullptr); }
void clearArgs() { call_args.clear(); }

int32_t getArgCount() { return call_args.size(); }
WIBool moveArg(int32_t index) {
    if (call_args.empty() || static_cast<int>(call_args.size()) < index)
        return false;
    if (index >= 0) {
        call_args[index] = call_args.back();
    } else {
        return_value = call_args.back();
    }
    call_args.pop_back();
    return true;
}

int32_t getValueI32(int32_t index) { return bit_cast<int32_t>(getValue(index)); }
int64_t getValueI64(int32_t index) { return bit_cast<int64_t>(getValue(index)); }
float getValueFloat(int32_t index) {
    void *value = getValue(index);
    return bit_cast<float>(value);
}
double getValueDouble(int32_t index) {
    void *value = getValue(index);
    return bit_cast<double>(value);
}
ObjectHandle getValueObject(int32_t index) { return objectHandles.add(getValue(index)); }

// Method calling
ObjectHandle getCallError() { return objectHandles.add(Il2Cpp::CppInterop::ToCsString(call_error)); }

namespace {
void logBadCall(MethodHandle index) {
    const auto& method = Il2Cpp::Dynamic::getMethod(index);
    g.logger->warn("FFI interface failed to call function '{}': {}", method.isValid() ? method.signature.empty() ? method.name : method.signature : "<invalid>",
                   call_error);
}
void logBadCall(Il2Cpp::API::MethodInfo *methodInfo) {
    Il2Cpp::API::Method method{methodInfo};
    g.logger->warn("FFI interface failed to call function '{}': {}",
                   method ? std::format("{} in {}", method.name(), method.declaring_type().name()) : "<invalid>", call_error);
}
void logBadCall(const char *name) { g.logger->warn("FFI interface failed to call function '{}': {}", name, call_error); }
void logBadCall() { g.logger->warn("FFI interface failed to call unkown function: {}", call_error); }
} // namespace

WIBool call2(MethodHandle index, int32_t argCount, WIBool returnsStruct) {
    // Handle unknown argument count
    if (argCount == unknownArgCount)
        argCount = call_args.size();

    // Make sure argCount matches argument count
    else if (argCount != call_args.size()) {
        call_error = std::format("Mismatched added ({}) vs. passed ({}) arg count", call_args.size(), argCount);
        logBadCall(index);
        return false;
    }

    // Get method
    const auto method = Dynamic::getMethod(index);
    if (!method.isValid()) {
        call_error = std::format("Bad method index ({})", index);
        logBadCall(index);
        return false;
    }

    // Check argument count
    const auto actualArgCount = method.getArgCount();
    if (argCount != actualArgCount) {
        call_error = std::format("Mismatched passed ({}) vs. actual ({}) arg count", argCount, actualArgCount);
        logBadCall(index);
        return false;
    }

    // Back up current mod in case called function triggers hook from another mod
    ModInfo *currentModBackup = FFILoader::FFIMod::getCurrent();

    // Call function
    const auto args = std::move(std::exchange(call_args, {}));
    bool fres = true;
    try {
        if (!returnsStruct) {
            return_value =
                reinterpret_cast<void *>(AnyCall::call(reinterpret_cast<const uintptr_t *>(args.data()), method.getFullAddress(), method.typeSignature));
        } else {
            // Lock array to prevent its deletion
            Il2Cpp::API::GcHandle AGH(reinterpret_cast<Il2Cpp::API::Il2CppObject *>(return_value), true);
            // Get pointer/reference into array
            auto *dataDest = reinterpret_cast<System_Byte_array *>(return_value);
            const size_t& dataLength = dataDest->bounds->length;
            // AnyCall specified function
            const AnyCall::Struct returnData =
                AnyCall::callStruct(reinterpret_cast<const uintptr_t *>(args.data()), method.getFullAddress(), method.typeSignature);
            // Copy function result into array
            std::memcpy(dataDest->m_Items, &returnData, std::min<size_t>(dataLength, sizeof(returnData)));
            return_value = dataDest;
        }
    } catch (const std::exception& e) {
        call_error = std::format("Method has thrown an exception: {}", e.what());
        logBadCall(index);
        fres = false;
    } catch (...) {
        call_error = "Method has thrown an unknown exception";
        logBadCall(index);
        fres = false;
    }

    // Restore current mod (see comment near declaration)
    FFILoader::FFIMod::setCurrent(currentModBackup);

    // Clear arguments again in case called function ended up adding arguments through hook
    call_args.clear();

    // Everything seems to have gone well
    return fres;
}
WIBool call(MethodHandle index, int32_t argCount) { return call2(index, argCount, false); }

ObjectHandle call3(MethodInfoHandle handle, int32_t argCount) {
    // Handle unknown argument count
    if (argCount == unknownArgCount)
        argCount = call_args.size();

    // Get method
    const auto method = methodInfoHandles.get(handle);
    if (!method) {
        call_error = std::format("Bad method handle ({})", handle);
        logBadCall();
        return -1;
    }

    // Make sure argCount matches argument count
    else if (argCount != call_args.size() - 1) {
        call_error = std::format("Mismatched added ({}) vs. passed ({}) arg count", call_args.size(), argCount);
        logBadCall(method);
        return -1;
    }

    // Make sure object instance is passed
    if (call_args.empty()) {
        call_error = std::format("Missing object instance", call_args.size(), argCount);
        logBadCall(method);
        return -1;
    }

    // Check argument count
    const auto actualArgCount = Il2Cpp::API::il2cpp_method_get_param_count(method);
    if (argCount != actualArgCount) {
        call_error = std::format("Mismatched passed ({}) vs. actual ({}) arg count", argCount, actualArgCount);
        logBadCall(method);
        return -1;
    }

    // Back up current mod in case called function triggers hook from another mod
    ModInfo *currentModBackup = FFILoader::FFIMod::getCurrent();

    // Call function
    Il2Cpp::API::Object __this = reinterpret_cast<Il2Cpp::API::Il2CppObject *>(call_args[0]);

    auto args = std::move(std::exchange(call_args, {}));
    Il2Cpp::API::Il2CppException *exc = nullptr;
    Il2CppObject *fres = Il2Cpp::API::il2cpp_runtime_invoke_convert_args(method, __this.unbox_if_value(), reinterpret_cast<Il2CppObject **>(args.data() + 1),
                                                                         static_cast<int>(args.size() - 1), &exc);
    return_value = exc ? nullptr : fres;

    // Restore current mod (see comment near declaration)
    FFILoader::FFIMod::setCurrent(currentModBackup);

    // Clear arguments again in case called function ended up adding arguments through hook
    call_args.clear();

    // Everything seems to have gone well
    return objectHandles.add(exc);
}

// Hooks
namespace {
struct FFIGameHookInfo {
    std::shared_ptr<GameHook> hook;
    ModInfo *modInfo;
    std::string callback;
};
using HookVariant = std::variant<MethodHandle, Il2Cpp::API::MethodInfo *>;
std::map<HookVariant, FFIGameHookInfo> hooks;
MethodHandle currentHookMethod;
void *ffiHook(void *a, void *b, void *c, void *d, void *e, void *f) noexcept {
    // Get method
    const auto method = Dynamic::getMethod(GameHook::getTrampolineCaller());
    if (!method.isValid()) {
        g.logger->error("FFI GameHook interface encountered an invalid hook!");
        return nullptr;
    }
    currentHookMethod = method.index;

    // Get hook
    auto res = hooks.find(currentHookMethod);
    if (res == hooks.end()) {
        g.logger->error("FFI GameHook interface encountered an unknown hook!");
        return nullptr;
    }
    FFIGameHookInfo& hookInfo = res->second;
    auto hook = hookInfo.hook; // Prevents UB when hook is erased in callback

    // Make sure mod is still loaded
    if (!hookInfo.modInfo->isLoaded()) {
        g.logger->error("FFI GameHook interface attempted to handle hook for "
                        "unloaded mod!");
        hooks.erase(method.index);
        return nullptr;
    }

    // Set up argument list
    call_args = {a, b, c, d, e, f};
    call_args.resize(method.getArgCount());

    // Call hook callback
    try {
        GameHookRelease GHR(*hooks.at(currentHookMethod).hook);
        hookInfo.modInfo->get<FFILoader::FFIMod>()->simpleCall(hookInfo.callback.c_str());
        return return_value;
    } catch (const std::exception& e) {
        g.logger->error("Exception while executing FFI GameHook callback '{}': {}", hookInfo.callback, e.what());
        return nullptr;
    } catch (const ModPanic& e) {
        g.logger->error("FFI Mod '{}' has panicked: {}", e.where(), e.what());
        return nullptr;
    } catch (...) {
        g.logger->error("Unknown exception while executing FFI GameHook callback '{}'!", hookInfo.callback);
        return nullptr;
    }
}
GAMEHOOK_TRAMPOLINE(ffiHook)
} // namespace
WIBool hook(MethodHandle method, const char *callback) {
    const auto methodInfo = Dynamic::getMethod(method);
    if (!methodInfo.isValid())
        return false;
    if (methodInfo.getArgCount() > 6) {
        g.logger->warn("FFI Mod attempted to hook function with more than 6 "
                       "arguments (which is unsupported)");
        return false;
    }
    auto hook = GameHook::safeCreate(methodInfo.getFullAddress(), reinterpret_cast<void *>(hookTrampoline_ffiHook), true);
    if (!hook.has_value())
        return false;
    return hooks.emplace(method, FFIGameHookInfo{std::make_shared<GameHook>(std::move(*hook)), FFILoader::FFIMod::getCurrent(), callback}).second;
}
WIBool hook2(MethodInfoHandle method, const char *callback) {
    auto *methodInfo = methodInfoHandles.get(method);
    if (!methodInfo)
        return false;
    if (Il2Cpp::API::il2cpp_method_get_param_count(methodInfo) > 5) {
        g.logger->warn("FFI Mod attempted to hook function with more than 5 "
                       "arguments (which is unsupported)");
        return false;
    }
    auto hook = GameHook::safeCreate(methodInfo->methodPointer, reinterpret_cast<void *>(hookTrampoline_ffiHook), true);
    if (!hook.has_value())
        return false;
    return hooks.emplace(methodInfo, FFIGameHookInfo{std::make_shared<GameHook>(std::move(*hook)), FFILoader::FFIMod::getCurrent(), callback}).second;
}
WIBool unhook(MethodHandle method) { return hooks.erase(method); }
WIBool unhook2(MethodInfoHandle method) {
    auto *methodInfo = methodInfoHandles.get(method);
    if (!methodInfo)
        return false;
    return hooks.erase(methodInfo);
}
MethodHandle getOriginal() { return currentHookMethod; }

// ImGui wrappers
void ImGuiBegin(const char *name) { ImGui::Begin(name); }
void ImGuiEnd() { ImGui::End(); }
void ImGuiText(ObjectHandle text) {
    ImGui::TextUnformatted(Il2Cpp::CppInterop::ToCppString(reinterpret_cast<System_String_o *>(objectHandles.get(text))).c_str());
}
void ImGuiCheckbox(const char *label, bool *v) { ImGui::Checkbox(label, v); }
WIBool ImGuiCheckbox2(const char *label, WIBool v) {
    bool fres = v;
    ImGui::Checkbox(label, &fres);
    return fres;
}
WIBool ImGuiCheckbox3(const char *label, bool *v) { return ImGui::Checkbox(label, v); }
int32_t ImGuiCheckbox4(const char *label, WIBool v) {
    bool state = v;
    bool fres = ImGui::Checkbox(label, &state);
    return (state ? 0b01 : 0) | (fres ? 0b10 : 0);
}
WIBool ImGuiButton(const char *label) { return ImGui::Button(label); }
void ImGuiSeparator() { ImGui::Separator(); }
void ImGuiSeparatorText(const char *label) { ImGui::SeparatorText(label); }

// WASM runtime support
void abort(const char *message, const char *filename, int32_t lineNumber, int32_t columnNumber) {
    auto modInfo = FFILoader::FFIMod::getCurrent();
    const auto msg = std::format("FFI module {} has called abort()!\n - Message: "
                                 "{}\n - Filename: {}\n - Line: {}\n - Column: {}",
                                 modInfo->name, message ? message : "none", filename ? filename : "unknown", lineNumber, columnNumber);
    g.logger->critical(msg);
    throw ModPanic(*modInfo, msg);
}
} // namespace FFIInterface
