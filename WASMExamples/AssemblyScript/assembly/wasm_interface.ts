export type ObjectHandle = i32;
export type MethodHandle = i32;
export type WIBool = i32;
export const unknownArgCount: i32 = 0x6D616E63;

@external("env", "dropObject")
export declare function dropObject(handle: ObjectHandle): void;
@external("env", "isValidObject")
export declare function isValidObject(handle: ObjectHandle): WIBool;
@external("env", "getNull")
export declare function getNull(): ObjectHandle;

@external("env", "toCsString")
export declare function toCsString(str: ArrayBuffer): ObjectHandle;
@external("env", "toCsStringWithLength")
export declare function toCsStringWithLength(str: ArrayBuffer, length: i32): ObjectHandle;
@external("env", "toCString")
export declare function toCString(str: ObjectHandle, buf: ArrayBuffer, maxlen: i32): void;

@external("env", "logTrace")
export declare function logTrace(message: ObjectHandle): void;
@external("env", "logDebug")
export declare function logDebug(message: ObjectHandle): void;
@external("env", "logInfo")
export declare function logInfo(message: ObjectHandle): void;
@external("env", "logWarn")
export declare function logWarn(message: ObjectHandle): void;
@external("env", "logError")
export declare function logError(message: ObjectHandle): void;
@external("env", "logCritical")
export declare function logCritical(message: ObjectHandle): void;

@external("env", "getMethodByIdentifier")
export declare function getMethodByIdentifier(identifier: ArrayBuffer): MethodHandle;
@external("env", "getMethodByAddress")
export declare function getMethodByAddress(address: i64): MethodHandle;
@external("env", "getMethodName")
export declare function getMethodName(handle: MethodHandle): ObjectHandle;
@external("env", "getMethodSignature")
export declare function getMethodSignature(handle: MethodHandle): ObjectHandle;

@external("env", "addArgI32")
export declare function addArgI32(v: i32): void;
@external("env", "addArgI64")
export declare function addArgI64(v: i64): void;
@external("env", "addArgFloat")
export declare function addArgFloat(v: f32): void;
@external("env", "addArgDouble")
export declare function addArgDouble(v: f64): void;
@external("env", "addArgObject")
export declare function addArgObject(v: ObjectHandle): void;
@external("env", "addArgNull")
export declare function addArgNull(): void;
@external("env", "clearArgs")
export declare function clearArgs(): void;
@external("env", "getArgCount")
export declare function getArgCount(): i32;
@external("env", "moveArg")
export declare function moveArg(index: i32): WIBool;
@external("env", "getValueI32")
export declare function getValueI32(index: i32): i32;
@external("env", "getValueI64")
export declare function getValueI64(index: i32): i64;
@external("env", "getValueFloat")
export declare function getValueFloat(index: i32): f32;
@external("env", "getValueDouble")
export declare function getValueDouble(index: i32): f64;
@external("env", "getValueObject")
export declare function getValueObject(index: i32): ObjectHandle;
@external("env", "getCallError")
export declare function getCallError(): ObjectHandle;
@external("env", "call")
export declare function call(method: MethodHandle, argCount: i32): WIBool;
@external("env", "hook")
export declare function hook(method: MethodHandle, callback: ArrayBuffer): WIBool;
@external("env", "unhook")
export declare function unhook(method: MethodHandle): WIBool;
@external("env", "getOriginal")
export declare function getOriginal(): MethodHandle;

@external("env", "ImGuiBegin")
export declare function ImGuiBegin(name: ArrayBuffer): void;
@external("env", "ImGuiEnd")
export declare function ImGuiEnd(): void;
@external("env", "ImGuiText")
export declare function ImGuiText(text: ObjectHandle): void;
//@external("env", "ImGuiCheckbox")
//export declare function ImGuiCheckbox(label: ArrayBuffer, v: ref_any): void;
@external("env", "ImGuiButton")
export declare function ImGuiButton(label: ArrayBuffer): WIBool;
@external("env", "ImGuiSeparator")
export declare function ImGuiSeparator(): void;
@external("env", "ImGuiSeparatorText")
export declare function ImGuiSeparatorText(label: ArrayBuffer): void;


export function asToCString(str: string): ArrayBuffer {
  return String.UTF8.encode(str, true);
}

export function asToCsString(str: string): ObjectHandle {
  return toCsString(asToCString(str));
}
