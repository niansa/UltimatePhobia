#pragma once

#ifndef FFI_NOSTL
#ifndef FFI_USE_CLP
#include <tuple>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#ifndef FFI_NOSTL_CONTAINERS
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <algorithm>
#endif
#else
#include <clp/tuple>
#include <clp/type_traits>
#include <cstdint>
#include <cstddef>
#ifndef FFI_NOSTL_CONTAINERS
#include <clp/string>
#include <clp/vector>
#include <clp/optional>
#include <clp/memory>
#include <clp/algorithm>
#endif
#endif
#else
#define FFI_NOSTL_CONTAINERS
#include <stdint.h>
#include <stddef.h>
#endif

#if !defined(FFI_EXT) && defined(WASM)
#define FFI_EXT
#endif

#ifndef UP_API
#ifdef WASM
#define UP_API WASM_IMPORT
#else
#define UP_API
#endif
#endif

#ifndef UP_DEPRECATED
#define UP_DEPRECATED(message)
#endif

namespace FFIInterface {
#ifndef FFI_USE_CLP
namespace tl = std;
#else
namespace tl = clp;
#endif

#ifdef FFI_EXT
#define FFI_DEFINE_GENERIC_HANDLE_TYPE(name) enum class name : int32_t { Null = 0, Invalid = -1 }
enum class MethodHandle : int32_t { Invalid = -1 };
enum class GCHandle : int32_t { Invalid = -1 };
#else
#define FFI_DEFINE_GENERIC_HANDLE_TYPE(name) using name = int32_t
using MethodHandle = int32_t;
using GCHandle = int32_t;
#endif
FFI_DEFINE_GENERIC_HANDLE_TYPE(ObjectHandle);
FFI_DEFINE_GENERIC_HANDLE_TYPE(ValueHandle);
FFI_DEFINE_GENERIC_HANDLE_TYPE(ImageHandle);
FFI_DEFINE_GENERIC_HANDLE_TYPE(ClassHandle);
FFI_DEFINE_GENERIC_HANDLE_TYPE(DomainHandle);
FFI_DEFINE_GENERIC_HANDLE_TYPE(AssemblyHandle);
FFI_DEFINE_GENERIC_HANDLE_TYPE(TypeHandle);
FFI_DEFINE_GENERIC_HANDLE_TYPE(MethodInfoHandle);
FFI_DEFINE_GENERIC_HANDLE_TYPE(FieldHandle);
FFI_DEFINE_GENERIC_HANDLE_TYPE(PropertyHandle);
FFI_DEFINE_GENERIC_HANDLE_TYPE(EventHandle);
using WIBool = int32_t;
constexpr int32_t unknownArgCount = 0x6D616E63;

#ifdef FFI_USES_FTABLE
namespace Signatures {
#endif

/**
 * @brief Gets current ftable size to ensure runtime is recent enough
 * @return Amount of items in ftable
 */
UP_API int32_t getFtableItemCount();

/**
 * @brief Invalidates given handle allowing it to be reused
 */
UP_API void dropObject(ObjectHandle);
/**
 * @brief Invalidates given handle allowing it to be reused
 */
UP_API void dropValue(ValueHandle);
/**
 * @brief Invalidates given handle allowing it to be reused
 */
UP_API void dropImage(ImageHandle);
/**
 * @brief Invalidates given handle allowing it to be reused
 */
UP_API void dropClass(ClassHandle);
/**
 * @brief Invalidates given handle allowing it to be reused
 */
UP_API void dropDomain(DomainHandle);
/**
 * @brief Invalidates given handle allowing it to be reused
 */
UP_API void dropAssembly(AssemblyHandle);
/**
 * @brief Invalidates given handle allowing it to be reused
 */
UP_API void dropType(TypeHandle);
/**
 * @brief Invalidates given handle allowing it to be reused
 */
UP_API void dropMethodInfo(MethodInfoHandle);
/**
 * @brief Invalidates given handle allowing it to be reused
 */
UP_API void dropField(FieldHandle);
/**
 * @brief Invalidates given handle allowing it to be reused
 */
UP_API void dropProperty(PropertyHandle);
/**
 * @brief Invalidates given handle allowing it to be reused
 */
UP_API void dropEvent(EventHandle);
/**
 * @brief Check if given handle is a valid C# object
 * @return 1 if handle is valid, otherwisewise 0
 */
UP_API WIBool isValidObject(ObjectHandle);
/**
 * @brief Returns the address of given object
 * @return Address of object
 */
UP_API int64_t getObjectAddress(ObjectHandle);
/**
 * @return C# null object (0 handle)
 */
UP_API ObjectHandle getNull();

/**
 * @brief Converts given C string to System.String object until null-terminator
 * @param str C string
 */
UP_API ObjectHandle toCsString(const char *str);
/**
 * @brief Converts given C string of specified length to System.String object
 * @param str C string
 * @param length Length of C string
 */
UP_API ObjectHandle toCsStringWithLength(const char *str, int32_t length);
/**
 * @brief Converts given string to C string (including null terminator)
 * @param str System.String object
 * @param buf Buffer to write string to
 * @param maxlen Length of buffer
 */
UP_API void toCString(ObjectHandle str, char *buf, int32_t maxlen);

/**
 * @brief Returns System.String length
 */
UP_API int32_t stringGetLength(ObjectHandle str);
/**
 * @brief Returns interned string
 */
UP_API ObjectHandle stringIntern(ObjectHandle str);
/**
 * @brief Returns interned string if exists else 0
 */
UP_API ObjectHandle stringIsInterned(ObjectHandle str);

/**
 * @brief Gets corlib image handle
 * @return Image handle
 */
UP_API ImageHandle getImageCorlib();
/**
 * @brief Gets specified class handle
 * @param image Handle of image to load class from
 * @param namespaze Namespace to load class from
 * @param name Name of class to load
 * @return Class handle
 */
UP_API ClassHandle getClassFromName(ImageHandle image, const char *namespaze, const char *name);
/**
 * @brief Gets array class from class
 * @param elementClass Element type
 * @param rank Rank (1 for single-dimension)
 * @return Class handle
 */
UP_API ClassHandle getArrayFromClass(ClassHandle elementClass, int32_t rank);
/**
 * @brief Create an array
 * @param elementClass Element type
 * @param length Length of array
 * @return Handle of new array object
 */
UP_API ObjectHandle createArray(ClassHandle elementClass, int32_t length);
/**
 * @brief Gets length of managed array
 * @param array Array object
 * @return Number of elements in array
 */
UP_API int32_t arrayGetLength(ObjectHandle array);
/**
 * @brief Gets byte length of managed array storage
 * @param array Array object
 * @return Number of bytes occupied by array data
 */
UP_API int32_t arrayGetByteLength(ObjectHandle array);
/**
 * @brief Create an array of specific array type (e.g. int[] type)
 * @param arrayTypeInfo Array IL2CPP class (e.g. System.Int32[])
 * @param length Length of array
 * @return Handle of new array object
 */
UP_API ObjectHandle createArraySpecific(ClassHandle arrayTypeInfo, int32_t length);
/**
 * @brief Gets array class for bounded arrays
 * @param elementClass Element IL2CPP class
 * @param rank Rank (1 for single-dimension)
 * @param bounded True for bounded array
 * @return Class handle for the array class
 */
UP_API ClassHandle getBoundedArrayClass(ClassHandle elementClass, int32_t rank, WIBool bounded);
/**
 * @brief Gets element size of array class
 * @param arrayClass Array IL2CPP class
 * @return Element size in bytes
 */
UP_API int32_t arrayElementSize(ClassHandle arrayClass);
/**
 * @brief Copies an array into memory
 * @param array Array to copy from
 * @param offset Amount of bytes to skip
 * @param length Amount of bytes to copy
 * @param to Buffer to copy bytes into
 */
UP_API void copyArrayBytes(ObjectHandle array, int32_t offset, int32_t length, void *to);

/**
 * @brief Gets IL2CPP domain
 * @return Domain handle
 */
UP_API DomainHandle domainGet();
/**
 * @brief Opens an assembly by name in domain
 * @param domain Domain handle
 * @param name Assembly name (e.g. "Assembly-CSharp.dll")
 * @return Assembly handle or 0 if not found
 */
UP_API AssemblyHandle domainAssemblyOpen(DomainHandle domain, const char *name);
/**
 * @brief Gets number of assemblies in domain
 * @param domain Domain handle
 * @return Assembly count
 */
UP_API int32_t domainGetAssemblyCount(DomainHandle domain);
/**
 * @brief Gets assembly at index in domain (0..count-1)
 * @param domain Domain handle
 * @param index Index
 * @return Assembly handle or 0 if out of bounds
 */
UP_API AssemblyHandle domainGetAssemblyAt(DomainHandle domain, int32_t index);
/**
 * @brief Gets image for assembly
 * @param assembly Assembly handle
 * @return Image handle
 */
UP_API ImageHandle assemblyGetImage(AssemblyHandle assembly);

/**
 * @brief Gets enum base type for class
 * @param klass Class handle
 * @return Type handle or 0
 */
UP_API TypeHandle classEnumBaseType(ClassHandle klass);
/**
 * @brief Checks if class is generic
 */
UP_API WIBool classIsGeneric(ClassHandle klass);
/**
 * @brief Checks if class is inflated (generic instantiation)
 */
UP_API WIBool classIsInflated(ClassHandle klass);
/**
 * @brief Checks if klass is assignable from oklass
 */
UP_API WIBool classIsAssignableFrom(ClassHandle klass, ClassHandle oklass);
/**
 * @brief Checks if klass is subclass of klassc
 * @param checkInterfaces Also check interfaces
 */
UP_API WIBool classIsSubclassOf(ClassHandle klass, ClassHandle klassc, WIBool checkInterfaces);
/**
 * @brief Checks if klass has klassc as parent
 */
UP_API WIBool classHasParent(ClassHandle klass, ClassHandle klassc);
/**
 * @brief Gets class from IL2CPP type
 */
UP_API ClassHandle getClassFromType(TypeHandle type);
/**
 * @brief Gets class from System.Type (reflection)
 */
UP_API ClassHandle getClassFromSystemType(ObjectHandle reflectionType);
/**
 * @brief Gets array element class
 */
UP_API ClassHandle classGetElementClass(ClassHandle klass);
/**
 * @brief Gets class name
 * @return System.String
 */
UP_API ObjectHandle classGetName(ClassHandle klass);
/**
 * @brief Gets class namespace
 * @return System.String
 */
UP_API ObjectHandle classGetNamespace(ClassHandle klass);
/**
 * @brief Gets parent class
 */
UP_API ClassHandle classGetParent(ClassHandle klass);
/**
 * @brief Gets declaring type (enclosing class), or 0
 */
UP_API ClassHandle classGetDeclaringType(ClassHandle klass);
/**
 * @brief Gets instance size of class (including object header)
 */
UP_API int32_t classInstanceSize(ClassHandle klass);
/**
 * @brief Returns number of fields in class
 */
UP_API int32_t classNumFields(ClassHandle klass);
/**
 * @brief Returns true if valuetype (struct)
 */
UP_API WIBool classIsValueType(ClassHandle klass);
/**
 * @brief Returns true if blittable
 */
UP_API WIBool classIsBlittable(ClassHandle klass);
/**
 * @brief Returns class flags (TypeAttributes)
 */
UP_API int32_t classGetFlags(ClassHandle klass);
/**
 * @brief Returns true if abstract
 */
UP_API WIBool classIsAbstract(ClassHandle klass);
/**
 * @brief Returns true if interface
 */
UP_API WIBool classIsInterface(ClassHandle klass);
/**
 * @brief Returns element size for array class
 */
UP_API int32_t classArrayElementSize(ClassHandle klass);
/**
 * @brief Gets IL2CPP type for class
 */
UP_API TypeHandle classGetType(ClassHandle klass);
/**
 * @brief Gets type token for class
 */
UP_API int32_t classGetTypeToken(ClassHandle klass);
/**
 * @brief Returns true if class has attribute attrClass
 */
UP_API WIBool classHasAttribute(ClassHandle klass, ClassHandle attrClass);
/**
 * @brief Returns true if class has GC references
 */
UP_API WIBool classHasReferences(ClassHandle klass);
/**
 * @brief Returns true if class is enum
 */
UP_API WIBool classIsEnum(ClassHandle klass);
/**
 * @brief Gets image for class
 */
UP_API ImageHandle classGetImage(ClassHandle klass);
/**
 * @brief Gets assembly name for class
 * @return System.String
 */
UP_API ObjectHandle classGetAssemblyName(ClassHandle klass);
/**
 * @brief Gets class rank (for array types)
 */
UP_API int32_t classGetRank(ClassHandle klass);
/**
 * @brief Gets data size (for value types)
 */
UP_API int32_t classGetDataSize(ClassHandle klass);

/**
 * @brief Gets event at index (iterate 0..N-1)
 */
UP_API EventHandle classGetEventAt(ClassHandle klass, int32_t index);
/**
 * @brief Gets field at index (iterate 0..N-1)
 */
UP_API FieldHandle classGetFieldAt(ClassHandle klass, int32_t index);
/**
 * @brief Gets nested type at index (iterate 0..N-1)
 */
UP_API ClassHandle classGetNestedTypeAt(ClassHandle klass, int32_t index);
/**
 * @brief Gets interface at index (iterate 0..N-1)
 */
UP_API ClassHandle classGetInterfaceAt(ClassHandle klass, int32_t index);
/**
 * @brief Gets property at index (iterate 0..N-1)
 */
UP_API PropertyHandle classGetPropertyAt(ClassHandle klass, int32_t index);
/**
 * @brief Gets method at index (iterate 0..N-1)
 */
UP_API MethodInfoHandle classGetMethodAt(ClassHandle klass, int32_t index);
/**
 * @brief Gets property by name
 */
UP_API PropertyHandle classGetPropertyFromName(ClassHandle klass, const char *name);
/**
 * @brief Gets field by name
 */
UP_API FieldHandle classGetFieldFromName(ClassHandle klass, const char *name);
/**
 * @brief Gets method by name and arg count
 */
UP_API MethodInfoHandle classGetMethodFromName(ClassHandle klass, const char *name, int32_t argsCount);

/**
 * @brief Raises managed exception (noreturn in managed)
 */
UP_API void exceptionRaise(ObjectHandle ex);
/**
 * @brief Creates exception by name with message
 */
UP_API ObjectHandle exceptionFromNameMsg(ImageHandle image, const char *namespaze, const char *name, const char *msg);
/**
 * @brief Creates ArgumentNullException for argument
 */
UP_API ObjectHandle exceptionGetArgumentNull(const char *arg);
/**
 * @brief Formats exception message
 * @return System.String with formatted exception
 */
UP_API ObjectHandle exceptionFormat(ObjectHandle ex);
/**
 * @brief Formats exception stack trace
 * @return System.String with formatted stack trace
 */
UP_API ObjectHandle exceptionFormatStackTrace(ObjectHandle ex);
/**
 * @brief Marks exception as unhandled
 */
UP_API void exceptionUnhandled(ObjectHandle ex);

/**
 * @brief Gets field flags
 */
UP_API int32_t fieldGetFlags(FieldHandle field);
/**
 * @brief Gets field name
 * @return System.String
 */
UP_API ObjectHandle fieldGetName(FieldHandle field);
/**
 * @brief Gets field parent class
 */
UP_API ClassHandle fieldGetParent(FieldHandle field);
/**
 * @brief Gets field offset
 */
UP_API int32_t fieldGetOffset(FieldHandle field);
/**
 * @brief Gets field type
 */
UP_API TypeHandle fieldGetType(FieldHandle field);
/**
 * @brief Checks if field has attribute
 */
UP_API WIBool fieldHasAttribute(FieldHandle field, ClassHandle attrClass);
/**
 * @brief Sets instance field (int32)
 */
UP_API void fieldSetValueI32(ObjectHandle obj, FieldHandle field, int32_t value);
/**
 * @brief Sets instance field (int64)
 */
UP_API void fieldSetValueI64(ObjectHandle obj, FieldHandle field, int64_t value);
/**
 * @brief Sets instance field (float)
 */
UP_API void fieldSetValueFloat(ObjectHandle obj, FieldHandle field, float value);
/**
 * @brief Sets instance field (double)
 */
UP_API void fieldSetValueDouble(ObjectHandle obj, FieldHandle field, double value);
/**
 * @brief Sets instance field (object)
 */
UP_API void fieldSetValueObject(ObjectHandle obj, FieldHandle field, ObjectHandle value);
/**
 * @brief Gets instance field (int32)
 */
UP_API int32_t fieldGetValueI32(ObjectHandle obj, FieldHandle field);
/**
 * @brief Gets instance field (int64)
 */
UP_API int64_t fieldGetValueI64(ObjectHandle obj, FieldHandle field);
/**
 * @brief Gets instance field (float)
 */
UP_API float fieldGetValueFloat(ObjectHandle obj, FieldHandle field);
/**
 * @brief Gets instance field (double)
 */
UP_API double fieldGetValueDouble(ObjectHandle obj, FieldHandle field);
/**
 * @brief Gets instance field as object (boxes value types)
 */
UP_API ObjectHandle fieldGetValueObject(ObjectHandle obj, FieldHandle field);
/**
 * @brief Gets static field (int32)
 */
UP_API int32_t fieldStaticGetValueI32(FieldHandle field);
/**
 * @brief Gets static field (int64)
 */
UP_API int64_t fieldStaticGetValueI64(FieldHandle field);
/**
 * @brief Gets static field (float)
 */
UP_API float fieldStaticGetValueFloat(FieldHandle field);
/**
 * @brief Gets static field (double)
 */
UP_API double fieldStaticGetValueDouble(FieldHandle field);
/**
 * @brief Sets static field (int32)
 */
UP_API void fieldStaticSetValueI32(FieldHandle field, int32_t value);
/**
 * @brief Sets static field (int64)
 */
UP_API void fieldStaticSetValueI64(FieldHandle field, int64_t value);
/**
 * @brief Sets static field (float)
 */
UP_API void fieldStaticSetValueFloat(FieldHandle field, float value);
/**
 * @brief Sets static field (double)
 */
UP_API void fieldStaticSetValueDouble(FieldHandle field, double value);
/**
 * @brief Returns true if field is literal
 */
UP_API WIBool fieldIsLiteral(FieldHandle field);

/**
 * @brief Create a garbage collection handle
 * @param object Object to create handle for
 * @param pinned If object should be pinned
 * @return Handle that has to be deleted later
 */
UP_API GCHandle gcCreateHandle(ObjectHandle object, WIBool pinned);
/**
 * @brief Deletes garbage collection handle allowing the underlaying object to be garbage collected
 */
UP_API void gcDeleteHandle(GCHandle);
/**
 * @brief Triggers full GC (up to given generation)
 */
UP_API void gcCollect(int32_t maxGenerations);
/**
 * @brief Performs small incremental GC step
 * @return Amount of work done (implementation dependent)
 */
UP_API int32_t gcCollectALittle();
/**
 * @brief Disables GC
 */
UP_API void gcDisable();
/**
 * @brief Enables GC
 */
UP_API void gcEnable();
/**
 * @brief Returns true if GC disabled
 */
UP_API WIBool gcIsDisabled();
/**
 * @brief Gets max incremental GC time slice in ns
 */
UP_API int64_t gcGetMaxTimeSliceNs();
/**
 * @brief Sets max incremental GC time slice in ns
 */
UP_API void gcSetMaxTimeSliceNs(int64_t ns);
/**
 * @brief Returns true if GC is incremental
 */
UP_API WIBool gcIsIncremental();
/**
 * @brief Gets used managed heap size in bytes
 */
UP_API int64_t gcGetUsedSize();
/**
 * @brief Gets total managed heap size in bytes
 */
UP_API int64_t gcGetHeapSize();
/**
 * @brief Returns true if strict write barriers are used
 */
UP_API WIBool gcHasStrictWBarriers();
/**
 * @brief Stops GC world
 */
UP_API void gcStopWorld();
/**
 * @brief Starts GC world
 */
UP_API void gcStartWorld();
/**
 * @brief Creates weak GC handle
 * @param object Object
 * @param trackResurrection If resurrection should be tracked
 */
UP_API GCHandle gcCreateWeakHandle(ObjectHandle object, WIBool trackResurrection);
/**
 * @brief Gets object from GC handle
 * @return Object or 0 if finalized
 */
UP_API ObjectHandle gcGetTarget(GCHandle handle);

/**
 * @brief Gets return type of method
 */
UP_API TypeHandle methodInfoGetReturnType(MethodInfoHandle method);
/**
 * @brief Gets declaring class
 */
UP_API ClassHandle methodInfoGetDeclaringType(MethodInfoHandle method);
/**
 * @brief Gets method name
 * @return System.String
 */
UP_API ObjectHandle methodInfoGetName(MethodInfoHandle method);
/**
 * @brief Gets MethodInfo from reflection method
 */
UP_API MethodInfoHandle methodInfoGetFromReflection(ObjectHandle reflectionMethod);
/**
 * @brief Gets reflection method object for MethodInfo
 */
UP_API ObjectHandle methodInfoGetObject(MethodInfoHandle method, ClassHandle refclass);
/**
 * @brief Returns true if generic
 */
UP_API WIBool methodInfoIsGeneric(MethodInfoHandle method);
/**
 * @brief Returns true if inflated (generic instantiation)
 */
UP_API WIBool methodInfoIsInflated(MethodInfoHandle method);
/**
 * @brief Returns true if instance method
 */
UP_API WIBool methodInfoIsInstance(MethodInfoHandle method);
/**
 * @brief Gets parameter count
 */
UP_API int32_t methodInfoGetParamCount(MethodInfoHandle method);
/**
 * @brief Gets parameter type at index
 */
UP_API TypeHandle methodInfoGetParamType(MethodInfoHandle method, int32_t index);
/**
 * @brief Gets class of method
 */
UP_API ClassHandle methodInfoGetClass(MethodInfoHandle method);
/**
 * @brief Returns true if method has attribute
 */
UP_API WIBool methodInfoHasAttribute(MethodInfoHandle method, ClassHandle attrClass);
/**
 * @brief Gets method flags (TypeAttributes)
 */
UP_API int32_t methodInfoGetFlags(MethodInfoHandle method);
/**
 * @brief Gets method implementation flags
 */
UP_API int32_t methodInfoGetImplFlags(MethodInfoHandle method);
/**
 * @brief Gets metadata token
 */
UP_API int32_t methodInfoGetToken(MethodInfoHandle method);
/**
 * @brief Gets parameter name at index
 * @return System.String
 */
UP_API ObjectHandle methodInfoGetParamName(MethodInfoHandle method, int32_t index);

/**
 * @brief Gets property flags
 */
UP_API int32_t propertyGetFlags(PropertyHandle prop);
/**
 * @brief Gets getter method
 */
UP_API MethodInfoHandle propertyGetGetMethod(PropertyHandle prop);
/**
 * @brief Gets setter method
 */
UP_API MethodInfoHandle propertyGetSetMethod(PropertyHandle prop);
/**
 * @brief Gets property name
 * @return System.String
 */
UP_API ObjectHandle propertyGetName(PropertyHandle prop);
/**
 * @brief Gets property parent class
 */
UP_API ClassHandle propertyGetParent(PropertyHandle prop);

/**
 * @brief Gets class of object
 */
UP_API ClassHandle objectGetClass(ObjectHandle obj);
/**
 * @brief Gets managed object size
 */
UP_API int32_t objectGetSize(ObjectHandle obj);
/**
 * @brief Gets virtual method for object (final dispatch)
 */
UP_API MethodInfoHandle objectGetVirtualMethod(ObjectHandle obj, MethodInfoHandle method);
/**
 * @brief Allocates new object (no ctor)
 */
UP_API ObjectHandle objectNew(ClassHandle klass);
/**
 * @brief Unboxes to value with same lifetime as object
 */
UP_API ValueHandle objectUnboxValue(ObjectHandle obj);
/**
 * @brief Unboxes to int32
 */
UP_API int32_t objectUnboxI32(ObjectHandle obj);
/**
 * @brief Unboxes to int64
 */
UP_API int64_t objectUnboxI64(ObjectHandle obj);
/**
 * @brief Unboxes to float
 */
UP_API float objectUnboxFloat(ObjectHandle obj);
/**
 * @brief Unboxes to double
 */
UP_API double objectUnboxDouble(ObjectHandle obj);
/**
 * @brief Boxes value
 */
UP_API ObjectHandle valueBox(ClassHandle klass, ValueHandle value);
/**
 * @brief Boxes int32
 */
UP_API ObjectHandle valueBoxI32(ClassHandle klass, int32_t value);
/**
 * @brief Boxes int64
 */
UP_API ObjectHandle valueBoxI64(ClassHandle klass, int64_t value);
/**
 * @brief Boxes float
 */
UP_API ObjectHandle valueBoxFloat(ClassHandle klass, float value);
/**
 * @brief Boxes double
 */
UP_API ObjectHandle valueBoxDouble(ClassHandle klass, double value);

/**
 * @brief Enters monitor
 */
UP_API void monitorEnter(ObjectHandle obj);
/**
 * @brief Tries to enter monitor with timeout (ms)
 */
UP_API WIBool monitorTryEnter(ObjectHandle obj, int32_t timeout);
/**
 * @brief Exits monitor
 */
UP_API void monitorExit(ObjectHandle obj);
/**
 * @brief Pulses one waiter
 */
UP_API void monitorPulse(ObjectHandle obj);
/**
 * @brief Pulses all waiters
 */
UP_API void monitorPulseAll(ObjectHandle obj);
/**
 * @brief Waits on monitor (indefinitely)
 */
UP_API void monitorWait(ObjectHandle obj);
/**
 * @brief Tries to wait on monitor with timeout (ms)
 */
UP_API WIBool monitorTryWait(ObjectHandle obj, int32_t timeout);

/**
 * @brief Invokes .cctor (class constructor)
 */
UP_API void runtimeClassInit(ClassHandle klass);
/**
 * @brief Calls object .ctor (no-arg) if available (unsafe if wrong type)
 */
UP_API void runtimeObjectInit(ObjectHandle obj);
/**
 * @brief Calls object .ctor and returns success (sets last call error otherwise)
 */
UP_API WIBool runtimeObjectInitException(ObjectHandle obj);
/**
 * @brief Sets unhandled exception policy
 */
UP_API void runtimeUnhandledExceptionPolicySet(int32_t value);

/**
 * @brief Gets System.Type object for IL2CPP type
 */
UP_API ObjectHandle typeGetObject(TypeHandle type);
/**
 * @brief Gets IL2CPP type code (Il2CppTypeEnum)
 */
UP_API int32_t typeGetType(TypeHandle type);
/**
 * @brief Gets class or element class for IL2CPP type
 */
UP_API ClassHandle typeGetClassOrElementClass(TypeHandle type);
/**
 * @brief Gets full type name
 * @return System.String
 */
UP_API ObjectHandle typeGetName(TypeHandle type);
/**
 * @brief Returns true if byref (ref/out)
 */
UP_API WIBool typeIsByRef(TypeHandle type);
/**
 * @brief Gets type attributes
 */
UP_API int32_t typeGetAttrs(TypeHandle type);
/**
 * @brief Returns true if types equal
 */
UP_API WIBool typeEquals(TypeHandle a, TypeHandle b);
/**
 * @brief Gets assembly-qualified type name
 * @return System.String
 */
UP_API ObjectHandle typeGetAssemblyQualifiedName(TypeHandle type);
/**
 * @brief Returns true if static type
 */
UP_API WIBool typeIsStatic(TypeHandle type);
/**
 * @brief Returns true if pointer type (T*)
 */
UP_API WIBool typeIsPointerType(TypeHandle type);

/**
 * @brief Gets assembly for image
 */
UP_API AssemblyHandle imageGetAssembly(ImageHandle image);
/**
 * @brief Gets image name
 * @return System.String
 */
UP_API ObjectHandle imageGetName(ImageHandle image);
/**
 * @brief Gets image filename
 * @return System.String
 */
UP_API ObjectHandle imageGetFilename(ImageHandle image);
/**
 * @brief Gets image entry point method
 */
UP_API MethodInfoHandle imageGetEntryPoint(ImageHandle image);
/**
 * @brief Gets number of classes in image
 */
UP_API int32_t imageGetClassCount(ImageHandle image);
/**
 * @brief Gets class by index in image
 */
UP_API ClassHandle imageGetClassAt(ImageHandle image, int32_t index);

/**
 * @brief Logs given message on trace level
 * @param message System.String object containing message
 */
UP_API void logTrace(ObjectHandle message);
/**
 * @brief Logs given message on debug level
 * @param message System.String object containing message
 */
UP_API void logDebug(ObjectHandle message);
/**
 * @brief Logs given message on info level
 * @param message System.String object containing message
 */
UP_API void logInfo(ObjectHandle message);
/**
 * @brief Logs given message on warn level
 * @param message System.String object containing message
 */
UP_API void logWarn(ObjectHandle message);
/**
 * @brief Logs given message on error level
 * @param message System.String object containing message
 */
UP_API void logError(ObjectHandle message);
/**
 * @brief Logs given message on critical level
 * @param message System.String object containing message
 */
UP_API void logCritical(ObjectHandle message);

/**
 * @brief Gets given method
 * @param identifier Name or signature to look up function by
 * @return First method found, or -1
 */
UP_API MethodHandle getMethodByIdentifier(const char *identifier);
/**
 * @brief Gets given method
 * @param addr Address to look up function by
 * @return First method found, or -1
 */
UP_API MethodHandle getMethodByAddress(int64_t addr);
/**
 * @brief Gets name of given method
 * @return System.String object
 */
UP_API ObjectHandle getMethodName(MethodHandle);
/**
 * @brief Gets signature of given method
 * @return System.String object
 */
UP_API ObjectHandle getMethodSignature(MethodHandle);
/**
 * @brief Gets address of given method
 * @return System.String object
 */
UP_API int64_t getMethodAddresss(MethodHandle);
/**
 * @brief Registers an icall
 * @param identifier Name in format "Namspace.Class$$Method"
 * @param typeSignature Signature in format as given in script.json
 * @return true if success or method already known
 */
UP_API WIBool registerICallMethod(const char *identifier, const char *typeSignature);

/**
 * @brief Adds 32 bit integer to argument list
 * @param v Value to add
 */
UP_API void addArgI32(int32_t v);
/**
 * @brief Adds 64 bit integer to argument list
 * @param v Value to add
 */
UP_API void addArgI64(int64_t v);
/**
 * @brief Adds float to argument list
 * @param v Value to add
 */
UP_API void addArgFloat(float v);
/**
 * @brief Adds double to argument list
 * @param v Value to add
 */
UP_API void addArgDouble(double v);
/**
 * @brief Adds C# object to argument list
 * @param v C# object to add
 */
UP_API void addArgObject(ObjectHandle v);
/**
 * @brief Adds null value to argument list
 */
UP_API void addArgNull();
/**
 * @brief Clears arguments
 */
UP_API void clearArgs();
/**
 * @brief Returns amount of arguments
 */
UP_API int32_t getArgCount();
/**
 * @brief Moves last argument to specified index replacing its value
 * @param index New argument index or negative value to replace return value
 * @return 1 on success, 0 on error (if argument list empty or index out of
 * range)
 */
UP_API WIBool moveArg(int32_t index);
/**
 * @brief Gets 32 bit integer stored in argument list or return value
 * @param index argument index or -1 for return value
 */
UP_API int32_t getValueI32(int32_t index = -1);
/**
 * @brief Gets 64 bit integer stored in argument list or return value
 * @param index argument index or -1 for return value
 */
UP_API int64_t getValueI64(int32_t index = -1);
/**
 * @brief Gets float stored in argument list or return value
 * @param index argument index or -1 for return value
 */
UP_API float getValueFloat(int32_t index = -1);
/**
 * @brief Gets double stored in argument list or return value
 * @param index argument index or -1 for return value
 */
UP_API double getValueDouble(int32_t index = -1);
/**
 * @brief Gets C# object stored in argument list or return value
 * @param index argument index or -1 for return value
 */
UP_API ObjectHandle getValueObject(int32_t index = -1);

/**
 * @brief Gets error message from call function
 * @return System.String object
 */
UP_API ObjectHandle getCallError();
/**
 * @brief Calls given method with previously added arguments (also clears them
 * if function call was able to be initiated)
 * @param argCount Amount of arguments previously added including this pointer (optional, may be
 * unknownArgCount)
 * @return 1 on success, 0 on failure (use getCallError to get error string)
 */
UP_API WIBool call(MethodHandle, int32_t argCount);
/**
 * @brief Calls given method with previously added arguments (also clears them
 * if function call was able to be initiated)
 * @param argCount Amount of arguments previously added including this pointer (optional, may be
 * unknownArgCount)
 * @param returnsStruct Set to true if function returns struct
 * @return 1 on success, 0 on failure (use getCallError to get error string)
 */
UP_API WIBool call2(MethodHandle, int32_t argCount, WIBool returnsStruct = false);

/**
 * @brief Hooks given method
 * @param callback Name of function to call with arguments passed by game added
 * instead when game calls given method
 * @return 1 on success, 0 on failure (if method is already hooked or is
 * invalid)
 */
UP_API WIBool hook(MethodHandle, const char *callback);
/**
 * @brief Unhooks given method. MUST NOT be called inside hook for given method!
 * @return 1 on success, 0 on failure (if method isn't hooked or is invalid)
 */
UP_API WIBool unhook(MethodHandle);
/**
 * @brief Gets original method. MUST be called inside hook!
 * @return Original method or bogus data if not called inside hook
 */
UP_API MethodHandle getOriginal();

UP_API void ImGuiBegin(const char *name = "");
UP_API void ImGuiEnd();
UP_API void ImGuiText(ObjectHandle text);
UP_DEPRECATED("Unsafe! Use ImGuiCheckbox2 instead.")
UP_API void ImGuiCheckbox(const char *label, bool *v);
UP_API WIBool ImGuiCheckbox2(const char *label, WIBool v);
UP_DEPRECATED("Unsafe! Use ImGuiCheckbox4 instead.")
UP_API WIBool ImGuiCheckbox3(const char *label, bool *v);
/**
 * @return bitmap [msb -> lsb: {..., interacted flag, new value}]
 */
UP_API int32_t ImGuiCheckbox4(const char *label, WIBool v);
UP_API WIBool ImGuiButton(const char *label);
UP_API void ImGuiSeparator();
UP_API void ImGuiSeparatorText(const char *label);

UP_API void abort(const char *message, const char *filename, int32_t lineNumber, int32_t columnNumber);

#define FFI_FUNCTION_LIST                                                                                                                                      \
    FFI_FUNCTION_LIST_ENTRY(void, dropObject, (ObjectHandle a), a)                                                                                             \
    FFI_FUNCTION_LIST_ENTRY(WIBool, isValidObject, (ObjectHandle a), a)                                                                                        \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, getNull, (), )                                                                                                       \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, toCsString, (const char *str), str)                                                                                  \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, toCsStringWithLength, (const char *str, int32_t length), str, length)                                                \
    FFI_FUNCTION_LIST_ENTRY(void, toCString, (ObjectHandle str, char *buf, int32_t maxlen), str, buf, maxlen)                                                  \
    FFI_FUNCTION_LIST_ENTRY(void, logTrace, (ObjectHandle message), message)                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(void, logDebug, (ObjectHandle message), message)                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(void, logInfo, (ObjectHandle message), message)                                                                                    \
    FFI_FUNCTION_LIST_ENTRY(void, logWarn, (ObjectHandle message), message)                                                                                    \
    FFI_FUNCTION_LIST_ENTRY(void, logError, (ObjectHandle message), message)                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(void, logCritical, (ObjectHandle message), message)                                                                                \
    FFI_FUNCTION_LIST_ENTRY(MethodHandle, getMethodByIdentifier, (const char *identifier), identifier)                                                         \
    FFI_FUNCTION_LIST_ENTRY(MethodHandle, getMethodByAddress, (int64_t addr), addr)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, getMethodName, (MethodHandle a), a)                                                                                  \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, getMethodSignature, (MethodHandle a), a)                                                                             \
    FFI_FUNCTION_LIST_ENTRY(void, addArgI32, (int32_t v), v)                                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(void, addArgI64, (int64_t v), v)                                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(void, addArgFloat, (float v), v)                                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(void, addArgDouble, (double v), v)                                                                                                 \
    FFI_FUNCTION_LIST_ENTRY(void, addArgObject, (ObjectHandle v), v)                                                                                           \
    FFI_FUNCTION_LIST_ENTRY(void, addArgNull, (), )                                                                                                            \
    FFI_FUNCTION_LIST_ENTRY(void, clearArgs, (), )                                                                                                             \
    FFI_FUNCTION_LIST_ENTRY(int32_t, getArgCount, (), )                                                                                                        \
    FFI_FUNCTION_LIST_ENTRY(WIBool, moveArg, (int32_t index), index)                                                                                           \
    FFI_FUNCTION_LIST_ENTRY(int32_t, getValueI32, (int32_t index), index)                                                                                      \
    FFI_FUNCTION_LIST_ENTRY(int64_t, getValueI64, (int32_t index), index)                                                                                      \
    FFI_FUNCTION_LIST_ENTRY(float, getValueFloat, (int32_t index), index)                                                                                      \
    FFI_FUNCTION_LIST_ENTRY(double, getValueDouble, (int32_t index), index)                                                                                    \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, getValueObject, (int32_t index), index)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, getCallError, (), )                                                                                                  \
    FFI_FUNCTION_LIST_ENTRY(WIBool, call, (MethodHandle a, int32_t argCount), a, argCount)                                                                     \
    FFI_FUNCTION_LIST_ENTRY(WIBool, hook, (MethodHandle a, const char *callback), a, callback)                                                                 \
    FFI_FUNCTION_LIST_ENTRY(WIBool, unhook, (MethodHandle a), a)                                                                                               \
    FFI_FUNCTION_LIST_ENTRY(MethodHandle, getOriginal, (), )                                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(void, ImGuiBegin, (const char *name), name)                                                                                        \
    FFI_FUNCTION_LIST_ENTRY(void, ImGuiEnd, (), )                                                                                                              \
    FFI_FUNCTION_LIST_ENTRY(void, ImGuiText, (ObjectHandle text), text)                                                                                        \
    FFI_FUNCTION_LIST_ENTRY(void, ImGuiCheckbox, (const char *label, bool *v), label, v)                                                                       \
    FFI_FUNCTION_LIST_ENTRY(WIBool, ImGuiButton, (const char *label), label)                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(void, ImGuiSeparator, (), )                                                                                                        \
    FFI_FUNCTION_LIST_ENTRY(void, ImGuiSeparatorText, (const char *label), label)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(void, abort, (const char *message, const char *filename, int32_t lineNumber, int32_t columnNumber), message, filename, lineNumber, \
                            columnNumber)                                                                                                                      \
    FFI_FUNCTION_LIST_ENTRY(ImageHandle, getImageCorlib, (), )                                                                                                 \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, getClassFromName, (ImageHandle image, const char *namespaze, const char *name), image, namespaze, name)               \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, getArrayFromClass, (ClassHandle elementClass, int32_t rank), elementClass, rank)                                      \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, createArray, (ClassHandle elementClass, int32_t length), elementClass, length)                                       \
    FFI_FUNCTION_LIST_ENTRY(void, copyArrayBytes, (ObjectHandle array, int32_t offset, int32_t length, void *to), array, offset, length, to)                   \
    FFI_FUNCTION_LIST_ENTRY(int64_t, getMethodAddresss, (MethodHandle a), a)                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(int64_t, getObjectAddress, (ObjectHandle a), a)                                                                                    \
    FFI_FUNCTION_LIST_ENTRY(GCHandle, gcCreateHandle, (ObjectHandle object, int32_t pinned), object, pinned)                                                   \
    FFI_FUNCTION_LIST_ENTRY(void, gcDeleteHandle, (GCHandle a), a)                                                                                             \
    FFI_FUNCTION_LIST_ENTRY(WIBool, call2, (MethodHandle a, int32_t argCount, WIBool returnsStruct), a, argCount, returnsStruct)                               \
    FFI_FUNCTION_LIST_ENTRY(WIBool, ImGuiCheckbox2, (const char *label, WIBool v), label, v)                                                                   \
    FFI_FUNCTION_LIST_ENTRY(WIBool, ImGuiCheckbox3, (const char *label, bool *v), label, v)                                                                    \
    FFI_FUNCTION_LIST_ENTRY(int32_t, getFtableItemCount, (), )                                                                                                 \
    FFI_FUNCTION_LIST_ENTRY(int32_t, ImGuiCheckbox4, (const char *label, WIBool v), label, v)                                                                  \
    FFI_FUNCTION_LIST_ENTRY(WIBool, registerICallMethod, (const char *identifier, const char *typeSignature), identifier, typeSignature)                       \
    FFI_FUNCTION_LIST_ENTRY(void, dropImage, (ImageHandle a), a)                                                                                               \
    FFI_FUNCTION_LIST_ENTRY(void, dropClass, (ClassHandle a), a)                                                                                               \
    FFI_FUNCTION_LIST_ENTRY(int32_t, arrayGetLength, (ObjectHandle array), array)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(int32_t, arrayGetByteLength, (ObjectHandle array), array)                                                                          \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, createArraySpecific, (ClassHandle arrayTypeInfo, int32_t length), arrayTypeInfo, length)                             \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, getBoundedArrayClass, (ClassHandle elementClass, int32_t rank, WIBool bounded), elementClass, rank, bounded)          \
    FFI_FUNCTION_LIST_ENTRY(int32_t, arrayElementSize, (ClassHandle arrayClass), arrayClass)                                                                   \
    FFI_FUNCTION_LIST_ENTRY(DomainHandle, domainGet, (), )                                                                                                     \
    FFI_FUNCTION_LIST_ENTRY(AssemblyHandle, domainAssemblyOpen, (DomainHandle domain, const char *name), domain, name)                                         \
    FFI_FUNCTION_LIST_ENTRY(int32_t, domainGetAssemblyCount, (DomainHandle domain), domain)                                                                    \
    FFI_FUNCTION_LIST_ENTRY(AssemblyHandle, domainGetAssemblyAt, (DomainHandle domain, int32_t index), domain, index)                                          \
    FFI_FUNCTION_LIST_ENTRY(ImageHandle, assemblyGetImage, (AssemblyHandle assembly), assembly)                                                                \
    FFI_FUNCTION_LIST_ENTRY(TypeHandle, classEnumBaseType, (ClassHandle klass), klass)                                                                         \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classIsGeneric, (ClassHandle klass), klass)                                                                                \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classIsInflated, (ClassHandle klass), klass)                                                                               \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classIsAssignableFrom, (ClassHandle klass, ClassHandle oklass), klass, oklass)                                             \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classIsSubclassOf, (ClassHandle klass, ClassHandle klassc, WIBool checkInterfaces), klass, klassc, checkInterfaces)        \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classHasParent, (ClassHandle klass, ClassHandle klassc), klass, klassc)                                                    \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, getClassFromType, (TypeHandle type), type)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, getClassFromSystemType, (ObjectHandle reflectionType), reflectionType)                                                \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, classGetElementClass, (ClassHandle klass), klass)                                                                     \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, classGetName, (ClassHandle klass), klass)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, classGetNamespace, (ClassHandle klass), klass)                                                                       \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, classGetParent, (ClassHandle klass), klass)                                                                           \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, classGetDeclaringType, (ClassHandle klass), klass)                                                                    \
    FFI_FUNCTION_LIST_ENTRY(int32_t, classInstanceSize, (ClassHandle klass), klass)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(int32_t, classNumFields, (ClassHandle klass), klass)                                                                               \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classIsValueType, (ClassHandle klass), klass)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classIsBlittable, (ClassHandle klass), klass)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(int32_t, classGetFlags, (ClassHandle klass), klass)                                                                                \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classIsAbstract, (ClassHandle klass), klass)                                                                               \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classIsInterface, (ClassHandle klass), klass)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(int32_t, classArrayElementSize, (ClassHandle klass), klass)                                                                        \
    FFI_FUNCTION_LIST_ENTRY(TypeHandle, classGetType, (ClassHandle klass), klass)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(int32_t, classGetTypeToken, (ClassHandle klass), klass)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classHasAttribute, (ClassHandle klass, ClassHandle attrClass), klass, attrClass)                                           \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classHasReferences, (ClassHandle klass), klass)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(WIBool, classIsEnum, (ClassHandle klass), klass)                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(ImageHandle, classGetImage, (ClassHandle klass), klass)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, classGetAssemblyName, (ClassHandle klass), klass)                                                                    \
    FFI_FUNCTION_LIST_ENTRY(int32_t, classGetRank, (ClassHandle klass), klass)                                                                                 \
    FFI_FUNCTION_LIST_ENTRY(int32_t, classGetDataSize, (ClassHandle klass), klass)                                                                             \
    FFI_FUNCTION_LIST_ENTRY(EventHandle, classGetEventAt, (ClassHandle klass, int32_t index), klass, index)                                                    \
    FFI_FUNCTION_LIST_ENTRY(FieldHandle, classGetFieldAt, (ClassHandle klass, int32_t index), klass, index)                                                    \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, classGetNestedTypeAt, (ClassHandle klass, int32_t index), klass, index)                                               \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, classGetInterfaceAt, (ClassHandle klass, int32_t index), klass, index)                                                \
    FFI_FUNCTION_LIST_ENTRY(PropertyHandle, classGetPropertyAt, (ClassHandle klass, int32_t index), klass, index)                                              \
    FFI_FUNCTION_LIST_ENTRY(MethodInfoHandle, classGetMethodAt, (ClassHandle klass, int32_t index), klass, index)                                              \
    FFI_FUNCTION_LIST_ENTRY(PropertyHandle, classGetPropertyFromName, (ClassHandle klass, const char *name), klass, name)                                      \
    FFI_FUNCTION_LIST_ENTRY(FieldHandle, classGetFieldFromName, (ClassHandle klass, const char *name), klass, name)                                            \
    FFI_FUNCTION_LIST_ENTRY(MethodInfoHandle, classGetMethodFromName, (ClassHandle klass, const char *name, int32_t argsCount), klass, name, argsCount)        \
    FFI_FUNCTION_LIST_ENTRY(void, exceptionRaise, (ObjectHandle ex), ex)                                                                                       \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, exceptionFromNameMsg, (ImageHandle image, const char *namespaze, const char *name, const char *msg), image,          \
                            namespaze, name, msg)                                                                                                              \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, exceptionGetArgumentNull, (const char *arg), arg)                                                                    \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, exceptionFormat, (ObjectHandle ex), ex)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, exceptionFormatStackTrace, (ObjectHandle ex), ex)                                                                    \
    FFI_FUNCTION_LIST_ENTRY(void, exceptionUnhandled, (ObjectHandle ex), ex)                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(int32_t, fieldGetFlags, (FieldHandle field), field)                                                                                \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, fieldGetName, (FieldHandle field), field)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, fieldGetParent, (FieldHandle field), field)                                                                           \
    FFI_FUNCTION_LIST_ENTRY(int32_t, fieldGetOffset, (FieldHandle field), field)                                                                               \
    FFI_FUNCTION_LIST_ENTRY(TypeHandle, fieldGetType, (FieldHandle field), field)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(WIBool, fieldHasAttribute, (FieldHandle field, ClassHandle attrClass), field, attrClass)                                           \
    FFI_FUNCTION_LIST_ENTRY(void, fieldSetValueI32, (ObjectHandle obj, FieldHandle field, int32_t value), obj, field, value)                                   \
    FFI_FUNCTION_LIST_ENTRY(void, fieldSetValueI64, (ObjectHandle obj, FieldHandle field, int64_t value), obj, field, value)                                   \
    FFI_FUNCTION_LIST_ENTRY(void, fieldSetValueFloat, (ObjectHandle obj, FieldHandle field, float value), obj, field, value)                                   \
    FFI_FUNCTION_LIST_ENTRY(void, fieldSetValueDouble, (ObjectHandle obj, FieldHandle field, double value), obj, field, value)                                 \
    FFI_FUNCTION_LIST_ENTRY(void, fieldSetValueObject, (ObjectHandle obj, FieldHandle field, ObjectHandle value), obj, field, value)                           \
    FFI_FUNCTION_LIST_ENTRY(int32_t, fieldGetValueI32, (ObjectHandle obj, FieldHandle field), obj, field)                                                      \
    FFI_FUNCTION_LIST_ENTRY(int64_t, fieldGetValueI64, (ObjectHandle obj, FieldHandle field), obj, field)                                                      \
    FFI_FUNCTION_LIST_ENTRY(float, fieldGetValueFloat, (ObjectHandle obj, FieldHandle field), obj, field)                                                      \
    FFI_FUNCTION_LIST_ENTRY(double, fieldGetValueDouble, (ObjectHandle obj, FieldHandle field), obj, field)                                                    \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, fieldGetValueObject, (ObjectHandle obj, FieldHandle field), obj, field)                                              \
    FFI_FUNCTION_LIST_ENTRY(int32_t, fieldStaticGetValueI32, (FieldHandle field), field)                                                                       \
    FFI_FUNCTION_LIST_ENTRY(int64_t, fieldStaticGetValueI64, (FieldHandle field), field)                                                                       \
    FFI_FUNCTION_LIST_ENTRY(float, fieldStaticGetValueFloat, (FieldHandle field), field)                                                                       \
    FFI_FUNCTION_LIST_ENTRY(double, fieldStaticGetValueDouble, (FieldHandle field), field)                                                                     \
    FFI_FUNCTION_LIST_ENTRY(void, fieldStaticSetValueI32, (FieldHandle field, int32_t value), field, value)                                                    \
    FFI_FUNCTION_LIST_ENTRY(void, fieldStaticSetValueI64, (FieldHandle field, int64_t value), field, value)                                                    \
    FFI_FUNCTION_LIST_ENTRY(void, fieldStaticSetValueFloat, (FieldHandle field, float value), field, value)                                                    \
    FFI_FUNCTION_LIST_ENTRY(void, fieldStaticSetValueDouble, (FieldHandle field, double value), field, value)                                                  \
    FFI_FUNCTION_LIST_ENTRY(WIBool, fieldIsLiteral, (FieldHandle field), field)                                                                                \
    FFI_FUNCTION_LIST_ENTRY(void, gcCollect, (int32_t maxGenerations), maxGenerations)                                                                         \
    FFI_FUNCTION_LIST_ENTRY(int32_t, gcCollectALittle, (), )                                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(void, gcDisable, (), )                                                                                                             \
    FFI_FUNCTION_LIST_ENTRY(void, gcEnable, (), )                                                                                                              \
    FFI_FUNCTION_LIST_ENTRY(WIBool, gcIsDisabled, (), )                                                                                                        \
    FFI_FUNCTION_LIST_ENTRY(int64_t, gcGetMaxTimeSliceNs, (), )                                                                                                \
    FFI_FUNCTION_LIST_ENTRY(void, gcSetMaxTimeSliceNs, (int64_t ns), ns)                                                                                       \
    FFI_FUNCTION_LIST_ENTRY(WIBool, gcIsIncremental, (), )                                                                                                     \
    FFI_FUNCTION_LIST_ENTRY(int64_t, gcGetUsedSize, (), )                                                                                                      \
    FFI_FUNCTION_LIST_ENTRY(int64_t, gcGetHeapSize, (), )                                                                                                      \
    FFI_FUNCTION_LIST_ENTRY(WIBool, gcHasStrictWBarriers, (), )                                                                                                \
    FFI_FUNCTION_LIST_ENTRY(void, gcStopWorld, (), )                                                                                                           \
    FFI_FUNCTION_LIST_ENTRY(void, gcStartWorld, (), )                                                                                                          \
    FFI_FUNCTION_LIST_ENTRY(GCHandle, gcCreateWeakHandle, (ObjectHandle object, WIBool trackResurrection), object, trackResurrection)                          \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, gcGetTarget, (GCHandle handle), handle)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(TypeHandle, methodInfoGetReturnType, (MethodInfoHandle method), method)                                                            \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, methodInfoGetDeclaringType, (MethodInfoHandle method), method)                                                        \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, methodInfoGetName, (MethodInfoHandle method), method)                                                                \
    FFI_FUNCTION_LIST_ENTRY(MethodInfoHandle, methodInfoGetFromReflection, (ObjectHandle reflectionMethod), reflectionMethod)                                  \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, methodInfoGetObject, (MethodInfoHandle method, ClassHandle refclass), method, refclass)                              \
    FFI_FUNCTION_LIST_ENTRY(WIBool, methodInfoIsGeneric, (MethodInfoHandle method), method)                                                                    \
    FFI_FUNCTION_LIST_ENTRY(WIBool, methodInfoIsInflated, (MethodInfoHandle method), method)                                                                   \
    FFI_FUNCTION_LIST_ENTRY(WIBool, methodInfoIsInstance, (MethodInfoHandle method), method)                                                                   \
    FFI_FUNCTION_LIST_ENTRY(int32_t, methodInfoGetParamCount, (MethodInfoHandle method), method)                                                               \
    FFI_FUNCTION_LIST_ENTRY(TypeHandle, methodInfoGetParamType, (MethodInfoHandle method, int32_t index), method, index)                                       \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, methodInfoGetClass, (MethodInfoHandle method), method)                                                                \
    FFI_FUNCTION_LIST_ENTRY(WIBool, methodInfoHasAttribute, (MethodInfoHandle method, ClassHandle attrClass), method, attrClass)                               \
    FFI_FUNCTION_LIST_ENTRY(int32_t, methodInfoGetFlags, (MethodInfoHandle method), method)                                                                    \
    FFI_FUNCTION_LIST_ENTRY(int32_t, methodInfoGetImplFlags, (MethodInfoHandle method), method)                                                                \
    FFI_FUNCTION_LIST_ENTRY(int32_t, methodInfoGetToken, (MethodInfoHandle method), method)                                                                    \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, methodInfoGetParamName, (MethodInfoHandle method, int32_t index), method, index)                                     \
    FFI_FUNCTION_LIST_ENTRY(int32_t, propertyGetFlags, (PropertyHandle prop), prop)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(MethodInfoHandle, propertyGetGetMethod, (PropertyHandle prop), prop)                                                               \
    FFI_FUNCTION_LIST_ENTRY(MethodInfoHandle, propertyGetSetMethod, (PropertyHandle prop), prop)                                                               \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, propertyGetName, (PropertyHandle prop), prop)                                                                        \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, propertyGetParent, (PropertyHandle prop), prop)                                                                       \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, objectGetClass, (ObjectHandle obj), obj)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(int32_t, objectGetSize, (ObjectHandle obj), obj)                                                                                   \
    FFI_FUNCTION_LIST_ENTRY(MethodInfoHandle, objectGetVirtualMethod, (ObjectHandle obj, MethodInfoHandle method), obj, method)                                \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, objectNew, (ClassHandle klass), klass)                                                                               \
    FFI_FUNCTION_LIST_ENTRY(int32_t, objectUnboxI32, (ObjectHandle obj), obj)                                                                                  \
    FFI_FUNCTION_LIST_ENTRY(int64_t, objectUnboxI64, (ObjectHandle obj), obj)                                                                                  \
    FFI_FUNCTION_LIST_ENTRY(float, objectUnboxFloat, (ObjectHandle obj), obj)                                                                                  \
    FFI_FUNCTION_LIST_ENTRY(double, objectUnboxDouble, (ObjectHandle obj), obj)                                                                                \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, valueBoxI32, (ClassHandle klass, int32_t value), klass, value)                                                       \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, valueBoxI64, (ClassHandle klass, int64_t value), klass, value)                                                       \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, valueBoxFloat, (ClassHandle klass, float value), klass, value)                                                       \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, valueBoxDouble, (ClassHandle klass, double value), klass, value)                                                     \
    FFI_FUNCTION_LIST_ENTRY(void, monitorEnter, (ObjectHandle obj), obj)                                                                                       \
    FFI_FUNCTION_LIST_ENTRY(WIBool, monitorTryEnter, (ObjectHandle obj, int32_t timeout), obj, timeout)                                                        \
    FFI_FUNCTION_LIST_ENTRY(void, monitorExit, (ObjectHandle obj), obj)                                                                                        \
    FFI_FUNCTION_LIST_ENTRY(void, monitorPulse, (ObjectHandle obj), obj)                                                                                       \
    FFI_FUNCTION_LIST_ENTRY(void, monitorPulseAll, (ObjectHandle obj), obj)                                                                                    \
    FFI_FUNCTION_LIST_ENTRY(void, monitorWait, (ObjectHandle obj), obj)                                                                                        \
    FFI_FUNCTION_LIST_ENTRY(WIBool, monitorTryWait, (ObjectHandle obj, int32_t timeout), obj, timeout)                                                         \
    FFI_FUNCTION_LIST_ENTRY(void, runtimeClassInit, (ClassHandle klass), klass)                                                                                \
    FFI_FUNCTION_LIST_ENTRY(void, runtimeObjectInit, (ObjectHandle obj), obj)                                                                                  \
    FFI_FUNCTION_LIST_ENTRY(WIBool, runtimeObjectInitException, (ObjectHandle obj), obj)                                                                       \
    FFI_FUNCTION_LIST_ENTRY(void, runtimeUnhandledExceptionPolicySet, (int32_t value), value)                                                                  \
    FFI_FUNCTION_LIST_ENTRY(int32_t, stringGetLength, (ObjectHandle str), str)                                                                                 \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, stringIntern, (ObjectHandle str), str)                                                                               \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, stringIsInterned, (ObjectHandle str), str)                                                                           \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, typeGetObject, (TypeHandle type), type)                                                                              \
    FFI_FUNCTION_LIST_ENTRY(int32_t, typeGetType, (TypeHandle type), type)                                                                                     \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, typeGetClassOrElementClass, (TypeHandle type), type)                                                                  \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, typeGetName, (TypeHandle type), type)                                                                                \
    FFI_FUNCTION_LIST_ENTRY(WIBool, typeIsByRef, (TypeHandle type), type)                                                                                      \
    FFI_FUNCTION_LIST_ENTRY(int32_t, typeGetAttrs, (TypeHandle type), type)                                                                                    \
    FFI_FUNCTION_LIST_ENTRY(WIBool, typeEquals, (TypeHandle a, TypeHandle b), a, b)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, typeGetAssemblyQualifiedName, (TypeHandle type), type)                                                               \
    FFI_FUNCTION_LIST_ENTRY(WIBool, typeIsStatic, (TypeHandle type), type)                                                                                     \
    FFI_FUNCTION_LIST_ENTRY(WIBool, typeIsPointerType, (TypeHandle type), type)                                                                                \
    FFI_FUNCTION_LIST_ENTRY(AssemblyHandle, imageGetAssembly, (ImageHandle image), image)                                                                      \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, imageGetName, (ImageHandle image), image)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, imageGetFilename, (ImageHandle image), image)                                                                        \
    FFI_FUNCTION_LIST_ENTRY(MethodInfoHandle, imageGetEntryPoint, (ImageHandle image), image)                                                                  \
    FFI_FUNCTION_LIST_ENTRY(int32_t, imageGetClassCount, (ImageHandle image), image)                                                                           \
    FFI_FUNCTION_LIST_ENTRY(ClassHandle, imageGetClassAt, (ImageHandle image, int32_t index), image, index)                                                    \
    FFI_FUNCTION_LIST_ENTRY(void, dropDomain, (DomainHandle a), a)                                                                                             \
    FFI_FUNCTION_LIST_ENTRY(void, dropAssembly, (AssemblyHandle a), a)                                                                                         \
    FFI_FUNCTION_LIST_ENTRY(void, dropType, (TypeHandle a), a)                                                                                                 \
    FFI_FUNCTION_LIST_ENTRY(void, dropMethodInfo, (MethodInfoHandle a), a)                                                                                     \
    FFI_FUNCTION_LIST_ENTRY(void, dropField, (FieldHandle a), a)                                                                                               \
    FFI_FUNCTION_LIST_ENTRY(void, dropProperty, (PropertyHandle a), a)                                                                                         \
    FFI_FUNCTION_LIST_ENTRY(void, dropEvent, (EventHandle a), a)                                                                                               \
    FFI_FUNCTION_LIST_ENTRY(void, dropValue, (ValueHandle a), a)                                                                                               \
    FFI_FUNCTION_LIST_ENTRY(ValueHandle, objectUnboxValue, (ObjectHandle obj), obj)                                                                            \
    FFI_FUNCTION_LIST_ENTRY(ObjectHandle, valueBox, (ClassHandle klass, ValueHandle value), klass, value)

