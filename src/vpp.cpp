#include "vpp.hpp"

#include "modules.hpp"
#include "scanner.hpp"
#include "util.hpp"
#include "logger.hpp"
#include <stdexcept>

VPP::VPP() { }

bool VPP::Load() {
    try {
        // put code here!
        auto server = Memory::Modules::Instance()->Get("server");
        auto baseEntityScriptDesc = Memory::Scanner::Scan(server.span, "E8 ?? ?? ?? ?? 8B 55 1C 8B 0D", 1).ReadAsRelativePointer<GetScriptDesc_t>()();
        auto InsertBefore = Memory::Scanner::Scan(server.span, "E8 ?? ?? ?? ?? 8D 04 40 C1 E0 04 03 05 ?? ?? ?? ?? 89 58 2C", 1).ReadAsRelativePointer<CUtlVectorInsertBefore_t>();
        auto MemberBindingHSCRIPT = Memory::Scanner::Scan(server.span, "BE ?? ?? ?? ?? C7 40 0C 21 00 00 00", 1).ReadAsPointer<ScriptBindingFunc_t>();
        auto MemberBindingVoid = Memory::Scanner::Scan(server.span, "55 8B EC 83 7D 14 01 75 23 83 7D 18 00 75 1D 8B").As<ScriptBindingFunc_t>();
        auto DeduceFunctionSignatureEmitSound = Memory::Scanner::Scan(server.span, "E8 ?? ?? ?? ?? 83 C4 0C C7 46 ?? ?? ?? ?? ?? C7 46 ?? ?? ?? ?? ?? 89 5E 2C A1", 1).ReadAsRelativePointer<DeduceFunctionSignatureEmitSound_t>();
        VPP::CBaseEntityGetScriptInstance = Memory::Scanner::Scan(server.span, "55 8B EC 51 56 8B F1 83 BE 50").As<CBaseEntityGetScriptInstance_t>();
        VPP::CBaseEntityGetGroundEntity = Memory::Scanner::Scan(server.span, "8B 89 58 01 00 00 83").As<CBaseEntityGetGroundEntity_t>();
        VPP::CBaseEntityStopSound = Memory::Scanner::Scan(server.span, "55 8B EC 51 8B 41 1C 85").As<CBaseEntityStopSound_t>();

        {
            auto binding = &baseEntityScriptDesc->functionBindings.memory.memory[InsertBefore(&baseEntityScriptDesc->functionBindings, baseEntityScriptDesc->functionBindings.size)];
            binding->desc.description = "Gets the ground entity";
            binding->desc.scriptName = "GetGroundEntity";
            binding->desc.function = "ScriptGetGroundEntity";
            binding->desc.returnType = FIELD_HSCRIPT;
            binding->function = ScriptGetGroundEntity;
            binding->binding = MemberBindingHSCRIPT;
            binding->flags = 1;
        }

        {
            auto binding = &baseEntityScriptDesc->functionBindings.memory.memory[InsertBefore(&baseEntityScriptDesc->functionBindings, baseEntityScriptDesc->functionBindings.size)];
            binding->desc.description = "Stops a sound on this entity.";
            binding->desc.scriptName = "StopSound";
            binding->desc.function = "ScriptStopSound";
            DeduceFunctionSignatureEmitSound(&binding->desc);
            binding->function = ScriptStopSound;
            binding->binding = MemberBindingVoid;
            binding->flags = 1;
        }


    } catch(const std::exception& ex) {
        ERROR("%s\n", ex.what());
        return false;
    }

    LOG("vpp loaded :3\n");

    return true;
}

void VPP::Unload() {
    for(auto detour : this->detours) {
        delete detour;
    }

    LOG("unloading vpp >////<\n");
}

const char* VPP::Description() {
    return "vscript plus plus?";
}

HSCRIPT __fastcall VPP::ScriptGetGroundEntity(CBaseEntity* thisptr) {
    auto groundEntity = VPP::CBaseEntityGetGroundEntity(thisptr);
    return groundEntity != nullptr ? VPP::CBaseEntityGetScriptInstance(groundEntity) : nullptr;
}

void __fastcall VPP::ScriptStopSound(CBaseEntity* thisptr, void*, const char* soundname) {
    VPP::CBaseEntityStopSound(thisptr, soundname);
}

VPP::CBaseEntityGetScriptInstance_t VPP::CBaseEntityGetScriptInstance = nullptr;
VPP::CBaseEntityGetGroundEntity_t VPP::CBaseEntityGetGroundEntity = nullptr;
VPP::CBaseEntityStopSound_t VPP::CBaseEntityStopSound = nullptr;
