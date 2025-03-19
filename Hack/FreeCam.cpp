#include "FreeCam.h"
#include "MCPE/GameData.h"
#include "HackSDK.h"

void FreeCam::savePos()
{
	saved = *mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getPos();
}

void FreeCam::back()
{
	auto lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	lp->getAbilities()->setAbility(Abilities::flying, false);
	lp->getAbilities()->setAbility(Abilities::noclip, false);
	lp->teleportTo(saved, true, 0, 1);
}

FreeCam::FreeCam()
{
	ModuleType = "Render";

	UIType = 2;
	MenuCall = ([=]() {
		if (mGameData.getNowMinecraftGame()->isInGame()) {
			if (enabled) {
				enabled = false;
				mAndroid->Toast("FreeCam disabled");
				moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
					back();
				});
			}
			else {
				enabled = true;
				mAndroid->Toast("FreeCam enabled");
				moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
					savePos();
				});
			}
		}
	});
	enable_ptr = &enabled;
}

const char* FreeCam::GetName()
{
	return "FreeCam";
}




void FreeCam::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".FreeCam") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			enabled = true;
			savePos();
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			enabled = false;
			back();
		}
	}
}

void FreeCam::OnNewGame(std::string const& ip)
{
	enabled = false;
}

void FreeCam::OnSendPacket(Packet* packet)
{
	if (enabled) {
		if (packet->getName() == "PlayerAuthInputPacket" || packet->getName() == "MovePlayerPacket") {
			MCHook::noPacketThisTime = true;
		}
	}
}

void FreeCam::OnTick(Actor* act)
{
	if (enabled) {
		auto lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
		if (lp == act) {
			lp->getAbilities()->setAbility(Abilities::flying, true);
			lp->getAbilities()->setAbility(Abilities::noclip, true);
		}
	}
}