// Make sure signatures match
#ifndef FFI_NOSTL
#define FFI_FUNCTION_LIST_ENTRY(return_type, fnc, arguments, ...)                                                                                              \
    static_assert(tl::is_same<decltype(fnc), UP_API return_type arguments>(), "Signature of FFI function " #fnc " does not match that in function list");
FFI_FUNCTION_LIST
#undef FFI_FUNCTION_LIST_ENTRY
#endif

#ifdef FFI_USES_FTABLE
}
#endif

// Store local FFI function count
constexpr static int32_t getLocalFtableItemCount() {
    int32_t fres = 0;
#define FFI_FUNCTION_LIST_ENTRY(return_type, fnc, arguments, ...) ++fres;
    FFI_FUNCTION_LIST
#undef FFI_FUNCTION_LIST_ENTRY
    return fres;
}

#if !defined(FFI_EXT) || defined(FFI_USES_FTABLE)
#ifndef FFI_EXT
struct Exports
#else
struct Imports
#endif
{
#ifndef FFI_EXT
#define _FFI_FTABLE_DEFAULT_ASSIGN(name) = FFIInterface::name
#define _FFI_TYPE_OF(name) decltype(FFIInterface::name)
#else
#define _FFI_FTABLE_DEFAULT_ASSIGN(name)
#define _FFI_TYPE_OF(name) decltype(Signatures::name)
#endif
#define FFI_FUNCTION_LIST_ENTRY(_, name, ...) _FFI_TYPE_OF(name) * name _FFI_FTABLE_DEFAULT_ASSIGN(name);
    FFI_FUNCTION_LIST
#undef FFI_FUNCTION_LIST_ENTRY
};
#endif

