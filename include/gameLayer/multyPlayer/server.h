#pragma once
#include <unordered_map>

using CID = int32_t;

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
	std::unordered_map<CID, PerClientServerSettings> perClientSettings;

};

ServerSettings getServerSettingsCopy();

void setServerSettings(ServerSettings settings);

void addCidToServerSettings(CID cid);

void removeCidFromServerSettings(CID cid);