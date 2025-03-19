#include "Bhop.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"
#include <math.h>
#include "MCPE/SDK/MoveInputHandler.h"
#include "HackSDK.h"

Bhop::Bhop()
{
	addConfig(&speed, "speed");
	addConfig(&rentalGame, "rentalGame");


	ModuleType = "Movement";
	UIType = 0;
	enable_ptr = &enabled;
}

const char* Bhop::GetName()
{
	return "Bhop";
}

uint64_t  (*old_ClientMoveInputHandler_tick)(MoveInputHandler* obj, void* p);
uint64_t now_ClientMoveInputHandler_tick(MoveInputHandler* input, void* p) {
	uint64_t rs = (*old_ClientMoveInputHandler_tick)(input, p);
	Bhop* bhop = moduleManager->getModule<Bhop>();
	if (bhop->enabled) {
		auto player = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();

		vec2_t moveVec2d = { input->forwardMovement, -input->sideMovement };
		bool pressed = moveVec2d.magnitude() > 0.01f;

		if (bhop->rentalGame == false) {

			if (HackSDK::onGroundCheck() && pressed)
				player->jumpFromGround();

			float calcYaw = (player->rot.yaw + 90) * (PI / 180);
			vec3_t moveVec;
			float c = cos(calcYaw);
			float s = sin(calcYaw);
			moveVec2d = { moveVec2d.x * c - moveVec2d.y * s, moveVec2d.x * s + moveVec2d.y * c };
			moveVec.x = moveVec2d.x * bhop->speed;
			moveVec.y = player->motion.y;
			moveVec.z = moveVec2d.y * bhop->speed;
			if (pressed) {
				player->motion.x = moveVec.x;
				player->motion.y = moveVec.y;
				player->motion.z = moveVec.z;
			}
		}
		else {
			if (pressed) {
				if (input->jumpDown) {
					input->jumpDown = false;
				}
				else {
					input->jumpDown = true;
				}
			}
		}
	}
	return rs;
}

void Bhop::MinecraftInit()
{
	HackSDK::FastHook(mGameData.basePtr.ClientMoveInputHandler_tick, (void*)now_ClientMoveInputHandler_tick, (void**)&old_ClientMoveInputHandler_tick);
}


void Bhop::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".Bhop") {
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
			if (cmd->size() < 4)return;
			moduleManager->executedCMD = true;
			speed = atof((*cmd)[2].c_str());


			if ((*cmd)[3] == "true") {
				rentalGame = true;
			}
			else if ((*cmd)[3] == "false") {
				rentalGame = false;
			}
		}
	}
	
}

void Bhop::initViews() {
	Android::EditText* Bhop_Speed = mAndroid->newEditText();
	UIUtils::updateEditTextData(Bhop_Speed, "Speed");

	Android::TextView* Bhop_Save = mAndroid->newTextView();
	UIUtils::updateTextViewData(Bhop_Save, "Save", "#FF0000", 19);
	Bhop_Save->setOnClickListener([=](Android::View*) {
	
		if (Bhop_Speed->text == "")return;
		mAndroid->Toast("Bhop配置已保存");
		speed = UIUtils::et_getFloat(Bhop_Speed);
	});


	Android::TextView* Bhop_rentalGame = mAndroid->newTextView();
	UIUtils::updateTextViewData(Bhop_rentalGame, "rentalGame", "#FF0000", 19);
	Bhop_rentalGame->setOnClickListener([=](Android::View*) {
		if (rentalGame) {
			mAndroid->Toast("Bhop::rentalGame disabled!");
			rentalGame = false;
		}
		else {
			mAndroid->Toast("Bhop::rentalGame enabled!");
			rentalGame = true;
		}
		});

	SecondWindowList.push_back(Bhop_Speed);
	SecondWindowList.push_back(Bhop_Save); 
	SecondWindowList.push_back(Bhop_rentalGame);
}