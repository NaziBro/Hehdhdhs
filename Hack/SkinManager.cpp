#include "Chat.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include <math.h>
#include "SkinManager.h"
#include "HackSDK.h"
#include "MCPE/SDK/mce.h"
#include "Network.h"
#include "Utils.hpp"
#include "MCPE/Listener.h"
#include "MCPE/SDK/SerializedSkin.h"

SkinManager::SkinManager()
{
	addConfig(&Invisibility, "Invisibility");

	ModuleType = "World";
	UIType = 1;
}

const char* SkinManager::GetName()
{
	return "SkinManager";
}

void SkinManager::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".Invisibility") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			Invisibility = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			Invisibility = false;
		}
	}
	/*
	else if ((*cmd)[0] == ".Skin") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			DIYSkin = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			DIYSkin = false;
		}
	}
	*/
}
void SkinManager::initViews()
{
	Android::TextView* Skin_invisibility = mAndroid->newTextView();
	UIUtils::updateTextViewData(Skin_invisibility, "Invisibility", "#FF0000", 19);
	Skin_invisibility->setOnClickListener([=](Android::View*) {
		if (Invisibility == false) {
			Invisibility = true;
			mAndroid->Toast("租赁服隐身开启");

		}
		else {
			Invisibility = false;
			mAndroid->Toast("租赁服隐身关闭");
		}
	});
	/*
	Android::TextView* Skin_use = mAndroid->newTextView();
	UIUtils::updateTextViewData(Skin_use, "Use", "#FF0000", 19);
	Skin_use->setOnClickListener([=](Android::View*) {
		if (DIYSkin == false) {
			DIYSkin = true;
			mAndroid->Toast("启用自定义皮肤");

		}
		else {
			DIYSkin = false;
			mAndroid->Toast("禁用自定义皮肤");
		}
	});

	Android::EditText* Skin_path = mAndroid->newEditText();
	UIUtils::updateEditTextData(Skin_path, "SkinPath");

	Android::TextView*  Skin_setPath = mAndroid->newTextView();
	UIUtils::updateTextViewData(Skin_setPath, "SetPath", "#FF0000", 19);
	Skin_setPath->setOnClickListener([=](Android::View*) {
		SkinPath = Skin_path->text;
		mAndroid->Toast("皮肤路径设置成功");
	}); 
	*/
	SecondWindowList.push_back(Skin_invisibility);
	/*
	SecondWindowList.push_back(Skin_use);
	SecondWindowList.push_back(Skin_setPath);
	SecondWindowList.push_back(Skin_path);
	*/
}

class ConnectionRequest {
public:
	char f[256];
};

ConnectionRequest(*old_ConnectionRequest_create)(void* PrivateKeyManager, void* Certificate, const std::string& SelfSignedId, const std::string& ServerAddress, uint64_t ClientRandomId, const std::string& SkinId, const mce::Image& SkinData, const mce::Image& CapeData, const mce::Image& BloomData, const std::string& SkinIID, const unsigned int GrowthLevel, const SerializedSkin&, const std::string&, int, int, int, const std::string&, bool, const std::string&, int, const std::string&, const std::string&, bool, const std::string&, const std::string&, const std::string&);

ConnectionRequest  ConnectionRequest_create(void* PrivateKeyManager, void* Certificate, const std::string& SelfSignedId, const std::string& ServerAddress, uint64_t ClientRandomId, const std::string& SkinId, const mce::Image& SkinData, const mce::Image& CapeData, const mce::Image& BloomData, const std::string& SkinIID, const unsigned int GrowthLevel, const SerializedSkin& skin, const std::string& deviceID, int a2, int a3, int a4, const std::string& a5, bool a6, const std::string& a7, int a8, const std::string& a9, const std::string& a10, bool a11, const std::string& a12, const std::string& a13, const std::string& CapeId) {
	SkinManager* skinManager = moduleManager->getModule<SkinManager>();

	if (skinManager->Invisibility == true) {
		Json::Value geoJ;
		srand(time(0));
		std::stringstream os;
		os << rand();
		geoJ["geometry"]["default"] = os.str();
		((SerializedSkin&)skin).SkinResourcePatch = geoJ.toStyledString();
	}
	/*
	else if (skinManager->DIYSkin) {
		std::string SkinJsonPath = skinManager->SkinPath + "/skins.json";
		std::string SkinJsonFile = Utils::readFileIntoString(SkinJsonPath.c_str());
		Json::Reader reader;
		Json::Value SkinJson;
		reader.parse(SkinJsonFile, SkinJson);
		int i = 0;
		if (SkinJson.isMember("SkinIndex")) {
			i = SkinJson["SkinIndex"].asInt();
		}
		Json::Value Skin = SkinJson["skins"][i];
		std::string SkinImagePath = skinManager->SkinPath + "/" + Skin["texture"].asString();
		mce::Image newSkinImage;
		newSkinImage.loadImageFromFile(SkinImagePath);
		Json::Value geoJ;
		geoJ["geometry"]["default"] = Skin["geometry"].asString();
		((SerializedSkin&)skin).SkinResourcePatch = geoJ.toStyledString();



		MCJson::Value* GeometryData = ((SerializedSkin&)skin).getGeometryData();
		std::string oldGeometryData = GeometryData->toStyledString();
		MCJson::Reader mc_reader;
		std::string SkinGeometryPath = skinManager->SkinPath + "/geometry.json";
		mc_reader.parse(Utils::readFileIntoString(SkinGeometryPath.c_str()), *GeometryData, false);
		void* res = (*old_ConnectionRequest_create)(thiz, PrivateKeyManager, Certificate, SelfSignedId, ServerAddress, ClientRandomId, SkinId, newSkinImage, CapeData, BloomData, SkinIID, GrowthLevel, skin, deviceID, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, CapeId);
		mc_reader.parse(SkinGeometryPath, *GeometryData, false);
		return res;
	}
	*/
	return (*old_ConnectionRequest_create)(PrivateKeyManager, Certificate, SelfSignedId, ServerAddress, ClientRandomId, SkinId, SkinData, CapeData, BloomData, SkinIID, GrowthLevel, skin, deviceID, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, CapeId);

}

void SkinManager::MinecraftInit()
{
	HackSDK::FastHook(mGameData.basePtr.ConnectionRequest_create, (void*)ConnectionRequest_create, (void**)&old_ConnectionRequest_create);
}
