#include "../anycall.hpp"

#include <string>
#include <stdexcept>

namespace AnyCall {
uintptr_t call(const uintptr_t *args, void *fnc, std::string_view signature) {
    if (signature == "viii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])); return 0;}
    if (signature == "iiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "iii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "iiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "vi")
        {reinterpret_cast<void (*)(uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0])); return 0;}
    if (signature == "ii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0])));
    if (signature == "vii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1])); return 0;}
    if (signature == "viiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "viiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "viiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "viiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "viiiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "iiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iiiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iiiiiiiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12])));
    if (signature == "iiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iiiiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "iiiiiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10])));
    if (signature == "iiiiiiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11])));
    if (signature == "viiiiiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])); return 0;}
    if (signature == "ifi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "idi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(double, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "fiii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "diii")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "viiiiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "iji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "ijiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "ijiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viiiijjiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "iiiiijjiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10])));
    if (signature == "ji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0])));
    if (signature == "jii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "viji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])); return 0;}
    if (signature == "viiji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "vijijji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "jiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "iiji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "iijji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "iijjji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viijji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "viijjji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "viiiji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "jdi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(double, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "dji")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "jji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "jfi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "fii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "fji")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "ffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "fdi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(double, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "dii")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "dfi")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "ijii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "idii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(double, uintptr_t, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "vijii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "vijiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "viiiiiiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10])); return 0;}
    if (signature == "iidii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, double, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "iidi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "jiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "vifi")
        {reinterpret_cast<void (*)(uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2])); return 0;}
    if (signature == "vidi")
        {reinterpret_cast<void (*)(uintptr_t, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2])); return 0;}
    if (signature == "iddi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(double, double, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "idiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(double, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "diiii")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "jdii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(double, uintptr_t, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "fdii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(double, uintptr_t, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "ddii")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(double, uintptr_t, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "vji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1])); return 0;}
    if (signature == "viiiiiiiiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12])); return 0;}
    if (signature == "ijji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "ijiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "jjii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "fjii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "djii")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "ddi")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(double, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<uintptr_t>(args[1])));
    if (signature == "dddi")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(double, double, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "ddiii")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(double, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "jjji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "fffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "ifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "fiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "ifii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "iffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "jfii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "ffii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "dfii")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(float, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "iijiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "jiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viidii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, double, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "iijii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "iifii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "viiiiiiiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11])); return 0;}
    if (signature == "viijii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "jijii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "viijjiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "jijiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiiiiji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iiiji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "iiiiji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viiiiiiiiiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12]), bit_cast<uintptr_t>(args[13])); return 0;}
    if (signature == "fiffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "fifi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "jijji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "diddi")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(uintptr_t, double, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "jiji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "didi")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(uintptr_t, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "viiijji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "iijjii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viiijii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "vijjjii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "jjjji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "viijijii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "viijijiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "vijiji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "viijiijiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "viiiijiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "viifi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "jiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "vijji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "viifiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "viifii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "viijiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "viiiiji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "viiiiiiiji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "viiijjiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "viiijjii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "iiiijiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iidiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, double, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viidi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "diji")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "jidi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "fidi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "iifi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "iiiijjii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iiiijjiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iiidii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, double, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "vijjji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "iiiiiiji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "viffi")
        {reinterpret_cast<void (*)(uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "viffffi")
        {reinterpret_cast<void (*)(uintptr_t, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "iffffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, float, float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viiifii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "fi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0])));
    if (signature == "vfffi")
        {reinterpret_cast<void (*)(float, float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "vffi")
        {reinterpret_cast<void (*)(float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2])); return 0;}
    if (signature == "vifffi")
        {reinterpret_cast<void (*)(uintptr_t, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "vffffi")
        {reinterpret_cast<void (*)(float, float, float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "viiifi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "vfi")
        {reinterpret_cast<void (*)(float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1])); return 0;}
    if (signature == "vifii")
        {reinterpret_cast<void (*)(uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "iiiffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "viiffii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "iiifi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "iffffffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, float, float, float, float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "vffffffii")
        {reinterpret_cast<void (*)(float, float, float, float, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "vffffii")
        {reinterpret_cast<void (*)(float, float, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "iiiiffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iiiifi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiiifffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "ifffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "vfiii")
        {reinterpret_cast<void (*)(float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "ffffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "fffifi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, float, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "fffifffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, float, uintptr_t, float, float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "vjiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "viiffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "vifffii")
        {reinterpret_cast<void (*)(uintptr_t, float, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "iifffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiiifiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iiiifiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iiiifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iiiifii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "viiiijji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "vijjii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "viiiiiiiiiiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12]), bit_cast<uintptr_t>(args[13]), bit_cast<uintptr_t>(args[14])); return 0;}
    if (signature == "viiiifi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "vififfii")
        {reinterpret_cast<void (*)(uintptr_t, float, uintptr_t, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "vififi")
        {reinterpret_cast<void (*)(uintptr_t, float, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "iiifii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iiiiifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iiifiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iiiififiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "iiiifiifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "iiififiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iiifiifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iiiiiifiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "iiiiiiifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "iifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiifiifii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iifiifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iifiifii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iififiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iififii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iiiiiifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iiiiiifi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iiiififiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iiififiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iiififi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iiffiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iiiiiiffiiiiiiiiiffffiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12]), bit_cast<uintptr_t>(args[13]), bit_cast<uintptr_t>(args[14]), bit_cast<uintptr_t>(args[15]), bit_cast<float>(args[16]), bit_cast<float>(args[17]), bit_cast<float>(args[18]), bit_cast<float>(args[19]), bit_cast<uintptr_t>(args[20]), bit_cast<uintptr_t>(args[21]), bit_cast<uintptr_t>(args[22]), bit_cast<uintptr_t>(args[23])));
    if (signature == "iiiiiiffiiiiiiiiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12]), bit_cast<uintptr_t>(args[13]), bit_cast<uintptr_t>(args[14]), bit_cast<uintptr_t>(args[15]), bit_cast<uintptr_t>(args[16]), bit_cast<uintptr_t>(args[17]), bit_cast<uintptr_t>(args[18]), bit_cast<uintptr_t>(args[19]), bit_cast<uintptr_t>(args[20]), bit_cast<uintptr_t>(args[21])));
    if (signature == "viiiififfi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<float>(args[6]), bit_cast<float>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "viiiifiifi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<float>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "viiiifiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "viiiifiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])); return 0;}
    if (signature == "viiiifiiiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12])); return 0;}
    if (signature == "iiiiffiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11])));
    if (signature == "fifffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "fifffiii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, float, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "fiffffiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, float, float, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10])));
    if (signature == "fiffffii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, float, float, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "fiffffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "viiiiiffii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "ffffii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "ffffiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iffiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iffffiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, float, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "fiifi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "viffffii")
        {reinterpret_cast<void (*)(uintptr_t, float, float, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "viffffiii")
        {reinterpret_cast<void (*)(uintptr_t, float, float, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "viiffffiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11])); return 0;}
    if (signature == "viiifiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "iiififfi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iiiiifi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "vifffffi")
        {reinterpret_cast<void (*)(uintptr_t, float, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "viiiifii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "vjii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2])); return 0;}
    if (signature == "ijjiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iiijji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viififi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "viififfi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "di")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0])));
    if (signature == "viiiffii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "fiiffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiiffiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iiififii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "vififiii")
        {reinterpret_cast<void (*)(uintptr_t, float, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "viffii")
        {reinterpret_cast<void (*)(uintptr_t, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "viijjii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "iiijjiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iiijiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iiidiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, double, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "viifiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "viffiii")
        {reinterpret_cast<void (*)(uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "fifii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "viiiffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "viiifffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "fiifii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viiiiiffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "iiffffiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, float, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iiidfi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, double, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<double>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiijfi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiiffii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iiiififi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iiiffifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, float, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iiifiifiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "ifffii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "ffffifi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, float, float, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "ffffiffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, float, float, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "vifiii")
        {reinterpret_cast<void (*)(uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "iifiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iifiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iiffiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iiffifii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, float, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iififi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiffii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiffifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, float, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iiifiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iiiiifiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iiiiifii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "viiiiiifiifiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<float>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12])); return 0;}
    if (signature == "viidiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, double, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "diidi")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(uintptr_t, uintptr_t, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "fiifdi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, float, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<double>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viiiiiifddfiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, double, double, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<float>(args[6]), bit_cast<double>(args[7]), bit_cast<double>(args[8]), bit_cast<float>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12])); return 0;}
    if (signature == "viijiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "jiiji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "fiifji")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viiiiiifjjfiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<float>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12])); return 0;}
    if (signature == "viiiifiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "viiiiiiffffiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<float>(args[6]), bit_cast<float>(args[7]), bit_cast<float>(args[8]), bit_cast<float>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12])); return 0;}
    if (signature == "iifiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "fffffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, float, float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "fiiffffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "viidiji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, double, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "viidjii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, double, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "viiiidii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, double, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<double>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "vidiiiii")
        {reinterpret_cast<void (*)(uintptr_t, double, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "viiidjii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, double, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<double>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "iiiiiiidii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, double, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<double>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iiidi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "viiiidi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<double>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "viijijji")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "iiiidi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<double>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "vidii")
        {reinterpret_cast<void (*)(uintptr_t, double, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "viiiiidiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, double, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<double>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10])); return 0;}
    if (signature == "viddi")
        {reinterpret_cast<void (*)(uintptr_t, double, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3])); return 0;}
    if (signature == "viiiiidiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, double, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<double>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])); return 0;}
    if (signature == "difi")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2])));
    if (signature == "viiifiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "ijjji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "fiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viifffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "viiiiifffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<float>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "viiiiffffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<float>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "ifiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "fiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "fiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "viiiiffii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "iiiiiffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "ffffffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, float, float, float, float, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "fiiifi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viffiiiii")
        {reinterpret_cast<void (*)(uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "vffffffiiii")
        {reinterpret_cast<void (*)(float, float, float, float, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])); return 0;}
    if (signature == "viiiiiiifii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<float>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])); return 0;}
    if (signature == "viiiiifii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "viffffffi")
        {reinterpret_cast<void (*)(uintptr_t, float, float, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "viiiffffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "viiiiifi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "viiffffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "viifffffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "viiffffffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<float>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "viifffffffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, float, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<float>(args[7]), bit_cast<float>(args[8]), bit_cast<uintptr_t>(args[9])); return 0;}
    if (signature == "viiffffffffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, float, float, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<float>(args[7]), bit_cast<float>(args[8]), bit_cast<float>(args[9]), bit_cast<uintptr_t>(args[10])); return 0;}
    if (signature == "vifiiii")
        {reinterpret_cast<void (*)(uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "vidiii")
        {reinterpret_cast<void (*)(uintptr_t, double, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<double>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])); return 0;}
    if (signature == "viiffffffffiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, float, float, float, float, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<float>(args[7]), bit_cast<float>(args[8]), bit_cast<float>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12])); return 0;}
    if (signature == "viiiiffffii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<float>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])); return 0;}
    if (signature == "vifffiii")
        {reinterpret_cast<void (*)(uintptr_t, float, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "vfiiiii")
        {reinterpret_cast<void (*)(float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "vffffiiii")
        {reinterpret_cast<void (*)(float, float, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "iiiiijiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iiiifdi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<double>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "jiiiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "jiiiiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10])));
    if (signature == "jiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "jiiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "jiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iiifdi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<double>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iifdi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, double, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "fffii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "vifffffffi")
        {reinterpret_cast<void (*)(uintptr_t, float, float, float, float, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<float>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "iififfi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "viiiiiiiiiiiiiiiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12]), bit_cast<uintptr_t>(args[13]), bit_cast<uintptr_t>(args[14]), bit_cast<uintptr_t>(args[15]), bit_cast<uintptr_t>(args[16])); return 0;}
    if (signature == "viiiiififi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<float>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "viiiiiififi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<float>(args[8]), bit_cast<uintptr_t>(args[9])); return 0;}
    if (signature == "viiiiiifi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "viiiffifii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "viiiiffi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6])); return 0;}
    if (signature == "fiffii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iiifffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iiiiffiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "jiiifi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "viiffffiifiifi")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, float, float, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<float>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<float>(args[11]), bit_cast<uintptr_t>(args[12])); return 0;}
    if (signature == "ddddi")
        return bit_cast<uintptr_t>(reinterpret_cast<double (*)(double, double, double, uintptr_t)>(fnc)(bit_cast<double>(args[0]), bit_cast<double>(args[1]), bit_cast<double>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "vijjiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "iiiiffii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "viiiiffiii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "fifiii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iifiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "viifffffii")
        {reinterpret_cast<void (*)(uintptr_t, uintptr_t, float, float, float, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}
    if (signature == "iiiififii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "ffiii")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "iiiffiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iijiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iijiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "iijiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iijiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iijiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "vifffffii")
        {reinterpret_cast<void (*)(uintptr_t, float, float, float, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])); return 0;}
    if (signature == "iifffffiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, float, float, float, float, float, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "iijfffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iijfffiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7])));
    if (signature == "iijfi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "iijfiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "iijffi")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4])));
    if (signature == "iijffiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iijjiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iiijjiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6])));
    if (signature == "iijiiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10])));
    if (signature == "iijiiiiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11]), bit_cast<uintptr_t>(args[12])));
    if (signature == "iiiiiiiiji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iiiiiiiijiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10])));
    if (signature == "iijiiiiiiji")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "iijiiiiiijiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10]), bit_cast<uintptr_t>(args[11])));
    if (signature == "iijjiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])));
    if (signature == "iijjiiiiiiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<uintptr_t>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9]), bit_cast<uintptr_t>(args[10])));
    if (signature == "fiifffi")
        return bit_cast<uintptr_t>(reinterpret_cast<float (*)(uintptr_t, uintptr_t, float, float, float, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<uintptr_t>(args[5])));
    if (signature == "vfffiii")
        {reinterpret_cast<void (*)(float, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5])); return 0;}
    if (signature == "iiiiffffiii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, float, float, float, float, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "iffffffffii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(float, float, float, float, float, float, float, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<float>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<float>(args[3]), bit_cast<float>(args[4]), bit_cast<float>(args[5]), bit_cast<float>(args[6]), bit_cast<float>(args[7]), bit_cast<uintptr_t>(args[8]), bit_cast<uintptr_t>(args[9])));
    if (signature == "jjjii")
        return bit_cast<uintptr_t>(reinterpret_cast<uintptr_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<uintptr_t>(args[1]), bit_cast<uintptr_t>(args[2]), bit_cast<uintptr_t>(args[3])));
    if (signature == "viffiiifii")
        {reinterpret_cast<void (*)(uintptr_t, float, float, uintptr_t, uintptr_t, uintptr_t, float, uintptr_t, uintptr_t)>(fnc)(bit_cast<uintptr_t>(args[0]), bit_cast<float>(args[1]), bit_cast<float>(args[2]), bit_cast<uintptr_t>(args[3]), bit_cast<uintptr_t>(args[4]), bit_cast<uintptr_t>(args[5]), bit_cast<float>(args[6]), bit_cast<uintptr_t>(args[7]), bit_cast<uintptr_t>(args[8])); return 0;}

    throw std::runtime_error("Unknown function signature: " + std::string(signature));
}
} // namespace AnyCall
