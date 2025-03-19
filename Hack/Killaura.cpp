#include "Killaura.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include <thread>
#include "HackSDK.h"

Killaura::Killaura()
{
	ModuleType = "PVP";
	UIType = 0;
	enable_ptr = &enabled;

	addConfig(&cps_boost, "cps_boost");
	addConfig(&Range, "Range");
	addConfig(&PlayerMode, "PlayerMode");
	addConfig(&cps, "cps");
	addConfig(&attackAll, "attackAll");
	addConfig(&antibot, "antibot");
	addConfig(&back, "back");
	addConfig(&spin, "spin");
	addConfig(&up, "up");
	addConfig(&swing, "swing");
}

void Killaura::attackActor(Actor* actor)
{
	auto gm = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->gamemode;
	for (int i = 0; i != cps_boost; i = i + 1) {
		gm->attack(*actor);
	}
}

const char* Killaura::GetName()
{
	return "Killaura";
}



void Killaura::OnTick(Actor* act)
{

	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if (lp!= act)return;

	Player* nearest = nullptr;
	if (enabled) {
		float mindist = Range + 1.5f;
		lp->getDimension()->forEachPlayer([&](Player& object) {
			if (&object != lp && object.getRuntimeID() != lp->getRuntimeID() && object.getUniqueID()!= lp->getUniqueID()) {
				if (antibot) {
					if (HackSDK::isRealPlayer(&object) == false) {
						return true;
					}
				}
				float dist = lp->distanceTo(object);
				if (dist < mindist) {
					mindist = dist;
					nearest = &object;
					
				}

			}
			return true;
		});
		if (nearest) {
			if (back) {
				float x = nearest->getPos()->x + sin(nearest->rot.yaw * PI / 180) * 2;
				float y = nearest->getPos()->y;
				float z = nearest->getPos()->z - cos(nearest->rot.yaw * PI / 180) * 2;
				lp->motion.y = 0;
				Vec3 pos(x, y, z);
				lp->teleportTo(pos, true, 0, 1);
				HackSDK::lookAt(*nearest->getPos());
			}
			else if (spin) {
				lp->motion.y = 0;
				spinAngle = spinAngle + 30;
				if (spinAngle == 360) {
					spinAngle = 0;
				}
				float radius = 3;

				float PH = sinf(spinAngle * PI / 180) * radius;
				float OH = cosf(spinAngle * PI / 180) * radius;
				Vec3 pos(nearest->getPos()->x + OH, nearest->getPos()->y, nearest->getPos()->z + PH);

				lp->teleportTo(pos, true, 0, 1);
				HackSDK::lookAt(*nearest->getPos());
			}
			else if (up) {
				lp->motion.y = 0;
				Vec3 pos(nearest->getPos()->x, nearest->getPos()->y + 4.0f, nearest->getPos()->z);
				lp->teleportTo(pos, true, 0, 1);
				HackSDK::lookAt(*nearest->getPos());
			}
		}
		
	}
	if (enabled) {
		if (attackAll == true) {
			if (handleAttack == false) {
				return;
			}
		}
		if (nearest) {

			if (handleAttack == true) {
				handleAttack = false;
				attackActor(nearest);
				attacked = true;
			}
		}

		Dimension* _dimension = act->getDimension();
		if (_dimension == nullptr)return;
		EntityIdMap* map = _dimension->getEntityIdMap();
		if (map == nullptr)return;
		if (PlayerMode == false) {
			map->forEachEntity([&](Actor& object) {
				if (object.distanceTo(*lp) < Range) {

					if (ActorClassTree::isInstanceOf(object, 256) == true) {
						if (attackAll == false) {
							if (handleAttack == true) {
								attacked = true;
								attackActor(&object);
								handleAttack = false;
							}
						}
						else {
							attacked = true;
							attackActor(&object);
						}
					}
				}
				});
		}

		_dimension->forEachPlayer([&](Player& object) {
			LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
			if (&object == lp) {
				return true;
			}

			if (object.distanceTo(*lp) < Range) {
				if (antibot) {
					if (HackSDK::isRealPlayer(&object) == false)return true;
				}
				if (attackAll == false) {
					if (handleAttack == true) {
						attacked = true;
						attackActor(&object);
						handleAttack = false;
					}
				}
				else {
					attacked = true;
					attackActor(&object);
				}
			}
			return true;
			});

		if (attacked == true) {
			if (swing) {
				mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->swing();
			}
			attacked = false;
			handleAttack = false;
		}
	}
}

void Killaura::MinecraftInit()
{
	std::thread([=]() {
		while (true) {
			if (enabled == true) {
				int sleepTime = 1000000 / cps;
				usleep(sleepTime);
				handleAttack = true;
			}
			else {
				usleep(100000);
			}
		}

	}).detach();
}