#ifndef FFI_USES_FTABLE
#define FFI_USE_FTABLE FFIInterface::
#else
#define FFI_USE_FTABLE (FFI_FTABLE).
#endif

#ifndef FFI_NOSTL
#ifdef FFI_USES_FTABLE
const static
#else
constexpr
#endif
    tl::tuple functions = {
#define FFI_FUNCTION_LIST_ENTRY(_, fnc, ...) &FFI_USE_FTABLE fnc,
        FFI_FUNCTION_LIST
#undef FFI_FUNCTION_LIST_ENTRY
};
#endif

enum class Functions {
#define FFI_FUNCTION_LIST_ENTRY(_, fnc, ...) fnc,
    FFI_FUNCTION_LIST
#undef FFI_FUNCTION_LIST_ENTRY
        __count
};

#ifdef FFI_EXT
namespace Helpers {
namespace {
template <size_t N> struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        for (unsigned it = 0; it != N; ++it)
            value[it] = str[it];
    }

    char value[N];

    operator const char *() const { return value; }
};

FFIInterface::ObjectHandle createCsString() { return FFI_USE_FTABLE toCsStringWithLength(nullptr, 0); }

bool ImGuiCheckbox(const char *label, bool *state) {
    const uint8_t fres = FFI_USE_FTABLE ImGuiCheckbox4(label, *state);
    *state = fres & 0b01;
    return fres & 0b10;
}

