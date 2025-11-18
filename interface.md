# FFI Argument Passing and Hook Handling

## **Warning: This document is somewhat outdated**

A completely new API now exists besides the old legacy API documented here. It now supports reflection and automatic overload resolution, see new handle types and `call3` in `ffi_interface.hpp`.

## Argument Passing to C# Functions

The FFI interface provides a stack-based mechanism for passing arguments to C# functions:

1. **Building the Argument List**:
   - Use `addArg*` functions to push arguments onto the stack (`addArgI32`, `addArgI64`, `addArgFloat`, etc.)
   - Arguments must be added in the order expected by the C# method
   - The argument list can be cleared with `clearArgs()`

2. **Calling the Function**:
   - After building arguments, call the method using `call()` or `call2()`
   - The call automatically consumes the arguments (clears the stack)

3. **Return Values**:
   - After a call, use `getValue*` functions to retrieve the return value
   - Pass -1 as index to access the return value

4. **Struct Returns**:
   - For methods returning structs, use `call2()` with `returnsStruct=true`
   - The struct will be copied into a byte array that you must read

## Hook Handling

When hooking C# methods:

1. **Setting Up Hooks**:
   - Use `hook()`  (or `GameHook` RAII class, see below) to redirect a method to your callback
   - The callback name must match an exported function in your module

2. **Inside Hook Callbacks**:
   - The argument stack will contain the hooked method's arguments
   - Use `getOriginal()` to get the original method handle
   - You can call the original method by building arguments and using `call()`

3. **Modifying Arguments/Return**:
   - Access arguments with `getValue*(index)` where index is 0-based
   - Modify return value by using `addArg*()` then `moveArg(-1)`
   - You can also modify arguments before calling the original method

4. **Hook Management**:
   - Use `unhook()` to remove a hook (but not from within its own callback) (optional if using `GameHook` class)
   - The `GameHook` helper class provides RAII-style hook management

---

# FFI Interface Documentation (`ffi_interface.hpp`)

## Overview

The FFI interface provides interoperability between native code and C# in the UltimatePhobia runtime. It includes:

- Core object/method handle management
- Method calling functionality
- Hook management
- Logging utilities
- Memory/GC management
- ImGui integration

## Core Types

```cpp
enum class ObjectHandle : int32_t { Null = 0, Invalid = -1 };
enum class MethodHandle : int32_t { Invalid = -1 };
enum class GCHandle : int32_t { Invalid = -1 };
using WIBool = int32_t; // Boolean type (0 = false, non-zero = true)
```

## Core Functions

### Object Management

```cpp
// Release a C# object handle
void dropObject(ObjectHandle);

// Check if handle is valid
WIBool isValidObject(ObjectHandle);

// Get C# null object
ObjectHandle getNull();

// Get object address
int64_t getObjectAddress(ObjectHandle);
```

### String Conversion

```cpp
// Convert C string to C# string
ObjectHandle toCsString(const char*);
ObjectHandle toCsStringWithLength(const char*, int32_t length);

// Convert C# string to C string
void toCString(ObjectHandle str, char* buf, int32_t maxlen);
```

### Reflection

```cpp
// Get core library image handle
ObjectHandle getImageCorlib();

// Get class by name
ObjectHandle getClassFromName(ObjectHandle image, const char* namespaze, const char* name);

// Create array type
ObjectHandle getArrayFromClass(ObjectHandle elementClass, int32_t rank);

// Create array instance
ObjectHandle createArray(ObjectHandle elementClass, int32_t length);

// Copy array contents
void copyArrayBytes(ObjectHandle array, int32_t offset, int32_t length, void* to);
```

### Garbage Collection

```cpp
// Create GC handle (prevents collection)
GCHandle gcCreateHandle(ObjectHandle object, WIBool pinned);

// Release GC handle
void gcDeleteHandle(GCHandle);
```

### Logging

```cpp
void logTrace(ObjectHandle message);
void logDebug(ObjectHandle message);
void logInfo(ObjectHandle message);
void logWarn(ObjectHandle message);
void logError(ObjectHandle message);
void logCritical(ObjectHandle message);
```

### Method Handling

```cpp
// Get method by identifier or address
MethodHandle getMethodByIdentifier(const char* identifier);
MethodHandle getMethodByAddress(int64_t addr);

// Get method info
ObjectHandle getMethodName(MethodHandle);
ObjectHandle getMethodSignature(MethodHandle);
int64_t getMethodAddresss(MethodHandle);
```

### Method Calling

