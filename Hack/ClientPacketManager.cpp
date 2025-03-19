#include "ClientPacketManager.h"
#include "MCPE/GameData.h"
#include "HackSDK.h"

ClientPacketManager::ClientPacketManager()
{
	addConfig(&packetNum, "packetNum");
	addConfig(&packetName, "packetName");
	addConfig(&packetBoost, "packetBoost");
	addConfig(&noPacket, "noPacket");

	ModuleType = "Other";
	UIType = 1;
}

void ClientPacketManager::initViews()
{
	SecondWindowList.push_back(UIUtils::createSimpleTextViewWithBool(&noPacket,"NoPacket"));
	SecondWindowList.push_back(UIUtils::createSimpleTextViewWithBool(&packetBoost, "PacketBoost"));
	SecondWindowList.push_back(UIUtils::createSimpleEditTextWithString(&packetName, "PacketName"));
	SecondWindowList.push_back(UIUtils::createSimpleEditTextWithInt(&packetNum, "packetNum"));

}

const char* ClientPacketManager::GetName()
{
	return "ClientPacketManager";
}


void ClientPacketManager::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".NoPacket") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			noPacket = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			noPacket = false;
		}
	}
	else if ((*cmd)[0] == ".PacketBoost") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			packetBoost = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			packetBoost = false;
		}
		else {
			moduleManager->executedCMD = true;
			packetNum = atof((*cmd)[1].c_str());
			if (cmd->size() > 2) {
				packetName = (*cmd)[2];
			}
		}
	}
}

void ClientPacketManager::OnSendPacket(Packet* packet)
{
	if (noPacket) {
		MCHook::noPacketThisTime = true;
		return;
	}
	if (packetBoost) {
		if (packetName == "") {
			for (int i = 0; i != packetNum; i = i + 1) {
				(*MCHook::old_LPSSendToServer)(mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getPacketSender(), *packet);
			}
		}
		else {
			if (packet->getName() == packetName) {
				for (int i = 0; i != packetNum; i = i + 1) {
					(*MCHook::old_LPSSendToServer)(mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getPacketSender(), *packet);
				}
			}
		}
	}
}
