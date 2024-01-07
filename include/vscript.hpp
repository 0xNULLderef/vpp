#ifndef VSCRIPT_HPP
#define VSCRIPT_HPP

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

typedef void* CBaseEntity;

#endif // VSCRIPT_HPP