```cpp
// Argument management
void addArgI32(int32_t);
void addArgI64(int64_t);
void addArgFloat(float);
void addArgDouble(double);
void addArgObject(ObjectHandle);
void addArgNull();
void clearArgs();
int32_t getArgCount();
WIBool moveArg(int32_t index); // Move argument to different position

// Value retrieval
int32_t getValueI32(int32_t index = -1);
int64_t getValueI64(int32_t index = -1);
float getValueFloat(int32_t index = -1);
double getValueDouble(int32_t index = -1);
ObjectHandle getValueObject(int32_t index = -1);

// Call methods
WIBool call(MethodHandle, int32_t argCount);
WIBool call2(MethodHandle, int32_t argCount, WIBool returnsStruct = false);
ObjectHandle getCallError(); // Get error message
```

### Hooking

```cpp
WIBool hook(MethodHandle, const char* callback);
WIBool unhook(MethodHandle);
MethodHandle getOriginal(); // Get original method inside hook
```

### ImGui

```cpp
void ImGuiBegin(const char* name = "");
void ImGuiEnd();
void ImGuiText(ObjectHandle text);
WIBool ImGuiCheckbox2(const char* label, WIBool v);
int32_t ImGuiCheckbox4(const char* label, WIBool v);
WIBool ImGuiButton(const char* label);
void ImGuiSeparator();
void ImGuiSeparatorText(const char* label);
```

## Helper Functions/Classes

The helper functions provide a safer, more convenient interface to the FFI system and should be preferred over direct FFI calls.

### String Literals

```cpp
namespace Literals {
ObjectHandle operator""_cs(const char* str, size_t len); // Create C# string
}
```

### String Conversion

```cpp
// Get C string from C# string (with buffer)
template <unsigned maxlen = 64>
const char* getCString(ObjectHandle str);

// Create empty C# string
ObjectHandle createCsString();
```

### Method Calling Helpers

```cpp
// Get method (cached)
template <StringLiteral identifier> 
MethodHandle getMethodCached();

// Type-safe argument adding
void addArg(int32_t);
void addArg(int64_t);
void addArg(bool);
void addArg(float);
void addArg(double);
void addArg(ObjectHandle);
void addArg(decltype(nullptr));

// Typed value getting
template <typename T> T getArg(int idx);
template <typename T> T getReturnValue();
template <typename T> void setReturnValue(T v);

// Variadic argument adding
void addArgs(); // Base case
template <typename Arg0, typename... Args> 
void addArgs(Arg0 arg0, Args... args);

// Convenient method calling
template <StringLiteral identifier, typename returnT = void, typename... Args>
returnT call(Args... args);

// Struct return calling
template <StringLiteral identifier, typename returnT, typename... Args>
returnT callRetStruct(Args... args);
```

### Array Helpers

```cpp
// Get byte array type
ObjectHandle getDataArrayType();

// Create byte array
ObjectHandle getDataArray(unsigned length);
```

### GameHook Class

RAII-style hook management:

```cpp
class GameHook {
public:
    GameHook(MethodHandle method, const char* callback);
    ~GameHook();
    
    // Move-only
    GameHook(GameHook&&) noexcept;
    GameHook& operator=(GameHook&&) noexcept;
    
    bool release(); // Unhook
    bool restore(); // Re-hook
    
    MethodHandle target() const;
    bool isActive() const;
};
```

### GameHookPool Class (STL-only)

```cpp
class GameHookPool {
public:
    std::shared_ptr<GameHook> add(MethodHandle, const char*);
    template <StringLiteral identifier> 
    std::shared_ptr<GameHook> add(const char*);
    
    std::shared_ptr<GameHook> get(MethodHandle) const;
    void remove(MethodHandle);
    void clear();
};
```

### UI Helpers

```cpp
// Safe checkbox handling
bool ImGuiCheckbox(const char* label, bool* state);

// Hook toggle helpers
bool hookToggle(const char* desc, std::optional<GameHook>&, bool&, 
               MethodHandle, const char*);
bool hookToggle(const char* desc, GameHookPool&, bool&, 
               MethodHandle, const char*);
```

## Best Practices

1. **Always use helper functions** when available - they provide type safety and cleaner code
2. **Use RAII** (e.g. `GameHook` class) for hook management
3. **Prefer cached methods** (e.g. `getMethodCached`) for performance
4. **Check runtime version** with `getFtableItemCount()` vs `getLocalFtableItemCount()`
5. **Handle errors** - check return values and use `getCallError()` when needed
6. **Clean up objects** - use `dropObject()` for temporary C# objects when done
7. **Use string literals** (`"text"_cs`) for cleaner string handling
