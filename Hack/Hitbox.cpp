#include "Hitbox.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"

Hitbox::Hitbox()
{
	addConfig(&r, "r");
	addConfig(&distance, "distance");
	addConfig(&PlayerMode, "PlayerMode");

	ModuleType = "PVP";
	UIType = 0;
	enable_ptr = &enabled;
}

const char* Hitbox::GetName()
{
	return "Hitbox";
}

void Hitbox::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".Hitbox") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			enabled = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			enabled = false;
		}
		else if ((*cmd)[1] == "config") {
			if (cmd->size() < 5)return;
			moduleManager->executedCMD = true;
			r = atof((*cmd)[2].c_str());
			distance = atof((*cmd)[3].c_str());
			if ((*cmd)[4] == "player") {
				PlayerMode = true;
			}
			else if ((*cmd)[4] == "mob") {
				PlayerMode = false;
			}
		}
	}
}


void Hitbox::initViews() {
	

	auto Hitbox_r = mAndroid->newEditText();
	UIUtils::updateEditTextData(Hitbox_r, "Range");

	auto Hitbox_distance = mAndroid->newEditText();
	UIUtils::updateEditTextData(Hitbox_distance, "Distance");
	
	auto Hitbox_PlayerMode = mAndroid->newEditText();
	UIUtils::updateEditTextData(Hitbox_PlayerMode, "PlayerMode");

	auto Hitbox_save = mAndroid->newTextView();
	UIUtils::updateTextViewData(Hitbox_save, "Save", "#FF0000", 19);
	Hitbox_save->setOnClickListener([=](Android::View*) {
		if (Hitbox_r->text == "" || Hitbox_distance->text == "" || Hitbox_PlayerMode->text == "")return;
		mAndroid->Toast("Hitbox配置已保存");
		r = UIUtils::et_getFloat(Hitbox_r);
		distance = UIUtils::et_getFloat(Hitbox_distance);
		PlayerMode = UIUtils::et_getBool(Hitbox_PlayerMode);
	});

	SecondWindowList.push_back(Hitbox_r);
	SecondWindowList.push_back(Hitbox_distance);
	SecondWindowList.push_back(Hitbox_PlayerMode);
	SecondWindowList.push_back(Hitbox_save);
}

void Hitbox::OnTick(Actor* obj)
{
	if (!enabled)return;
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if (obj != lp)return;
	auto list = HackSDK::getActorList();
	for (auto i : list) {
		if (i == lp)continue;
		if (i->distanceTo(*lp)) {
			if (PlayerMode) {
				if (ActorClassTree::isInstanceOf(*i, 63)) {
					i->weidth = r;
					i->height = r;
				}
			}
			else {
				i->weidth = r;
				i->height = r;
			}
		}
	}
}