void addArg(int32_t v) { FFI_USE_FTABLE addArgI32(v); }
void addArg(int64_t v) { FFI_USE_FTABLE addArgI64(v); }
void addArg(bool v) { FFI_USE_FTABLE addArgI32(v); }
void addArg(float v) { FFI_USE_FTABLE addArgFloat(v); }
void addArg(double v) { FFI_USE_FTABLE addArgDouble(v); }
void addArg(FFIInterface::ObjectHandle v) { FFI_USE_FTABLE addArgObject(v); }
void addArg(decltype(nullptr)) { FFI_USE_FTABLE addArgNull(); }

template <typename T> T getArg(int idx) = delete;
template <> void getArg<void>(int idx) {}
template <> int32_t getArg<int32_t>(int idx) { return FFI_USE_FTABLE getValueI32(idx); }
template <> int64_t getArg<int64_t>(int idx) { return FFI_USE_FTABLE getValueI64(idx); }
template <> bool getArg<bool>(int idx) { return (FFI_USE_FTABLE getValueI32(idx)) & 0xff; }
template <> float getArg<float>(int idx) { return FFI_USE_FTABLE getValueFloat(idx); }
template <> double getArg<double>(int idx) { return FFI_USE_FTABLE getValueDouble(idx); }
template <> FFIInterface::ObjectHandle getArg<FFIInterface::ObjectHandle>(int idx) { return FFI_USE_FTABLE getValueObject(idx); }

