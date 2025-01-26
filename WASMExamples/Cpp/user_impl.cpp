#include "ffi_interface.hpp"

#include <user-impl.h>

using namespace FFIInterface;
using namespace Helpers;
using namespace Helpers::Literals;



extern "C" {
namespace {
ObjectHandle buffer = createCsString();

void append_buffer(ObjectHandle str) {
    ObjectHandle new_buffer = call<"System_String_o* System_String__Concat (System_String_o* str0, System_String_o* str1, const MethodInfo* method);", ObjectHandle>(buffer, str, nullptr);
    dropObject(buffer);
    dropObject(str);
    buffer = new_buffer;
}
void flush_buffer() {
    logInfo(buffer);
    dropObject(buffer);
    buffer = createCsString();
}
}

void _putchar(char c) {
    if (c == '\n')
        flush_buffer();
    else
        append_buffer(toCsStringWithLength(&c, 1));
}
void _puts(const char *str) {
    const char *remaining = nullptr;
    size_t length = 0;

    for (; str[length]; ++length) {
        if (str[length] == '\n') {
            remaining = str + length + 1;
            break;
        }
    }

    append_buffer(toCsStringWithLength(str, length));
    if (remaining) {
        flush_buffer();
        return _puts(remaining);
    }
    flush_buffer();
}
}
