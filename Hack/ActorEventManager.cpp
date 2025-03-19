#include "ActorEventManager.h"
#include "MCPE/GameData.h"
#include "HackSDK.h"
#include "Utils.hpp"

std::string ActorEventManager::int64_len(int64_t t) {
	std::string T;
	std::stringstream os;
	os << t;
	T.push_back((char)os.str().size());
	std::stringstream os1;
	os1 << T << t;
	return os1.str();
}


void ActorEventManager::playAnimation(bool self, std::string const& animation)
{
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if (self) {
		std::string A;
		A.push_back((char)animation.size());
		std::stringstream data;
		data << Utils::hex2string("82 C4 08 5F 5F 74 79 70 65 5F 5F C4 05 74 75 70 6C 65 C4 05 76 61 6C 75 65 93 C4 0B 4D 6F 64 45 76 65 6E 74 43 32 53 82 C4 08 5F 5F 74 79 70 65 5F 5F C4 05 74 75 70 6C 65 C4 05 76 61 6C 75 65 94 C4 09 4D 69 6E 65 63 72 61 66 74 C4 05 65 6D 6F 74 65 C4 0E 50 6C 61 79 45 6D 6F 74 65 45 76 65 6E 74 83 C4 08 70 6C 61 79 65 72 49 64 C4") << int64_len((int64_t)*lp->getUniqueID()) << Utils::hex2string("C4 08 61 6E 69 6D 4E 61 6D 65 C4")<<A<<animation<<Utils::hex2string("C4 04 76 69 65 77 00 C0");
		HackSDK::sendOriginalPacket(PyRpc(data.str()));
	}
	else {

		lp->getDimension()->forEachPlayer([&](Player& object) {
			if (&object == lp)return true;
			std::string A;
			A.push_back((char)animation.size());
			std::stringstream data;
			data << Utils::hex2string("82 C4 08 5F 5F 74 79 70 65 5F 5F C4 05 74 75 70 6C 65 C4 05 76 61 6C 75 65 93 C4 0B 4D 6F 64 45 76 65 6E 74 43 32 53 82 C4 08 5F 5F 74 79 70 65 5F 5F C4 05 74 75 70 6C 65 C4 05 76 61 6C 75 65 94 C4 09 4D 69 6E 65 63 72 61 66 74 C4 05 65 6D 6F 74 65 C4 0E 50 6C 61 79 45 6D 6F 74 65 45 76 65 6E 74 83 C4 08 70 6C 61 79 65 72 49 64 C4") << int64_len((int64_t)*object.getUniqueID()) << Utils::hex2string("C4 08 61 6E 69 6D 4E 61 6D 65 C4") << A << animation << Utils::hex2string("C4 04 76 69 65 77 00 C0");
			HackSDK::sendOriginalPacket(PyRpc(data.str()));
			return true;
		});
	}
}



void ActorEventManager::dimensionBag(int dim)
{
	LocalPlayer* p = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	for (int i = 0; i != 36; i = i + 1) {
		std::stringstream data;
		std::string s1;
		s1.push_back((char)i);
		std::string s2;
		s2.push_back((char)(i + 35 * (dim - 1)));
		data << Utils::hex2string("82 C4 08 5F 5F 74 79 70 65 5F 5F C4 05 74 75 70 6C 65 C4 05 76 61 6C 75 65 93 C4 0B 4D 6F 64 45 76 65 6E 74 43 32 53 82 C4 08 5F 5F 74 79 70 65 5F 5F C4 05 74 75 70 6C 65 C4 05 76 61 6C 75 65 94 C4 09 4D 69 6E 65 63 72 61 66 74 C4 03 70 65 74 C4 11 73 77 61 70 5F 70 65 74 5F 62 61 67 5F 69 74 65 6D 86 C4 08 70 6C 61 79 65 72 49 64 C4") << int64_len(*p->getUniqueID()) << Utils::hex2string("C4 0B 74 61 6B 65 50 65 72 63 65 6E 74 01 C4 08 66 72 6F 6D 53 6C 6F 74") << s1 << Utils::hex2string("C4 06 74 6F 49 74 65 6D C0 C4 06 74 6F 53 6C 6F 74 C4 08 69 74 65 6D 42 74 6E") << s2 << Utils::hex2string("C4 08 66 72 6F 6D 49 74 65 6D C0 C0");
		HackSDK::sendOriginalPacket(PyRpc(data.str()));
	}
	
}