template <typename T> T getReturnValue() { return getArg<T>(-1); }
template <typename T> void setReturnValue(T v) {
    addArg(v);
    FFI_USE_FTABLE moveArg(-1);
}

inline void addArgs() {}
template <typename Arg0, typename... Args> void addArgs(Arg0 arg0, Args... args) {
    addArg(arg0);
    addArgs(args...);
}

template <unsigned maxlen = 64> const char *getCString(FFIInterface::ObjectHandle str) {
    static char buf[maxlen];
    FFI_USE_FTABLE toCString(str, buf, maxlen);
    return buf;
}

FFIInterface::MethodHandle getMethod(const char *identifier) { return FFI_USE_FTABLE getMethodByIdentifier(identifier); }
FFIInterface::MethodHandle getMethod(int64_t address) { return FFI_USE_FTABLE getMethodByAddress(address); }

template <StringLiteral identifier> FFIInterface::MethodHandle getMethodCached() {
    static FFIInterface::MethodHandle fres = FFI_USE_FTABLE getMethodByIdentifier(identifier);
    return fres;
}

ClassHandle getDataArrayType() {
    static const auto byteArrayType = []() {
        ImageHandle corlib = FFI_USE_FTABLE getImageCorlib();
        ClassHandle byteType = FFI_USE_FTABLE getClassFromName(corlib, "System", "Byte");
        ClassHandle byteArrayType = FFI_USE_FTABLE getArrayFromClass(byteType, 1);
        FFI_USE_FTABLE dropImage(corlib);
        FFI_USE_FTABLE dropClass(byteType);
        return byteArrayType;
    }();
    return byteArrayType;
}
ObjectHandle getDataArray(unsigned length) { return FFI_USE_FTABLE createArray(getDataArrayType(), length); }

