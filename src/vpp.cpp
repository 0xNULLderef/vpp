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
        VPP::CBaseEntityGetScriptInstance = Memory::Scanner::Scan(server.span, "55 8B EC 51 56 8B F1 83 BE 50").As<CBaseEntityGetScriptInstance_t>();
        VPP::CBaseEntityGetGroundEntity = Memory::Scanner::Scan(server.span, "8B 89 58 01 00 00 83").As<CBaseEntityGetGroundEntity_t>();

        auto binding = &baseEntityScriptDesc->functionBindings.memory.memory[InsertBefore(&baseEntityScriptDesc->functionBindings, baseEntityScriptDesc->functionBindings.size)];
        binding->desc.description = "Gets the ground entity";
        binding->desc.scriptName = "GetGroundEntity";
        binding->desc.function = "GetGroundEntity";
        binding->desc.returnType = FIELD_HSCRIPT;
        binding->function = ScriptGetGroundEntity;
        binding->binding = MemberBindingHSCRIPT;
        binding->flags = 1;
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