void ActorEventManager::deleteMobs()
{
	mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getDimension()->getEntityIdMap()->forEachEntity([=](Actor& object) {
		std::stringstream data;
		data << Utils::hex2string("82 C4 08 5F 5F 74 79 70 65 5F 5F C4 05 74 75 70 6C 65 C4 05 76 61 6C 75 65 93 C4 0B 4D 6F 64 45 76 65 6E 74 43 32 53 82 C4 08 5F 5F 74 79 70 65 5F 5F C4 05 74 75 70 6C 65 C4 05 76 61 6C 75 65 94 C4 09 4D 69 6E 65 63 72 61 66 74 C4 03 70 65 74 C4 14 64 65 73 74 72 6F 79 5F 70 65 74 73 5F 72 65 71 75 65 73 74 82 C4 0A 65 6E 74 69 74 79 5F 69 64 73 91 C4") << int64_len((int64_t)*object.getUniqueID()) << Utils::hex2string("C4 09 70 6C 61 79 65 72 5F 69 64 C4") << int64_len((int64_t)*mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getUniqueID()) << Utils::hex2string("C0");
		HackSDK::sendOriginalPacket(PyRpc(data.str()));
	});
}

ActorEventManager::ActorEventManager()
{
	ModuleType = "World";
	UIType = 1;

}

const char* ActorEventManager::GetName()
{
	return "ActorEventManager";
}


void ActorEventManager::OnCmd(std::vector<std::string>* cmd)
{
	  if ((*cmd)[0] == ".DeleteMobs") {
		 moduleManager->executedCMD = true;
		 deleteMobs();
	 }
	 else if ((*cmd)[0] == ".PlayAnimation") {
		 if (cmd->size() < 2)return;
		 moduleManager->executedCMD = true;
		 bool self = false;
		 if (cmd->size() >= 3) {
			 if ((*cmd)[2] == "true") {
				 self = true;
			 }
			 else if ((*cmd)[2] == "false") {
				 self = false;
			 }
		 }
		 playAnimation(self, (*cmd)[1]);
	 }
	 else if ((*cmd)[0] == ".DimensionBag") {
		 if (cmd->size() < 2)return;
		 moduleManager->executedCMD = true;
		 dimensionBag(atoi((*cmd)[1].c_str()));
	 }
}

void ActorEventManager::initViews()
{

	Android::TextView* DeleteMobs = mAndroid->newTextView();
	UIUtils::updateTextViewData(DeleteMobs, "DeleteMobs", "#FF0000", 19);
	DeleteMobs->setOnClickListener([=](Android::View*) {
		if (mGameData.getNowMinecraftGame()->isInGame()) {
			mAndroid->Toast("已清除实体!");
			moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
				deleteMobs();
			});
		}
	});


	Android::EditText* Animation_self = mAndroid->newEditText();
	UIUtils::updateEditTextData(Animation_self, "self");

	Android::EditText* Animation_name = mAndroid->newEditText();
	UIUtils::updateEditTextData(Animation_name, "name");

	Android::TextView* Animation = mAndroid->newTextView();
	UIUtils::updateTextViewData(Animation, "PlayAnimation", "#FF0000", 19);
	Animation->setOnClickListener([=](Android::View*) {
		if (mGameData.getNowMinecraftGame()->isInGame() == false)return;
		mAndroid->Toast("播放动作成功");
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			playAnimation(UIUtils::et_getBool(Animation_self), Animation_name->text);
		});
	});
	Android::EditText* DimensionBag_Dimension = mAndroid->newEditText();
	UIUtils::updateEditTextData(DimensionBag_Dimension, "Dimension(1-7)");

	Android::TextView* DimensionBag = mAndroid->newTextView();
	UIUtils::updateTextViewData(DimensionBag, "DimensionBag", "#FF0000", 19);
	DimensionBag->setOnClickListener([=](Android::View*) {
		if (mGameData.getNowMinecraftGame()->isInGame() == false)return;
		if (DimensionBag_Dimension->text == "")return;
		mAndroid->Toast("放置物品成功");
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			dimensionBag(UIUtils::et_getInt(DimensionBag_Dimension));
		});
	});
	SecondWindowList.push_back(DeleteMobs);
	SecondWindowList.push_back(Animation_name);
	SecondWindowList.push_back(Animation_self);
	SecondWindowList.push_back(Animation);
	SecondWindowList.push_back(DimensionBag_Dimension);
	SecondWindowList.push_back(DimensionBag);
}
