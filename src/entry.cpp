#include <cstring>
#include <stdexcept>
#include "vpp.hpp"
#include "singleton.hpp"
#include "logger.hpp"

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

class IServerPluginCallbacks {
public:
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) = 0;
    virtual void Unload() = 0;
    virtual void Pause() = 0;
    virtual void UnPause() = 0;
    virtual const char* GetPluginDescription() = 0;
    virtual void LevelInit(char const* pMapName) = 0;
    virtual void ServerActivate(void* pEdictList, int edictCount, int clientMax) = 0;
    virtual void GameFrame(bool simulating) = 0;
    virtual void LevelShutdown() = 0;
    virtual void ClientFullyConnect(void* pEdict) = 0;
    virtual void ClientActive(void* pEntity) = 0;
    virtual void ClientDisconnect(void* pEntity) = 0;
    virtual void ClientPutInServer(void* pEntity, char const* playername) = 0;
    virtual void SetCommandClient(int index) = 0;
    virtual void ClientSettingsChanged(void* pEdict) = 0;
    virtual int ClientConnect(bool* bAllowConnect, void* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen) = 0;
    virtual int ClientCommand(void* pEntity, const void*& args) = 0;
    virtual int NetworkIDValidated(const char* pszUserName, const char* pszNetworkID) = 0;
    virtual void OnQueryCvarValueFinished(int iCookie, void* pPlayerEntity, int eStatus, const char* pCvarName, const char* pCvarValue) = 0;
    virtual void OnEdictAllocated(void* edict) = 0;
    virtual void OnEdictFreed(const void* edict) = 0;
};

class Entry : public IServerPluginCallbacks, public Singleton<Entry> {
    bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) override;
    void Unload() override;
    void Pause() override;
    void UnPause() override;
    const char* GetPluginDescription() override;
    void LevelInit(char const* pMapName) override;
    void ServerActivate(void* pEdictList, int edictCount, int clientMax) override;
    void GameFrame(bool simulating) override;
    void LevelShutdown() override;
    void ClientFullyConnect(void* pEdict) override;
    void ClientActive(void* pEntity) override;
    void ClientDisconnect(void* pEntity) override;
    void ClientPutInServer(void* pEntity, char const* playername) override;
    void SetCommandClient(int index) override;
    void ClientSettingsChanged(void* pEdict) override;
    int ClientConnect(bool* bAllowConnect, void* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen) override;
    int ClientCommand(void* pEntity, const void*& args) override;
    int NetworkIDValidated(const char* pszUserName, const char* pszNetworkID) override;
    void OnQueryCvarValueFinished(int iCookie, void* pPlayerEntity, int eStatus, const char* pCvarName, const char* pCvarValue) override;
    void OnEdictAllocated(void* edict) override;
    void OnEdictFreed(const void* edict) override;
};

bool Entry::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) {
    return VPP::Instance()->Load();
}

void Entry::Unload() {
    return VPP::Instance()->Unload();
}

const char* Entry::GetPluginDescription() {
    return VPP::Instance()->Description();
}

void Entry::Pause() { }
void Entry::UnPause() { }
void Entry::LevelInit(char const* pMapName) { }
void Entry::ServerActivate(void* pEdictList, int edictCount, int clientMax) { }
void Entry::GameFrame(bool simulating) { }
void Entry::LevelShutdown() { }
void Entry::ClientFullyConnect(void* pEdict) { }
void Entry::ClientActive(void* pEntity) { }
void Entry::ClientDisconnect(void* pEntity) { }
void Entry::ClientPutInServer(void* pEntity, char const* playername) { }
void Entry::SetCommandClient(int index) { }
void Entry::ClientSettingsChanged(void* pEdict) { }
int Entry::ClientConnect(bool* bAllowConnect, void* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen) { return 0; }
int Entry::ClientCommand(void* pEntity, const void*& args) { return 0; }
int Entry::NetworkIDValidated(const char* pszUserName, const char* pszNetworkID) { return 0; }
void Entry::OnQueryCvarValueFinished(int iCookie, void* pPlayerEntity, int eStatus, const char* pCvarName, const char* pCvarValue) { }
void Entry::OnEdictAllocated(void* edict) { }
void Entry::OnEdictFreed(const void* edict) { }


extern "C" __declspec(dllexport) void* CreateInterface(const char* pName, int* pReturnCode) {
    if(strcmp(pName, "ISERVERPLUGINCALLBACKS002") == 0) {
        if(pReturnCode != nullptr) *pReturnCode = 0;
        return reinterpret_cast<void*>(Entry::Instance());
    } else {
        if(pReturnCode != nullptr) *pReturnCode = 1;
        return nullptr;
    }
}
