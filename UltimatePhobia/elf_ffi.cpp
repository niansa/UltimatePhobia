#include "elf_ffi.hpp"
#include "global_state.hpp"
#include "ffi_interface.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>

namespace {
__attribute__((sysv_abi)) void *malloc_wrap(size_t size) { return malloc(size); }
__attribute__((sysv_abi)) void free_wrap(void *buffer) { free(buffer); }
__attribute__((sysv_abi)) void *realloc_wrap(void *buffer, size_t size) { return realloc(buffer, size); }
using entrypoint_t = __attribute__((sysv_abi)) void (*)(decltype(malloc_wrap), decltype(free_wrap), decltype(realloc_wrap));
using callback_t = __attribute__((sysv_abi)) void (*)();
using init_imports_t = __attribute__((sysv_abi)) void (*)(const FFIInterface::Exports *);
} // namespace

ELFFFI::ELFFFI(const std::filesystem::path& modPath) : modPathStr(modPath.string()) {
    // Create a read callback function for ElfLoader
    auto reader = [this](uint64_t offset, std::span<std::byte> buffer) -> bool {
        static std::ifstream file(modPathStr, std::ios::binary);
        if (!file.is_open()) {
            g.logger->error("Failed to open ELF file: {}", modPathStr);
            return false;
        }

        file.seekg(offset);
        if (!file) {
            g.logger->error("Failed to seek to offset {} in ELF file", offset);
            return false;
        }

        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        if (file.gcount() != static_cast<std::streamsize>(buffer.size())) {
            g.logger->error("Failed to read {} bytes from ELF file at offset {}", buffer.size(), offset);
            return false;
        }

        return true;
    };

    try {
        // Create and load the ELF file
        loader.emplace(reader);
        loader->load();
        loader->relocate();

        // Call entrypoint with malloc/free/realloc function pointers
        reinterpret_cast<entrypoint_t>(loader->get_entry_point())(malloc_wrap, free_wrap, realloc_wrap);

        // Initialize imports
        const static FFIInterface::Exports exports;
        auto initImports = reinterpret_cast<init_imports_t>(loader->find_symbol("initImports"));

        if (!initImports)
            g.logger->error("ELF file is missing 'initImports' symbol!");
        else
            initImports(&exports);
    } catch (const ElfLoader::ElfError& e) {
        throw std::runtime_error(fmt::format("Failed to load ELF file '{}': {} (error code: {})", modPathStr, e.what(), static_cast<int>(e.code())));
    } catch (const std::exception& e) {
        throw std::runtime_error(fmt::format("Failed to load ELF file '{}': {}", modPathStr, e.what()));
    }
}

ELFFFI::~ELFFFI() {
    // ElfLoader will automatically clean up when destroyed
    loader.reset();
}

void ELFFFI::simpleCall(const char *name) {
    try {
        auto func = reinterpret_cast<callback_t>(loader->find_symbol(name));
        g.logger->flush();

        if (!func)
            g.logger->error("Failed to call ELF function {} in {} as it wasn't found", name, modPathStr);
        else
            func();
        g.logger->flush();
    } catch (const ElfLoader::ElfError& e) {
        g.logger->error("Error finding symbol '{}' in {}: {} (error code: {})", name, modPathStr, e.what(), static_cast<int>(e.code()));
    } catch (const std::exception& e) {
        g.logger->error("Error calling function '{}' in {}: {}", name, modPathStr, e.what());
    }
}