void Killaura::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".Killaura") {
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
			if (cmd->size() < 12)return;

			if ((*cmd)[3] == "player") {
				PlayerMode = true;
			}
			else if ((*cmd)[3] == "mob") {
				PlayerMode = false;
			}
			else {
				return;
			}
			Range = atof((*cmd)[2].c_str());
			cps = atof((*cmd)[4].c_str());
			if (cps > 20) {
				mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("[Killaura]CPS is too high,reset 20 cps!", "");
				cps = 20;
			}
			if ((*cmd)[5] == "true") {
				attackAll = true;
			}
			else if ((*cmd)[5] == "false") {
				attackAll = false;
			}
			if ((*cmd)[6] == "true") {
				antibot = true;
			}
			else if ((*cmd)[6] == "false") {
				antibot = false;
			}

			if ((*cmd)[7] == "true") {
				back = true;
			}
			else if ((*cmd)[7] == "false") {
				back = false;
			}


			if ((*cmd)[8] == "true") {
				spin = true;
			}
			else if ((*cmd)[8] == "false") {
				spin = false;
			}
			if ((*cmd)[9] == "true") {
				up = true;
			}
			else if ((*cmd)[9] == "false") {
				up = false;
			}

			if ((*cmd)[10] == "true") {
				swing = true;
			}
			else if ((*cmd)[10] == "false") {
				swing = false;
			}
			cps_boost = atof((*cmd)[11].c_str());
			moduleManager->executedCMD = true;
		}
	}
}


void Killaura::initViews() {
	Android::TextView* Killaura_BindBack = mAndroid->newTextView();
	UIUtils::updateTextViewData(Killaura_BindBack, "BindBack", "#FF0000", 19);
	Killaura_BindBack->setOnClickListener([=](Android::View*) {
		if (back) {
			mAndroid->Toast("Killaura::BindBack disabled!");
			back = false;
		}
		else {
			mAndroid->Toast("Killaura::BindBack enabled!");
			back = true;
		}
		});


	Android::TextView* Killaura_Spin = mAndroid->newTextView();
	UIUtils::updateTextViewData(Killaura_Spin, "Spin", "#FF0000", 19);
	Killaura_Spin->setOnClickListener([=](Android::View*) {
		if (spin) {
			mAndroid->Toast("Killaura::Spin disabled!");
			spin = false;
		}
		else {
			mAndroid->Toast("Killaura::Spin enabled!");
			spin = true;
		}
		});

	Android::TextView* Killaura_Up = mAndroid->newTextView();
	UIUtils::updateTextViewData(Killaura_Up, "Up", "#FF0000", 19);
	Killaura_Up->setOnClickListener([=](Android::View*) {
		if (up) {
			mAndroid->Toast("Killaura::Up disabled!");
			up = false;
		}
		else {
			mAndroid->Toast("Killaura::Up enabled!");
			up = true;
		}
		});

	Android::TextView* Killaura_AntiBot = mAndroid->newTextView();
	UIUtils::updateTextViewData(Killaura_AntiBot, "AntiBot", "#FF0000", 19);
	Killaura_AntiBot->setOnClickListener([=](Android::View*) {
		if (antibot) {
			antibot = false;
			mAndroid->Toast("Killaura::AntiBot disabled");
		}
		else {
			antibot = true;
			mAndroid->Toast("Killaura::AntiBot enabled");
		}
	});

	Android::EditText* Killaura_Range = mAndroid->newEditText();
	UIUtils::updateEditTextData(Killaura_Range, "Range");

	Android::EditText* Killaura_PlayerMode = mAndroid->newEditText();
	UIUtils::updateEditTextData(Killaura_PlayerMode, "PlayerMode");

	Android::EditText* Killaura_CPS = mAndroid->newEditText();
	UIUtils::updateEditTextData(Killaura_CPS, "CPS");

	Android::EditText* Killaura_CPSBOOST = mAndroid->newEditText();
	UIUtils::updateEditTextData(Killaura_CPSBOOST, "CPS_BOOST");

	Android::EditText* Killaura_AttackAll = mAndroid->newEditText();
	UIUtils::updateEditTextData(Killaura_AttackAll, "AttackAll");

	Android::TextView* Killaura_Save = mAndroid->newTextView();
	UIUtils::updateTextViewData(Killaura_Save, "Save", "#FF0000", 19);
	Killaura_Save->setOnClickListener([=](Android::View*) {
		if (Killaura_Range->text == "" || Killaura_PlayerMode->text == "" || Killaura_CPS->text == "" || Killaura_AttackAll->text == "")return;
		mAndroid->Toast("Killaura配置已保存");
		Range = UIUtils::et_getFloat(Killaura_Range);
		PlayerMode = UIUtils::et_getBool(Killaura_PlayerMode);
		cps = UIUtils::et_getInt(Killaura_CPS);
		cps_boost = UIUtils::et_getInt(Killaura_CPSBOOST);
		if (cps > 20) {
			cps = 20;
		}
		attackAll = UIUtils::et_getBool(Killaura_AttackAll);
	});


	Android::TextView* Killaura_Swing = mAndroid->newTextView();
	UIUtils::updateTextViewData(Killaura_Swing, "SwingArm", "#FF0000", 19);
	Killaura_Swing->setOnClickListener([=](Android::View*) {
		if (swing) {
			mAndroid->Toast("Killaura::Swing disabled!");
			swing = false;
		}
		else {
			mAndroid->Toast("Killaura::Swing enabled!");
			swing = true;
		}
		});

	SecondWindowList.push_back(Killaura_BindBack);
	SecondWindowList.push_back(Killaura_Spin);
	SecondWindowList.push_back(Killaura_Up);
	SecondWindowList.push_back(Killaura_AntiBot);
	SecondWindowList.push_back(Killaura_Range);
	SecondWindowList.push_back(Killaura_PlayerMode);
	SecondWindowList.push_back(Killaura_CPS);
	SecondWindowList.push_back(Killaura_CPSBOOST);
	SecondWindowList.push_back(Killaura_AttackAll);
	SecondWindowList.push_back(Killaura_Save);
	SecondWindowList.push_back(Killaura_Swing);
}