bool call_error;
template <StringLiteral identifier, typename returnT = void, typename... Args> returnT call(Args... args) {
    FFI_USE_FTABLE clearArgs();
    addArgs(args...);
    call_error = !FFI_USE_FTABLE call2(getMethodCached<identifier>(), FFIInterface::unknownArgCount, false);
    return getReturnValue<returnT>();
}
template <StringLiteral identifier, typename returnT, typename... Args> returnT callRetStruct(Args... args) {
    FFI_USE_FTABLE clearArgs();
    addArgs(args...);
    ObjectHandle byteArray = getDataArray(64);
    returnT fres;
    FFI_USE_FTABLE addArgObject(byteArray);
    FFI_USE_FTABLE moveArg(-1);
    call_error = !FFI_USE_FTABLE call2(getMethodCached<identifier>(), FFIInterface::unknownArgCount, true);
    FFI_USE_FTABLE copyArrayBytes(byteArray, 0, sizeof(fres), &fres);
    FFI_USE_FTABLE dropObject(byteArray);
    return fres;
}

class GameHook {
    FFIInterface::MethodHandle method = MethodHandle::Invalid;
    const char *callback_name = nullptr;
    bool active = false;

public:
    GameHook(FFIInterface::MethodHandle method, const char *callback) : method(method), callback_name(callback) { restore(); }
    GameHook() {}
    ~GameHook() { release(); }

    // Move only
    GameHook(GameHook&& o) noexcept : method(o.method), callback_name(o.callback_name), active(o.active) { o.active = false; }

    GameHook& operator=(GameHook&& o) noexcept {
        if (this != &o) {
            release();
            method = o.method;
            callback_name = o.callback_name;
            active = o.active;
            o.active = false;
        }
        return *this;
    }

    bool release() {
        if (active) {
            active = !FFI_USE_FTABLE unhook(method);
            return !active;
        }
        return false;
    }

    bool restore() {
        if (!active) {
            active = FFI_USE_FTABLE hook(method, callback_name);
            return active;
        }
        return false;
    }

    FFIInterface::MethodHandle target() const { return method; }
    bool isActive() const { return active; }
};

#ifndef FFI_NOSTL_CONTAINERS
class GameHookPool {
    tl::vector<tl::shared_ptr<GameHook>> hooks;

public:
    tl::shared_ptr<GameHook> add(FFIInterface::MethodHandle method, const char *callback) {
        auto hook = GameHook(method, callback);
        if (hook.isActive()) {
            auto ptr = tl::make_shared<GameHook>(tl::move(hook));
            hooks.push_back(ptr);
            return ptr;
        }
        return nullptr;
    }

    template <StringLiteral identifier> tl::shared_ptr<GameHook> add(const char *callback) {
        auto hook = GameHook(getMethodCached<identifier>(), callback);
        if (hook.isActive()) {
            auto ptr = tl::make_shared<GameHook>(tl::move(hook));
            hooks.push_back(ptr);
            return ptr;
        }
        return nullptr;
    }

    tl::shared_ptr<GameHook> get(FFIInterface::MethodHandle method) const {
        for (auto& hook : hooks) {
            if (hook->target() == method) {
                return hook;
            }
        }
        return nullptr;
    }

    void remove(FFIInterface::MethodHandle method) {
        auto it = tl::remove_if(hooks.begin(), hooks.end(), [method](const auto& hook) { return hook->target() == method; });
        hooks.erase(it, hooks.end());
    }

    void clear() { hooks.clear(); }
};

bool hookToggle(const char *description, tl::optional<GameHook>& hook, bool& boolean, FFIInterface::MethodHandle method, const char *hookFnc) {
    if (ImGuiCheckbox(description, &boolean)) {
        if (boolean) {
            auto newHook = GameHook(method, hookFnc);
            if (!newHook.isActive()) {
                boolean = false;
                return false;
            }
            hook.emplace(tl::move(newHook));
        } else {
            hook.reset();
        }
        return true;
    }
    return false;
}

bool hookToggle(const char *description, GameHookPool& hookPool, bool& boolean, FFIInterface::MethodHandle method, const char *hookFnc) {
    if (ImGuiCheckbox(description, &boolean)) {
        if (boolean)
            boolean = hookPool.add(method, hookFnc) != nullptr;
        else
            hookPool.remove(method);
        return true;
    }
    return false;
}
#endif

#ifndef FFI_NOSTL_CONTAINERS
// Helper to format managed exceptions
inline tl::string format_exception(ObjectHandle ex) {
    ObjectHandle formatted = FFI_USE_FTABLE exceptionFormat(ex);
    int32_t len = FFI_USE_FTABLE stringGetLength(formatted);
    tl::string result(len, '\0');
    FFI_USE_FTABLE toCString(formatted, result.data(), len);
    FFI_USE_FTABLE dropObject(formatted);
    return result;
}

// Helper to format managed stacktraces
inline tl::string format_stacktrace(ObjectHandle ex) {
    ObjectHandle formatted = FFI_USE_FTABLE exceptionFormatStackTrace(ex);
    int32_t len = FFI_USE_FTABLE stringGetLength(formatted);
    tl::string result(len, '\0');
    FFI_USE_FTABLE toCString(formatted, result.data(), len);
    FFI_USE_FTABLE dropObject(formatted);
    return result;
}
#endif

