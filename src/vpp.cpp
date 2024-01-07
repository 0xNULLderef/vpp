#include "vpp.hpp"

#include "modules.hpp"
#include "scanner.hpp"
#include "util.hpp"
#include "logger.hpp"
#include <stdexcept>

template<class T> struct CUtlMemory {
    T* memory;
    int allocationCount;
    int growSize;
};

template<class T, class A = CUtlMemory<T>> struct CUtlVector {
    A memory;
    int size;
    T* elements;
};

typedef void* HSCRIPT;

enum ScriptDataType_t : int {
    FIELD_VOID = 0,
    FIELD_FLOAT,
    FIELD_STRING,
    FIELD_VECTOR,
    FIELD_QUATERNION,
    FIELD_INTEGER,
    FIELD_BOOLEAN,
    FIELD_SHORT,
    FIELD_CHARACTER,
    FIELD_COLOR32,
    FIELD_EMBEDDED,
    FIELD_CUSTOM,
    FIELD_CLASSPTR,
    FIELD_EHANDLE,
    FIELD_EDICT,
    FIELD_POSITION_VECTOR,
    FIELD_TIME,
    FIELD_TICK,
    FIELD_MODELNAME,
    FIELD_SOUNDNAME,
    FIELD_INPUT,
    FIELD_FUNCTION,
    FIELD_VMATRIX,
    FIELD_VMATRIX_WORLDSPACE,
    FIELD_MATRIX3X4_WORLDSPACE,
    FIELD_INTERVAL,
    FIELD_MODELINDEX,
    FIELD_MATERIALINDEX,
    FIELD_VECTOR2D,
    FIELD_INTEGER64,
    FIELD_VECTOR4D,
    FIELD_TYPECOUNT,
    FIELD_TYPEUNKNOWN = FIELD_TYPECOUNT,
    FIELD_CSTRING,
    FIELD_HSCRIPT,
    FIELD_VARIANT
};

struct ScriptFuncDescriptor_t {
    const char* scriptName;
    const char* function;
    const char* description;
    ScriptDataType_t returnType;
    CUtlVector<ScriptDataType_t> parameterTypes;
};

struct ScriptVariant_t {
    union {
        int integerValue;
        float floatValue;
        const char* stringValue;
        char charValue;
        bool boolValue;
        HSCRIPT handleValue;
    };
    short type;
    short flags;
};

typedef void* ScriptFunctionBindingStorageType_t;

typedef bool(*ScriptBindingFunc_t)(ScriptFunctionBindingStorageType_t function, void* context, ScriptVariant_t* arguments, int argumentCount, ScriptVariant_t* returnValue);

struct ScriptFunctionBinding_t {
    ScriptFuncDescriptor_t desc;
    ScriptBindingFunc_t binding;
    ScriptFunctionBindingStorageType_t function;
    unsigned int flags;
};

struct ScriptClassDesc_t {
    const char* scriptName;
    const char* className;
    const char* description;
    ScriptClassDesc_t* baseDesc;
    CUtlVector<ScriptFunctionBinding_t> functionBindings;
    void* (*construct)();
    void (*destruct)(void*);
    void* helper;
    ScriptClassDesc_t* next;
};

typedef ScriptClassDesc_t*(*GetScriptDesc_t)();
typedef int(__thiscall* InsertBefore_t)(CUtlVector<ScriptFunctionBinding_t>* thisptr, int element);

typedef void* CBaseEntity;
typedef HSCRIPT (__thiscall* CBaseEntityGetScriptInstance_t)(CBaseEntity* thisptr);
static CBaseEntityGetScriptInstance_t CBaseEntityGetScriptInstance;

typedef CBaseEntity*(__thiscall* CBaseEntityGetGroundEntity_t)(CBaseEntity* thisptr);
static CBaseEntityGetGroundEntity_t CBaseEntityGetGroundEntity;

HSCRIPT __fastcall ScriptGetGroundEntity(CBaseEntity* thisptr) {
    auto groundEntity = CBaseEntityGetGroundEntity(thisptr);
    return groundEntity != nullptr ? CBaseEntityGetScriptInstance(groundEntity) : nullptr;
}

VPP::VPP() { }

bool VPP::Load() {
    try {
        // put code here!
        auto server = Memory::Modules::Instance()->Get("server");
        auto baseEntityScriptDesc = Memory::Scanner::Scan(server.span, "E8 ?? ?? ?? ?? 8B 55 1C 8B 0D", 1).ReadAsRelativePointer<GetScriptDesc_t>()();
        auto InsertBefore = Memory::Scanner::Scan(server.span, "E8 ?? ?? ?? ?? 8D 04 40 C1 E0 04 03 05 ?? ?? ?? ?? 89 58 2C", 1).ReadAsRelativePointer<InsertBefore_t>();
        auto MemberBindingHSCRIPT = Memory::Scanner::Scan(server.span, "BE ?? ?? ?? ?? C7 40 0C 21 00 00 00", 1).ReadAsPointer<ScriptBindingFunc_t>();
        CBaseEntityGetScriptInstance = Memory::Scanner::Scan(server.span, "55 8B EC 51 56 8B F1 83 BE 50").As<CBaseEntityGetScriptInstance_t>();
        CBaseEntityGetGroundEntity = Memory::Scanner::Scan(server.span, "8B 89 58 01 00 00 83").As<CBaseEntityGetGroundEntity_t>();

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
