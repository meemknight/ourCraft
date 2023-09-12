#pragma once
#include <unordered_map>


bool isServerRunning();

bool startServer();
void closeServer();
void serverWorkerFunction();


struct PerClientServerSettings
{

	bool validateStuff = true;


};

struct ServerSettings
{
	std::unordered_map<int32_t, PerClientServerSettings> perClientSettings;

};

ServerSettings getServerSettingsCopy();

void setServerSettings(ServerSettings settings);

void addCidToServerSettings(int32_t cid);

void removeCidFromServerSettings(int32_t cid);