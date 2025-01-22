import * as up from "./wasm_interface";

var doors: Array<up.ObjectHandle> = [];


export function onLoad(): void {
    up.hook(up.getMethodByIdentifier(up.asToCString("Door$$Start")), up.asToCString("onDoorStart"))
}

export function onUnload(): void {
    for (let i = 0; i < doors.length; ++i) {
        up.dropObject(doors[i]);
    }
}

export function onUiUpdate(): void {
    up.ImGuiBegin(up.asToCString("Door manager"));
    for (let i = 0; i < doors.length; ++i) {
        var door = doors[i];
        up.ImGuiSeparatorText(up.asToCString("Door "+i.toString()));
        up.clearArgs();
        if (up.ImGuiButton(up.asToCString("Lock##"+door.toString()))) {
            lockDoor(door);
        }
        if (up.ImGuiButton(up.asToCString("Unlock##"+door.toString()))) {
            unlockDoor(door);
        }
    }
    up.ImGuiEnd();
}

export function onDoorStart(): void {
    doors.push(up.getValueObject(0));
    up.call(up.getOriginal(), 2);
}


function lockDoor(door: up.ObjectHandle): void {
    up.addArgObject(door);
    up.addArgI32(true);
    up.addArgNull();
    up.call(up.getMethodByIdentifier(up.asToCString("Door$$LockDoor")), 3);
}

function unlockDoor(door: up.ObjectHandle): void {
    up.addArgObject(door);
    up.addArgNull();
    up.call(up.getMethodByIdentifier(up.asToCString("Door$$UnlockDoor")), 2);
}
