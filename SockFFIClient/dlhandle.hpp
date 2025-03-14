#ifndef __WIN32
#include <string>
#include <exception>
#include <dlfcn.h>

class Dlhandle {
    void *chandle;

public:
    class Exception : public std::exception {
        std::string errmsg;

    public:
        Exception(std::string errmsg) { this->errmsg = errmsg; }
        virtual const char *what() const throw() { return errmsg.c_str(); }
    };

    Dlhandle(std::string fpath, int flags = RTLD_LAZY) {
        chandle = dlopen(fpath.c_str(), flags);
        if (chandle == nullptr)
            throw Exception("dlopen(): " + fpath + ": " + dlerror());
    }
    ~Dlhandle() { dlclose(chandle); }
    Dlhandle(const Dlhandle&) = delete;
    Dlhandle(Dlhandle&&) = delete;

    template <typename T> T *get(const std::string& fname) {
        dlerror(); // Clear error
        auto fres = reinterpret_cast<T *>(dlsym(chandle, fname.c_str()));
        return (dlerror() == NULL) ? fres : nullptr;
    }
    auto getFnc(const std::string& fname) { return get<void *(...)>(fname); }
};
#else
#include <string>
#include <exception>
#include <libloaderapi.h>

class Dlhandle {
    HMODULE chandle;

public:
    class Exception : public std::exception {
        std::string errmsg;

    public:
        Exception(std::string errmsg) { this->errmsg = errmsg; }
        virtual const char *what() const throw() { return errmsg.c_str(); }
    };

    Dlhandle(std::string fpath) {
        chandle = LoadLibraryA(fpath.c_str());
        if (!chandle)
            throw Exception("dlopen(): " + fpath);
    }
    ~Dlhandle() { FreeLibrary(chandle); }
    Dlhandle(const Dlhandle&) = delete;
    Dlhandle(Dlhandle&&) = delete;

    template <typename T> T *get(const std::string& fname) { return reinterpret_cast<T *>(GetProcAddress(chandle, fname.c_str())); }
    auto getFnc(const std::string& fname) { return get<void *(...)>(fname); }
};
#endif
