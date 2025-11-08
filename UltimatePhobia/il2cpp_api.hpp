#pragma once

// https://github.com/MlgmXyysd/libil2cpp/tree/master/libil2cpp/Unity_2019.4/2019.4.31f1
// https://github.com/niansa/libil2cpp/tree/master/libil2cpp/Unity_2019.4/2019.4.31f1

#include <cstdint>
#include <cstddef>

namespace Il2Cpp {
namespace API {
typedef struct Il2CppClass Il2CppClass;
typedef struct Il2CppType Il2CppType;
typedef struct EventInfo EventInfo;
typedef struct MethodInfo {
    void *methodPointer;
} MethodInfo;
typedef struct FieldInfo FieldInfo;
typedef struct PropertyInfo PropertyInfo;

typedef struct Il2CppAssembly Il2CppAssembly;
typedef struct Il2CppArray Il2CppArray;
typedef struct Il2CppDelegate Il2CppDelegate;
typedef struct Il2CppDomain Il2CppDomain;
typedef struct Il2CppImage Il2CppImage;
typedef struct Il2CppException Il2CppException;
typedef struct Il2CppProfiler Il2CppProfiler;
typedef struct Il2CppObject Il2CppObject;
typedef struct Il2CppReflectionMethod Il2CppReflectionMethod;
typedef struct Il2CppReflectionType Il2CppReflectionType;
typedef struct Il2CppString Il2CppString;
typedef struct Il2CppThread Il2CppThread;
typedef struct Il2CppAsyncResult Il2CppAsyncResult;
typedef struct Il2CppManagedMemorySnapshot Il2CppManagedMemorySnapshot;
typedef struct Il2CppCustomAttrInfo Il2CppCustomAttrInfo;

typedef enum {
    IL2CPP_PROFILE_NONE = 0,
    IL2CPP_PROFILE_APPDOMAIN_EVENTS = 1 << 0,
    IL2CPP_PROFILE_ASSEMBLY_EVENTS = 1 << 1,
    IL2CPP_PROFILE_MODULE_EVENTS = 1 << 2,
    IL2CPP_PROFILE_CLASS_EVENTS = 1 << 3,
    IL2CPP_PROFILE_JIT_COMPILATION = 1 << 4,
    IL2CPP_PROFILE_INLINING = 1 << 5,
    IL2CPP_PROFILE_EXCEPTIONS = 1 << 6,
    IL2CPP_PROFILE_ALLOCATIONS = 1 << 7,
    IL2CPP_PROFILE_GC = 1 << 8,
    IL2CPP_PROFILE_THREADS = 1 << 9,
    IL2CPP_PROFILE_REMOTING = 1 << 10,
    IL2CPP_PROFILE_TRANSITIONS = 1 << 11,
    IL2CPP_PROFILE_ENTER_LEAVE = 1 << 12,
    IL2CPP_PROFILE_COVERAGE = 1 << 13,
    IL2CPP_PROFILE_INS_COVERAGE = 1 << 14,
    IL2CPP_PROFILE_STATISTICAL = 1 << 15,
    IL2CPP_PROFILE_METHOD_EVENTS = 1 << 16,
    IL2CPP_PROFILE_MONITOR_EVENTS = 1 << 17,
    IL2CPP_PROFILE_IOMAP_EVENTS = 1 << 18,
    IL2CPP_PROFILE_GC_MOVES = 1 << 19,
    IL2CPP_PROFILE_FILEIO = 1 << 20
} Il2CppProfileFlags;

typedef enum { IL2CPP_PROFILE_FILEIO_WRITE = 0, IL2CPP_PROFILE_FILEIO_READ } Il2CppProfileFileIOKind;

typedef enum {
    IL2CPP_GC_EVENT_START,
    IL2CPP_GC_EVENT_MARK_START,
    IL2CPP_GC_EVENT_MARK_END,
    IL2CPP_GC_EVENT_RECLAIM_START,
    IL2CPP_GC_EVENT_RECLAIM_END,
    IL2CPP_GC_EVENT_END,
    IL2CPP_GC_EVENT_PRE_STOP_WORLD,
    IL2CPP_GC_EVENT_POST_STOP_WORLD,
    IL2CPP_GC_EVENT_PRE_START_WORLD,
    IL2CPP_GC_EVENT_POST_START_WORLD
} Il2CppGCEvent;

typedef enum {
    IL2CPP_STAT_NEW_OBJECT_COUNT,
    IL2CPP_STAT_INITIALIZED_CLASS_COUNT,
    // IL2CPP_STAT_GENERIC_VTABLE_COUNT,
    // IL2CPP_STAT_USED_CLASS_COUNT,
    IL2CPP_STAT_METHOD_COUNT,
    // IL2CPP_STAT_CLASS_VTABLE_SIZE,
    IL2CPP_STAT_CLASS_STATIC_DATA_SIZE,
    IL2CPP_STAT_GENERIC_INSTANCE_COUNT,
    IL2CPP_STAT_GENERIC_CLASS_COUNT,
    IL2CPP_STAT_INFLATED_METHOD_COUNT,
    IL2CPP_STAT_INFLATED_TYPE_COUNT,
    // IL2CPP_STAT_DELEGATE_CREATIONS,
    // IL2CPP_STAT_MINOR_GC_COUNT,
    // IL2CPP_STAT_MAJOR_GC_COUNT,
    // IL2CPP_STAT_MINOR_GC_TIME_USECS,
    // IL2CPP_STAT_MAJOR_GC_TIME_USECS
} Il2CppStat;

typedef enum { IL2CPP_UNHANDLED_POLICY_LEGACY, IL2CPP_UNHANDLED_POLICY_CURRENT } Il2CppRuntimeUnhandledExceptionPolicy;

typedef enum Il2CppTypeEnum {
    IL2CPP_TYPE_END = 0x00, /* End of List */
    IL2CPP_TYPE_VOID = 0x01,
    IL2CPP_TYPE_BOOLEAN = 0x02,
    IL2CPP_TYPE_CHAR = 0x03,
    IL2CPP_TYPE_I1 = 0x04,
    IL2CPP_TYPE_U1 = 0x05,
    IL2CPP_TYPE_I2 = 0x06,
    IL2CPP_TYPE_U2 = 0x07,
    IL2CPP_TYPE_I4 = 0x08,
    IL2CPP_TYPE_U4 = 0x09,
    IL2CPP_TYPE_I8 = 0x0a,
    IL2CPP_TYPE_U8 = 0x0b,
    IL2CPP_TYPE_R4 = 0x0c,
    IL2CPP_TYPE_R8 = 0x0d,
    IL2CPP_TYPE_STRING = 0x0e,
    IL2CPP_TYPE_PTR = 0x0f,         /* arg: <type> token */
    IL2CPP_TYPE_BYREF = 0x10,       /* arg: <type> token */
    IL2CPP_TYPE_VALUETYPE = 0x11,   /* arg: <type> token */
    IL2CPP_TYPE_CLASS = 0x12,       /* arg: <type> token */
    IL2CPP_TYPE_VAR = 0x13,         /* Generic parameter in a generic type definition, represented as number (compressed unsigned integer) number */
    IL2CPP_TYPE_ARRAY = 0x14,       /* type, rank, boundsCount, bound1, loCount, lo1 */
    IL2CPP_TYPE_GENERICINST = 0x15, /* <type> <type-arg-count> <type-1> \x{2026} <type-n> */
    IL2CPP_TYPE_TYPEDBYREF = 0x16,
    IL2CPP_TYPE_I = 0x18,
    IL2CPP_TYPE_U = 0x19,
    IL2CPP_TYPE_FNPTR = 0x1b, /* arg: full method signature */
    IL2CPP_TYPE_OBJECT = 0x1c,
    IL2CPP_TYPE_SZARRAY = 0x1d,   /* 0-based one-dim-array */
    IL2CPP_TYPE_MVAR = 0x1e,      /* Generic parameter in a generic method definition, represented as number (compressed unsigned integer)  */
    IL2CPP_TYPE_CMOD_REQD = 0x1f, /* arg: typedef or typeref token */
    IL2CPP_TYPE_CMOD_OPT = 0x20,  /* optional arg: typedef or typref token */
    IL2CPP_TYPE_INTERNAL = 0x21,  /* CLR internal type */

    IL2CPP_TYPE_MODIFIER = 0x40, /* Or with the following types */
    IL2CPP_TYPE_SENTINEL = 0x41, /* Sentinel for varargs method signature */
    IL2CPP_TYPE_PINNED = 0x45,   /* Local var that points to pinned object */

    IL2CPP_TYPE_ENUM = 0x55 /* an enumeration */
} Il2CppTypeEnum;

typedef struct Il2CppStackFrameInfo {
    const MethodInfo *method;
} Il2CppStackFrameInfo;

typedef void (*Il2CppMethodPointer)();

typedef struct Il2CppMethodDebugInfo {
    Il2CppMethodPointer methodPointer;
    int32_t code_size;
    const char *file;
} Il2CppMethodDebugInfo;

typedef struct {
    void *(*malloc_func)(size_t size);
    void *(*aligned_malloc_func)(size_t size, size_t alignment);
    void (*free_func)(void *ptr);
    void (*aligned_free_func)(void *ptr);
    void *(*calloc_func)(size_t nmemb, size_t size);
    void *(*realloc_func)(void *ptr, size_t size);
    void *(*aligned_realloc_func)(void *ptr, size_t size, size_t alignment);
} Il2CppMemoryCallbacks;

typedef struct {
    const char *name;
    void (*connect)(const char *address);
    int (*wait_for_attach)(void);
    void (*close1)(void);
    void (*close2)(void);
    int (*send)(void *buf, int len);
    int (*recv)(void *buf, int len);
} Il2CppDebuggerTransport;

#if !__SNC__               // SNC doesn't like the following define: "warning 1576: predefined
                           // meaning of __has_feature discarded"
#ifndef __has_feature      // clang specific __has_feature check
#define __has_feature(x) 0 // Compatibility with non-clang compilers.
#endif
#endif

#if _MSC_VER
typedef wchar_t Il2CppChar;
#elif __has_feature(cxx_unicode_literals)
typedef char16_t Il2CppChar;
#else
typedef uint16_t Il2CppChar;
#endif

#if _MSC_VER
typedef wchar_t Il2CppNativeChar;
#define IL2CPP_NATIVE_STRING(str) L##str
#else
typedef char Il2CppNativeChar;
#define IL2CPP_NATIVE_STRING(str) str
#endif

typedef void (*il2cpp_register_object_callback)(Il2CppObject **arr, int size, void *userdata);
typedef void (*il2cpp_WorldChangedCallback)();
typedef void (*Il2CppFrameWalkFunc)(const Il2CppStackFrameInfo *info, void *user_data);
typedef void (*Il2CppProfileFunc)(Il2CppProfiler *prof);
typedef void (*Il2CppProfileMethodFunc)(Il2CppProfiler *prof, const MethodInfo *method);
typedef void (*Il2CppProfileAllocFunc)(Il2CppProfiler *prof, Il2CppObject *obj, Il2CppClass *klass);
typedef void (*Il2CppProfileGCFunc)(Il2CppProfiler *prof, Il2CppGCEvent event, int generation);
typedef void (*Il2CppProfileGCResizeFunc)(Il2CppProfiler *prof, int64_t new_size);
typedef void (*Il2CppProfileFileIOFunc)(Il2CppProfiler *prof, Il2CppProfileFileIOKind kind, int count);
typedef void (*Il2CppProfileThreadFunc)(Il2CppProfiler *prof, unsigned long tid);

typedef const Il2CppNativeChar *(*Il2CppSetFindPlugInCallback)(const Il2CppNativeChar *);
typedef void (*Il2CppLogCallback)(const char *);

typedef size_t (*Il2CppBacktraceFunc)(Il2CppMethodPointer *buffer, size_t maxSize);

struct Il2CppManagedMemorySnapshot;

typedef uintptr_t il2cpp_array_size_t;

void init();

#define DO_API(r, n, p) extern r(*n) p;
#include "il2cpp_api.inc"
#undef DO_API
} // namespace API
} // namespace Il2Cpp
