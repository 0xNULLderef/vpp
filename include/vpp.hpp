#ifndef VPP_HPP
#define VPP_HPP

#include "singleton.hpp"
#include "detour.hpp"
#include "vscript.hpp"
#include <vector>

class VPP : public Singleton<VPP> {
public:
    VPP();

    bool Load();
    void Unload();
    const char* Description();

    using GetScriptDesc_t = ScriptClassDesc_t*(*)();
    using CUtlVectorInsertBefore_t = int(__thiscall*)(void* thisptr, int elem);
    using CBaseEntityGetScriptInstance_t = HSCRIPT(__thiscall*)(void* thisptr);
    using CBaseEntityGetGroundEntity_t = CBaseEntity*(__thiscall*)(CBaseEntity* thisptr);

    static CBaseEntityGetScriptInstance_t CBaseEntityGetScriptInstance;
    static CBaseEntityGetGroundEntity_t CBaseEntityGetGroundEntity;

    static HSCRIPT __fastcall ScriptGetGroundEntity(CBaseEntity* thisptr);

private:
    std::vector<Memory::DetourBase*> detours;
};

#endif // VPP_HPP
