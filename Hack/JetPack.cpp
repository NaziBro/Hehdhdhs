#include "JetPack.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include <math.h>

JetPack::JetPack()
{
	addConfig(&val, "val");

	ModuleType = "Movement";

	UIType = 0;
	enable_ptr = &enabled;
}

const char* JetPack::GetName()
{
	return "JetPack";
}



void JetPack::OnTick(Actor* act){
	if (enabled == false || mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer() != act)return;

	float yaw = act->rot.yaw * 3.1415926f / 180.0f;
	float pitch = act->rot.pitch * 3.1415926f / 180.0f;
	float xx = -sinf(yaw) * cosf(pitch);
	float yy = -sinf(pitch);
	float zz = cosf(yaw) * cosf(pitch);
	act->motion.x = xx * val;
	act->motion.y = yy * val;
	act->motion.z = zz * val;
}

void JetPack::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".JetPack") {
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
			if (cmd->size() < 3)return;
			moduleManager->executedCMD = true;
			val = atof((*cmd)[2].c_str());
		}
	}
	
}

void JetPack::initViews() {
	Android::EditText* JetPack_Speed = mAndroid->newEditText();
	UIUtils::updateEditTextData(JetPack_Speed, "Speed");

	Android::TextView* JetPack_Save = mAndroid->newTextView();
	UIUtils::updateTextViewData(JetPack_Save, "Save", "#FF0000", 19);
	JetPack_Save->setOnClickListener([=](Android::View*) {
	
		if (JetPack_Speed->text == "")return;
		mAndroid->Toast("JetPack配置已保存");
		val = UIUtils::et_getFloat(JetPack_Speed);
	});

	SecondWindowList.push_back(JetPack_Speed);
	SecondWindowList.push_back(JetPack_Save);
}