#ifndef VPP_HPP
#define VPP_HPP

#include "singleton.hpp"
#include "detour.hpp"
#include <vector>

class VPP : public Singleton<VPP> {
public:
    VPP();

    bool Load();
    void Unload();
    const char* Description();

private:
    std::vector<Memory::DetourBase*> detours;
};

#endif // VPP_HPP
