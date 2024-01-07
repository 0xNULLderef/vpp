#include "modules.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
#else
#endif
#include <filesystem>
#include <stdexcept>

namespace Memory {
    Modules::Modules() {
        this->PopulateModules();
    }

    void Modules::PopulateModules() {
        HMODULE modules[1024];
        auto processHandle = GetCurrentProcess();
        DWORD modulesNeeded;
        if(EnumProcessModules(processHandle, modules, sizeof(modules), &modulesNeeded)) {
            for(DWORD i = 0; i < (modulesNeeded / sizeof(HMODULE)); i++) {
                char pathBuffer[MAX_PATH];
                if(!GetModuleFileNameA(modules[i], pathBuffer, sizeof(pathBuffer))) continue;
                MODULEINFO moduleInfo = {};
                if(!GetModuleInformation(processHandle, modules[i], &moduleInfo, sizeof(MODULEINFO))) continue;
                std::filesystem::path modulePath(pathBuffer);
                this->loadedModules.insert(
                    std::make_pair(
                        modulePath.stem().string(),
                        Module(
                            modules[i],
                            std::span<std::byte>(
                                reinterpret_cast<std::byte*>(moduleInfo.lpBaseOfDll),
                                static_cast<size_t>(moduleInfo.SizeOfImage)
                            )
                        )
                    )
                );
            }
        }
    }

    const Module& Modules::Get(std::string name) {
        if(this->loadedModules.empty()) {
            PopulateModules();
        }

        if(this->loadedModules.contains(name)) {
            return this->loadedModules[name];
        } else {
            throw std::runtime_error("Failed to Get a required module");
        }
    }
};