// FFI handle deleters
template <typename HandleT> struct UniqueHandleDeleter;

#define FFI_DEFINE_DELETER(Handle, DropFunc)                                                                                                                   \
    template <> struct UniqueHandleDeleter<Handle> {                                                                                                           \
        void operator()(Handle h) const { FFI_USE_FTABLE DropFunc(h); }                                                                                        \
    }
FFI_DEFINE_DELETER(ObjectHandle, dropObject);
FFI_DEFINE_DELETER(ValueHandle, dropValue);
FFI_DEFINE_DELETER(ImageHandle, dropImage);
FFI_DEFINE_DELETER(ClassHandle, dropClass);
FFI_DEFINE_DELETER(DomainHandle, dropDomain);
FFI_DEFINE_DELETER(AssemblyHandle, dropAssembly);
FFI_DEFINE_DELETER(TypeHandle, dropType);
FFI_DEFINE_DELETER(MethodInfoHandle, dropMethodInfo);
FFI_DEFINE_DELETER(FieldHandle, dropField);
FFI_DEFINE_DELETER(PropertyHandle, dropProperty);
FFI_DEFINE_DELETER(EventHandle, dropEvent);
#undef FFI_DEFINE_DELETER

// FFI unique handle (like unique_ptr)
template <typename HandleT, typename DeleterT = UniqueHandleDeleter<HandleT>> class UniqueHandle {
    HandleT ptr{};
    bool owned = true;

public:
    constexpr UniqueHandle() noexcept : ptr{} {}
    constexpr UniqueHandle(HandleT ptr, bool own) noexcept : ptr(ptr), owned(own) {}

    ~UniqueHandle() { reset(); }

    UniqueHandle(const UniqueHandle&) = delete;
    UniqueHandle& operator=(const UniqueHandle&) = delete;

    UniqueHandle(UniqueHandle&& other) noexcept : ptr(other.release()) {}

    explicit operator HandleT() const noexcept { return ptr; }
    explicit operator bool() const noexcept { return ptr != HandleT{}; }

    static inline UniqueHandle Own(HandleT handle) noexcept { return UniqueHandle{handle, true}; }
    static inline UniqueHandle Lend(HandleT handle) noexcept { return UniqueHandle{handle, false}; }

    UniqueHandle& operator=(UniqueHandle&& other) noexcept {
        if (this != &other)
            reset(other.release());
        return *this;
    }

    HandleT release() noexcept {
        HandleT oldHandle = ptr;
        ptr = HandleT{};
        return oldHandle;
    }

    void reset(HandleT handle = HandleT{}) noexcept {
        HandleT oldHandle = ptr;
        ptr = handle;
        if (owned && oldHandle != HandleT{})
            DeleterT{}(oldHandle);
    }

    HandleT get() const noexcept { return ptr; }
};

// GC handle wrapper
struct GcHandle {
    FFIInterface::GCHandle handle{0};

    GcHandle() = default;
    explicit GcHandle(ObjectHandle obj, bool pinned = false) { handle = FFI_USE_FTABLE gcCreateHandle(obj, pinned ? 1 : 0); }

    static GcHandle weak(ObjectHandle obj, bool track_resurrection = false) {
        GcHandle result;
        result.handle = FFI_USE_FTABLE gcCreateWeakHandle(obj, track_resurrection ? 1 : 0);
        return result;
    }

    ~GcHandle() {
        if (handle != GCHandle::Invalid)
            FFI_USE_FTABLE gcDeleteHandle(handle);
    }

    GcHandle(const GcHandle&) = delete;
    GcHandle& operator=(const GcHandle&) = delete;

    GcHandle(GcHandle&& o) noexcept : handle(o.handle) { o.handle = GCHandle::Invalid; }

    GcHandle& operator=(GcHandle&& o) noexcept {
        if (this != &o) {
            if (handle != GCHandle::Invalid)
                FFI_USE_FTABLE gcDeleteHandle(handle);
            handle = o.handle;
            o.handle = GCHandle::Invalid;
        }
        return *this;
    }

    ObjectHandle target() const { return FFI_USE_FTABLE gcGetTarget(handle); }

    explicit operator bool() const noexcept { return handle != GCHandle::Invalid && target() != ObjectHandle{}; }
};

#ifndef FFI_NOSTL_CONTAINERS
// Thin wrappers
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
    UniqueHandle<ObjectHandle> ptr;

    explicit operator bool() const { return ptr && ptr.get() != ObjectHandle{} && FFI isValidObject(ptr.get()); }

    Class klass() const;

    uint32_t size() const { return FFI_USE_FTABLE objectGetSize(ptr.get()); }

    Method virtual_method(const Method& m) const;

    template <typename T> auto unbox() {
        if constexpr (tl::is_same<T, int32_t>())
            return FFI_USE_FTABLE objectUnboxI32(ptr.get());
        if constexpr (tl::is_same<T, int64_t>())
            return FFI_USE_FTABLE objectUnboxI64(ptr.get());
        if constexpr (tl::is_same<T, float>())
            return FFI_USE_FTABLE objectUnboxFloat(ptr.get());
        if constexpr (tl::is_same<T, double>())
            return FFI_USE_FTABLE objectUnboxDouble(ptr.get());
        static_assert(false, "Type can't be unboxed");
    }

    GcHandle gchandle(bool pinned = false) { return GcHandle(ptr.get(), pinned); }
};

struct String : Object {
    String() = default;
    explicit String(ObjectHandle s) { ptr.reset(s); }

    static String New(tl::string_view s) {
        String result;
        result.ptr.reset(FFI_USE_FTABLE toCsStringWithLength(s.data(), s.size()));
        return result;
    }

    int32_t length() const { return FFI_USE_FTABLE stringGetLength(ptr.get()); }

    tl::string std() const {
        int32_t len = FFI_USE_FTABLE stringGetLength(ptr.get());
        tl::string result(len, '\0');
        FFI_USE_FTABLE toCString(ptr.get(), result.data(), len);
        return result;
    }
};

struct Domain {
    UniqueHandle<DomainHandle> ptr;

    static Domain get() {
        Domain result;
        result.ptr.reset(FFI_USE_FTABLE domainGet());
        return result;
    }

    explicit operator bool() const { return ptr && ptr.get() != DomainHandle{}; }

    Assembly open_assembly(const char *name);
    tl::vector<Assembly> get_assemblies() const;
};

struct Assembly {
    UniqueHandle<AssemblyHandle> ptr;

    explicit operator bool() const { return ptr && ptr.get() != AssemblyHandle{}; }

    Image image() const;
};

struct Image {
    UniqueHandle<ImageHandle> ptr;

    explicit operator bool() const { return ptr && ptr.get() != ImageHandle{}; }

    static Image get_corlib() {
        Image result;
        result.ptr.reset(FFI_USE_FTABLE getImageCorlib());
        return result;
    }

    String name() const { return String(FFI_USE_FTABLE imageGetName(ptr.get())); }

    String filename() const { return String(FFI_USE_FTABLE imageGetFilename(ptr.get())); }

    MethodInfoHandle entry_point() const { return FFI_USE_FTABLE imageGetEntryPoint(ptr.get()); }

    size_t class_count() const { return FFI_USE_FTABLE imageGetClassCount(ptr.get()); }

    Class get_class(size_t index) const;
    Assembly get_assembly() const;
};

struct Type {
    UniqueHandle<TypeHandle> ptr;

    explicit operator bool() const { return ptr && ptr.get() != TypeHandle{}; }

    bool is_byref() const { return FFI_USE_FTABLE typeIsByRef(ptr.get()); }
    bool is_static() const { return FFI_USE_FTABLE typeIsStatic(ptr.get()); }
    bool is_pointer() const { return FFI_USE_FTABLE typeIsPointerType(ptr.get()); }

    uint32_t attrs() const { return FFI_USE_FTABLE typeGetAttrs(ptr.get()); }
    int kind() const { return FFI_USE_FTABLE typeGetType(ptr.get()); }

    String name_owned() const { return String(FFI_USE_FTABLE typeGetName(ptr.get())); }

    String assembly_qualified_name_owned() const { return String(FFI_USE_FTABLE typeGetAssemblyQualifiedName(ptr.get())); }

    Class class_or_element() const;

    bool equals(Type other) const { return FFI_USE_FTABLE typeEquals(ptr.get(), other.ptr.get()); }
};

struct Class {
    UniqueHandle<ClassHandle> ptr;

    explicit operator bool() const { return ptr && ptr.get() != ClassHandle{}; }

    static Class from_type(TypeHandle t) {
        Class result;
        result.ptr.reset(FFI_USE_FTABLE getClassFromType(t));
        return result;
    }

    static Class from_name(const Image& img, const char *ns, const char *name) {
        Class result;
        result.ptr.reset(FFI_USE_FTABLE getClassFromName(img.ptr.get(), ns, name));
        return result;
    }

    String name() const { return String(FFI_USE_FTABLE classGetName(ptr.get())); }

    String namespaze() const { return String(FFI_USE_FTABLE classGetNamespace(ptr.get())); }

    Image image() const {
        Image result;
        result.ptr.reset(FFI_USE_FTABLE classGetImage(ptr.get()));
        return result;
    }

    Class parent() const {
        Class result;
        result.ptr.reset(FFI_USE_FTABLE classGetParent(ptr.get()));
        return result;
    }

    Class declaring_type() const {
        Class result;
        result.ptr.reset(FFI_USE_FTABLE classGetDeclaringType(ptr.get()));
        return result;
    }

    bool is_value_type() const { return FFI_USE_FTABLE classIsValueType(ptr.get()); }
    bool is_enum() const { return FFI_USE_FTABLE classIsEnum(ptr.get()); }
    bool is_abstract() const { return FFI_USE_FTABLE classIsAbstract(ptr.get()); }
    bool is_interface() const { return FFI_USE_FTABLE classIsInterface(ptr.get()); }
    bool is_generic() const { return FFI_USE_FTABLE classIsGeneric(ptr.get()); }
    bool is_inflated() const { return FFI_USE_FTABLE classIsInflated(ptr.get()); }
    bool has_references() const { return FFI_USE_FTABLE classHasReferences(ptr.get()); }

    int rank() const { return FFI_USE_FTABLE classGetRank(ptr.get()); }
    int32_t instance_size() const { return FFI_USE_FTABLE classInstanceSize(ptr.get()); }
    int array_element_size() const { return FFI_USE_FTABLE classArrayElementSize(ptr.get()); }
    int flags() const { return FFI_USE_FTABLE classGetFlags(ptr.get()); }
    uint32_t type_token() const { return FFI_USE_FTABLE classGetTypeToken(ptr.get()); }
    uint32_t data_size() const { return FFI_USE_FTABLE classGetDataSize(ptr.get()); }

    Type type() const {
        Type result;
        result.ptr.reset(FFI_USE_FTABLE classGetType(ptr.get()));
        return result;
    }

    Class element_class() const {
        Class result;
        result.ptr.reset(FFI_USE_FTABLE classGetElementClass(ptr.get()));
        return result;
    }

    Field get_field(const char *name) const;
    Method get_method(const char *name, int args) const;
    Property get_property(const char *name) const;

    tl::vector<Field> fields() const;
    tl::vector<Method> methods() const;
    tl::vector<Class> nested_types() const;
    tl::vector<Property> properties() const;

    bool has_attribute(Class attr_class) const { return FFI_USE_FTABLE classHasAttribute(ptr.get(), attr_class.ptr.get()); }

    bool is_assignable_from(Class other) const { return FFI_USE_FTABLE classIsAssignableFrom(ptr.get(), other.ptr.get()); }

    bool is_subclass_of(Class klassc, bool check_interfaces) const {
        return FFI_USE_FTABLE classIsSubclassOf(ptr.get(), klassc.ptr.get(), check_interfaces ? 1 : 0);
    }

    bool has_parent(Class klassc) const { return FFI_USE_FTABLE classHasParent(ptr.get(), klassc.ptr.get()); }
};

struct Method {
    UniqueHandle<MethodInfoHandle> ptr;

    explicit operator bool() const { return ptr && ptr.get() != MethodInfoHandle{}; }

