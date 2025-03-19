#include "Scaffold.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"

Scaffold::Scaffold()
{
	ModuleType = "Movement";

	UIType = 0;
	enable_ptr = &enabled;
}

const char* Scaffold::GetName()
{
	return "Scaffold";
}

void Scaffold::OnTick(Actor* act)
{

	if (enabled == false)return;

	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	
	if (act != lp)return;
	if (lp->getSelectedItem() == nullptr)return;
	if (lp->getSelectedItem()->count() == 0)return;
	if (lp->getRegion() == nullptr)return;

	
	vec3_t vel(lp->motion.x, lp->motion.y, lp->motion.z);
	float speed = vel.magnitudexz();
	vel = vel.normalize();

	Vec3 blockBelow(lp->getPos()->x,lp->PosData.from.y - 0.5f,lp->getPos()->z);
	
	if (!HackSDK::TryBuildBlock(blockBelow)) {
		if (speed > 0.05f) {  // Are we actually walking?
			blockBelow.z -= vel.z * 0.4f;
			if (!HackSDK::TryBuildBlock(blockBelow)) {
				blockBelow.x -= vel.x * 0.4f;
				if (!HackSDK::TryBuildBlock(blockBelow) && speed > 0.9f) {
					blockBelow.z += vel.z;
					blockBelow.x += vel.x;
					HackSDK::TryBuildBlock(blockBelow);
				}
			}
		}
	}
}



void Scaffold::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".Scaffold") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			enabled = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			enabled = false;
		}
	}
}
