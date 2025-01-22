export type ObjectHandle = i32;
export type MethodHandle = i32;
export type WIBool = i32;
export const unknownArgCount: i32 = 0x6D616E63;

export declare function dropObject(handle: ObjectHandle): void;
export declare function isValidObject(handle: ObjectHandle): WIBool;
export declare function getNull(): ObjectHandle;

export declare function toCsString(str: ArrayBuffer): ObjectHandle;
export declare function toCsStringWithLength(str: ArrayBuffer, length: i32): ObjectHandle;
export declare function toCString(str: ObjectHandle, buf: ArrayBuffer, maxlen: i32): void;

export declare function logTrace(message: ObjectHandle): void;
export declare function logDebug(message: ObjectHandle): void;
export declare function logInfo(message: ObjectHandle): void;
export declare function logWarn(message: ObjectHandle): void;
export declare function logError(message: ObjectHandle): void;
export declare function logCritical(message: ObjectHandle): void;

export declare function getMethodByIdentifier(identifier: ArrayBuffer): MethodHandle;
export declare function getMethodByAddress(address: i64): MethodHandle;
export declare function getMethodName(handle: MethodHandle): ObjectHandle;
export declare function getMethodSignature(handle: MethodHandle): ObjectHandle;

export declare function addArgI32(v: i32): void;
export declare function addArgI64(v: i64): void;
export declare function addArgFloat(v: f32): void;
export declare function addArgDouble(v: f64): void;
export declare function addArgObject(v: ObjectHandle): void;
export declare function addArgNull(): void;
export declare function clearArgs(): void;
export declare function getArgCount(): i32;
export declare function moveArg(index: i32): WIBool;
export declare function getValueI32(index: i32): i32;
export declare function getValueI64(index: i32): i64;
export declare function getValueFloat(index: i32): f32;
export declare function getValueDouble(index: i32): f64;
export declare function getValueObject(index: i32): ObjectHandle;
export declare function getCallError(): ObjectHandle;
export declare function call(method: MethodHandle, argCount: i32): WIBool;
export declare function hook(method: MethodHandle, callback: ArrayBuffer): WIBool;
export declare function unhook(method: MethodHandle): WIBool;
export declare function getOriginal(): MethodHandle;

export declare function ImGuiBegin(name: ArrayBuffer): void;
export declare function ImGuiEnd(): void;
export declare function ImGuiText(text: ObjectHandle): void;
//export declare function ImGuiCheckbox(label: ArrayBuffer, v: ref_any): void;
export declare function ImGuiButton(label: ArrayBuffer): WIBool;
export declare function ImGuiSeparator(): void;
export declare function ImGuiSeparatorText(label: ArrayBuffer): void;


export function asToCString(str: string): ArrayBuffer {
  return String.UTF8.encode(str, true);
}

export function asToCsString(str: string): ObjectHandle {
  return toCsString(asToCString(str));
}