    String name() const { return String(FFI_USE_FTABLE methodInfoGetName(ptr.get())); }

    Class declaring_type() const {
        Class result;
        result.ptr.reset(FFI_USE_FTABLE methodInfoGetDeclaringType(ptr.get()));
        return result;
    }

    Class klass() const {
        Class result;
        result.ptr.reset(FFI_USE_FTABLE methodInfoGetClass(ptr.get()));
        return result;
    }

    bool is_generic() const { return FFI_USE_FTABLE methodInfoIsGeneric(ptr.get()); }
    bool is_inflated() const { return FFI_USE_FTABLE methodInfoIsInflated(ptr.get()); }
    bool is_instance() const { return FFI_USE_FTABLE methodInfoIsInstance(ptr.get()); }

    uint32_t param_count() const { return FFI_USE_FTABLE methodInfoGetParamCount(ptr.get()); }

    Type param(uint32_t index) const {
        Type result;
        result.ptr.reset(FFI_USE_FTABLE methodInfoGetParamType(ptr.get(), index));
        return result;
    }

    String param_name(uint32_t index) const { return String(FFI_USE_FTABLE methodInfoGetParamName(ptr.get(), index)); }

    Type return_type() const {
        Type result;
        result.ptr.reset(FFI_USE_FTABLE methodInfoGetReturnType(ptr.get()));
        return result;
    }

    uint32_t flags(uint32_t *iflags = nullptr) const {
        uint32_t mainFlags = FFI_USE_FTABLE methodInfoGetFlags(ptr.get());
        if (iflags) {
            *iflags = FFI_USE_FTABLE methodInfoGetImplFlags(ptr.get());
        }
        return mainFlags;
    }

    uint32_t token() const { return FFI_USE_FTABLE methodInfoGetToken(ptr.get()); }

    bool has_attribute(Class attr_class) const { return FFI_USE_FTABLE methodInfoHasAttribute(ptr.get(), attr_class.ptr.get()); }
};

struct Field {
    UniqueHandle<FieldHandle> ptr;

    explicit operator bool() const { return ptr && ptr.get() != FieldHandle{}; }

    String name() const { return String(FFI_USE_FTABLE fieldGetName(ptr.get())); }

    uint32_t flags() const { return FFI_USE_FTABLE fieldGetFlags(ptr.get()); }

    Class parent() const {
        Class result;
        result.ptr.reset(FFI_USE_FTABLE fieldGetParent(ptr.get()));
        return result;
    }

    size_t offset() const { return FFI_USE_FTABLE fieldGetOffset(ptr.get()); }

    Type type() const {
        Type result;
        result.ptr.reset(FFI_USE_FTABLE fieldGetType(ptr.get()));
        return result;
    }

    bool has_attribute(Class attr_class) const { return FFI_USE_FTABLE fieldHasAttribute(ptr.get(), attr_class.ptr.get()); }

    bool is_literal() const { return FFI_USE_FTABLE fieldIsLiteral(ptr.get()); }

    template <class T> T get_value(ObjectHandle obj) const;
    ObjectHandle get_value_object(ObjectHandle obj) const { return FFI_USE_FTABLE fieldGetValueObject(obj, ptr.get()); }

    template <class T> void set_value(ObjectHandle obj, const T& v) const;

    template <class T> T static_get_value() const;
    template <class T> void static_set_value(const T& v) const;

    void set_value_object(ObjectHandle obj, ObjectHandle value) const { FFI_USE_FTABLE fieldSetValueObject(obj, ptr.get(), value); }
};

struct Property {
    UniqueHandle<PropertyHandle> ptr;

    explicit operator bool() const { return ptr && ptr.get() != PropertyHandle{}; }

    String name() const { return String(FFI_USE_FTABLE propertyGetName(ptr.get())); }

    Class parent() const {
        Class result;
        result.ptr.reset(FFI_USE_FTABLE propertyGetParent(ptr.get()));
        return result;
    }

    uint32_t flags() const { return FFI_USE_FTABLE propertyGetFlags(ptr.get()); }

    Method getter() const {
        Method result;
        result.ptr.reset(FFI_USE_FTABLE propertyGetGetMethod(ptr.get()));
        return result;
    }

    Method setter() const {
        Method result;
        result.ptr.reset(FFI_USE_FTABLE propertyGetSetMethod(ptr.get()));
        return result;
    }
};

struct Array {
    UniqueHandle<ObjectHandle> ptr;

    explicit operator bool() const { return ptr && ptr.get() != ObjectHandle{} && FFI isValidObject(ptr.get()); }

    static Array New(ClassHandle element, size_t length) {
        Array result;
        result.ptr.reset(FFI_USE_FTABLE createArray(element, length));
        return result;
    }

    static Array NewSpecific(ClassHandle arrayType, size_t length) {
        Array result;
        result.ptr.reset(FFI_USE_FTABLE createArraySpecific(arrayType, length));
        return result;
    }

    uint32_t length() const { return FFI_USE_FTABLE arrayGetLength(ptr.get()); }

    uint32_t byte_length() const { return FFI_USE_FTABLE arrayGetByteLength(ptr.get()); }
};

// Field value access template specializations
template <> inline int32_t Field::get_value<int32_t>(ObjectHandle obj) const { return FFI_USE_FTABLE fieldGetValueI32(obj, ptr.get()); }
template <> inline int64_t Field::get_value<int64_t>(ObjectHandle obj) const { return FFI_USE_FTABLE fieldGetValueI64(obj, ptr.get()); }
template <> inline float Field::get_value<float>(ObjectHandle obj) const { return FFI_USE_FTABLE fieldGetValueFloat(obj, ptr.get()); }
template <> inline double Field::get_value<double>(ObjectHandle obj) const { return FFI_USE_FTABLE fieldGetValueDouble(obj, ptr.get()); }

template <> inline void Field::set_value<int32_t>(ObjectHandle obj, const int32_t& v) const { FFI_USE_FTABLE fieldSetValueI32(obj, ptr.get(), v); }
template <> inline void Field::set_value<int64_t>(ObjectHandle obj, const int64_t& v) const { FFI_USE_FTABLE fieldSetValueI64(obj, ptr.get(), v); }
template <> inline void Field::set_value<float>(ObjectHandle obj, const float& v) const { FFI_USE_FTABLE fieldSetValueFloat(obj, ptr.get(), v); }
template <> inline void Field::set_value<double>(ObjectHandle obj, const double& v) const { FFI_USE_FTABLE fieldSetValueDouble(obj, ptr.get(), v); }
template <> inline void Field::set_value<ObjectHandle>(ObjectHandle obj, const ObjectHandle& v) const { FFI_USE_FTABLE fieldSetValueObject(obj, ptr.get(), v); }

// Static field access template specializations
template <> inline int32_t Field::static_get_value<int32_t>() const { return FFI_USE_FTABLE fieldStaticGetValueI32(ptr.get()); }
template <> inline int64_t Field::static_get_value<int64_t>() const { return FFI_USE_FTABLE fieldStaticGetValueI64(ptr.get()); }
template <> inline float Field::static_get_value<float>() const { return FFI_USE_FTABLE fieldStaticGetValueFloat(ptr.get()); }
template <> inline double Field::static_get_value<double>() const { return FFI_USE_FTABLE fieldStaticGetValueDouble(ptr.get()); }

template <> inline void Field::static_set_value<int32_t>(const int32_t& v) const { FFI_USE_FTABLE fieldStaticSetValueI32(ptr.get(), v); }
template <> inline void Field::static_set_value<int64_t>(const int64_t& v) const { FFI_USE_FTABLE fieldStaticSetValueI64(ptr.get(), v); }
template <> inline void Field::static_set_value<float>(const float& v) const { FFI_USE_FTABLE fieldStaticSetValueFloat(ptr.get(), v); }
template <> inline void Field::static_set_value<double>(const double& v) const { FFI_USE_FTABLE fieldStaticSetValueDouble(ptr.get(), v); }

// Implementations dependent on other wrappers
inline Class Object::klass() const {
    Class result;
    result.ptr.reset(FFI_USE_FTABLE objectGetClass(ptr.get()));
    return result;
}

inline Method Object::virtual_method(const Method& m) const {
    Method result;
    result.ptr.reset(FFI_USE_FTABLE objectGetVirtualMethod(ptr.get(), m.ptr.get()));
    return result;
}

inline Class Type::class_or_element() const {
    Class result;
    result.ptr.reset(FFI_USE_FTABLE typeGetClassOrElementClass(ptr.get()));
    return result;
}

inline Assembly Domain::open_assembly(const char *name) {
    Assembly result;
    result.ptr.reset(FFI_USE_FTABLE domainAssemblyOpen(ptr.get(), name));
    return result;
}

inline tl::vector<Assembly> Domain::get_assemblies() const {
    tl::vector<Assembly> result;
    int32_t count = FFI_USE_FTABLE domainGetAssemblyCount(ptr.get());
    result.reserve(count);

    for (int32_t i = 0; i < count; ++i) {
        Assembly assembly;
        assembly.ptr.reset(FFI_USE_FTABLE domainGetAssemblyAt(ptr.get(), i));
        if (assembly)
            result.push_back(tl::move(assembly));
    }

    return result;
}

inline Image Assembly::image() const {
    Image result;
    result.ptr.reset(FFI_USE_FTABLE assemblyGetImage(ptr.get()));
    return result;
}

inline Class Image::get_class(size_t index) const {
    Class result;
    result.ptr.reset(FFI_USE_FTABLE imageGetClassAt(ptr.get(), index));
    return result;
}

inline Field Class::get_field(const char *n) const {
    Field result;
    result.ptr.reset(FFI_USE_FTABLE classGetFieldFromName(ptr.get(), n));
    return result;
}

inline Method Class::get_method(const char *n, int args) const {
    Method result;
    result.ptr.reset(FFI_USE_FTABLE classGetMethodFromName(ptr.get(), n, args));
    return result;
}

inline Property Class::get_property(const char *n) const {
    Property result;
    result.ptr.reset(FFI_USE_FTABLE classGetPropertyFromName(ptr.get(), n));
    return result;
}

inline tl::vector<Field> Class::fields() const {
    tl::vector<Field> result;
    int32_t count = FFI_USE_FTABLE classNumFields(ptr.get());
    result.reserve(count);

    for (int32_t i = 0; i < count; ++i) {
        Field field;
        field.ptr.reset(FFI_USE_FTABLE classGetFieldAt(ptr.get(), i));
        if (field) {
            result.push_back(tl::move(field));
        }
    }

    return result;
}

inline tl::vector<Method> Class::methods() const {
    tl::vector<Method> result;
    for (int32_t i = 0;; ++i) {
        MethodInfoHandle handle = FFI_USE_FTABLE classGetMethodAt(ptr.get(), i);
        if (handle == MethodInfoHandle::Null)
            break;

        Method method;
        method.ptr.reset(handle);
        result.push_back(tl::move(method));
    }

    return result;
}

inline tl::vector<Class> Class::nested_types() const {
    tl::vector<Class> result;
    // FFI doesn't provide a direct nested type count, so we need to iterate
    for (int32_t i = 0;; ++i) {
        ClassHandle handle = FFI_USE_FTABLE classGetNestedTypeAt(ptr.get(), i);
        if (handle == ClassHandle{})
            break;

        Class nested;
        nested.ptr.reset(handle);
        result.push_back(tl::move(nested));
    }

    return result;
}

inline tl::vector<Property> Class::properties() const {
    tl::vector<Property> result;
    // FFI doesn't provide a direct property count, so we need to iterate
    for (int32_t i = 0;; ++i) {
        PropertyHandle handle = FFI_USE_FTABLE classGetPropertyAt(ptr.get(), i);
        if (handle == PropertyHandle{})
            break;

        Property property;
        property.ptr.reset(handle);
        result.push_back(tl::move(property));
    }

    return result;
}
#endif

namespace Literals {
inline FFIInterface::ObjectHandle operator""_cs(const char *str, size_t len) { return FFI_USE_FTABLE toCsStringWithLength(str, len); }
} // namespace Literals
} // namespace
} // namespace Helpers
#endif
} // namespace FFIInterface
