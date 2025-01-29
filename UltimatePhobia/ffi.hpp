#pragma once


class FFI {
public:
    FFI() {}
    virtual ~FFI() {}

    virtual void simpleCall(const char *name) = 0;
};
