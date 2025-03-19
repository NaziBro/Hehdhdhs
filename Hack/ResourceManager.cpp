#include "ResourceManager.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "MCPE/SDK/Resource.h"
#include "Utils.hpp"
#include "HackSDK.h"
#include "Zip/unzip.h"
#include <thread>
#include <memory>
#include "MCPE/SDK/mce.h"


void addResource(std::string const& path,ResourcePackStack* _ResourcePackStack) {


	std::string ManifestPath = path + "/manifest.json";
	if (access(ManifestPath.c_str(), F_OK) != 0) {
		ManifestPath = path + "/pack_manifest.json";
	}
	Json::Reader reader;
	Json::Value value;
	reader.parse(Utils::readFileIntoString(ManifestPath.c_str()), value);
	std::string uuid_str = value["header"]["uuid"].asString();
	mce::UUID uuid = mce::UUID::fromString(uuid_str);

	ResourcePackRepository* mResourcePackRepository = mGameData.getNowMinecraftGame()->getResourcePackRepository();

	PackSettingsFactory* mPackSettingsFactory = mResourcePackRepository->getPackSettingsFactory();
	ResourcePack* pack = mResourcePackRepository->getResourcePackByUUID(uuid);
	if (pack) {
		PackManifest* mPackManifest = pack->getManifest();
		PackSettings* mPackSettings = mPackSettingsFactory->getPackSettings(*mPackManifest);

		PackInstance instance(pack, -1, false, mPackSettings);

		_ResourcePackStack->add(instance, *mResourcePackRepository, false);
	}
}


uint64_t (*old_ResourcePackManager_setStack)(void* obj, std::unique_ptr<ResourcePackStack>, int ResourcePackStackType, bool b);
uint64_t now_ResourcePackManager_setStack(void* obj, std::unique_ptr<ResourcePackStack> _ResourcePackStack, int ResourcePackStackType, bool b) {

		moduleManager->getModule<ResourceManager>()->mtx.lock();
		for (auto i : moduleManager->getModule<ResourceManager>()->resourceList) {
			addResource(i, _ResourcePackStack.get());
		}
		moduleManager->getModule<ResourceManager>()->mtx.unlock();
		return (*old_ResourcePackManager_setStack)(obj, std::move(_ResourcePackStack), ResourcePackStackType, b);
	
}


ResourceManager::ResourceManager()
{
	ModuleType = "World";
	UIType = 1;
	reloadResourceInfo();
}


void ResourceManager::reloadResourceInfo()
{
	resourceList.clear();
	auto value = getResourceInfo();
	if (value.isMember("PackList")) {
		for (auto i : value["PackList"]) {
			resourceList.push_back(i["CachePath"].asString());
		}
	}
}

std::string ResourceManager::getResourceInfoPath()
{
	std::stringstream path;
	path << "/data/data/" << mGameData.getPackageName() << "/resource.json";
	return path.str();
}

Json::Value ResourceManager::getResourceInfo()
{
	Json::Reader reader;
	Json::Value value;
	reader.parse(Utils::readFileIntoString(getResourceInfoPath().c_str()), value);
	return value;
}

void ResourceManager::saveResourceInfo(Json::Value const& value)
{
	remove(getResourceInfoPath().c_str());
	Utils::WriteStringToFile(getResourceInfoPath().c_str(), value.toStyledString());
}

void ResourceManager::addPack(std::string zipPath)
{
	auto sv = Utils::split(zipPath, "/");
	std::string fileName = sv[sv.size() - 1];
	sv = Utils::split(fileName, ".");
	fileName = sv[0];
	std::stringstream PackPath;
	PackPath << "/data/data/" << mGameData.getPackageName() << "/files/games/com.netease/resource_packs/" << fileName;
	if (access(PackPath.str().c_str(), F_OK) == 0) {
		Utils::DeleteFile(PackPath.str().c_str());
	}
	Utils::folder_mkdirs(PackPath.str().c_str());
	HZIP hz = OpenZip(zipPath.c_str(), NULL);
	SetUnzipBaseDir(hz, PackPath.str().c_str());
	ZIPENTRY ze;
	GetZipItem(hz, -1, &ze);
	int numitems = ze.index;
	for (int zi = 0; zi < numitems; zi++)
	{
		ZIPENTRY ze;
		GetZipItem(hz, zi, &ze);
		UnzipItem(hz, zi, ze.name);
	}
	CloseZip(hz);
	mtx.lock();
	resourceList.push_back(PackPath.str());
	mtx.unlock();

	bool f = false;
	auto v = getResourceInfo();
	for (auto i : v["PackList"]) {
		if (i["ZipPath"].asString() == zipPath) {
			f = true;
			break;
		}
	}
	if (f == false) {
		Json::Value info;
		info["ZipPath"] = zipPath;
		info["CachePath"] = PackPath.str();
		v["PackList"].append(info);
	}

	saveResourceInfo(v);
}

void ResourceManager::removePack(std::string zipPath)
{
	auto v = getResourceInfo();
	Json::Value now;
	for (auto i : v["PackList"]) {
		if (i["ZipPath"].asString() != zipPath) {
			now["PackList"].append(i);
		}
		else {
			Utils::DeleteFile(now["CachePath"].asString().c_str());
		}
	}

	saveResourceInfo(v);
}

const char* ResourceManager::GetName()
{
	return "ResourceManager";
}


void ResourceManager::MinecraftInit()
{
	HackSDK::FastHook(mGameData.basePtr.ResourcePackManager_setStack, (void*)now_ResourcePackManager_setStack, (void**)&old_ResourcePackManager_setStack);
}

void ResourceManager::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".ImportResource") {
		if (cmd->size() < 2)return;
		moduleManager->executedCMD = true;
		std::thread thread([=]() {
			addPack((*cmd)[1]);
		});
		thread.detach();
	}
	else if ((*cmd)[0] == ".RemoveResource") {
		if (cmd->size() < 2)return;
		moduleManager->executedCMD = true;
		removePack((*cmd)[1]);
	}


}


void ResourceManager::initViews() {


	Android::EditText* Resource_path = mAndroid->newEditText();
	UIUtils::updateEditTextData(Resource_path, "ResourcePath");

	Android::TextView* Resource_add = mAndroid->newTextView();
	UIUtils::updateTextViewData(Resource_add, "addPack", "#FF0000", 19);
	Resource_add->setOnClickListener([=](Android::View*) {
		std::thread thread([=]() {
			addPack(Resource_path->text);
			mAndroid->Toast("材质包已导入");
		});
		thread.detach();
	});


	Android::TextView* Resource_remove = mAndroid->newTextView();
	UIUtils::updateTextViewData(Resource_remove, "removePack", "#FF0000", 19);
	Resource_remove->setOnClickListener([=](Android::View*) {
		removePack(Resource_path->text);
		mAndroid->Toast("材质包已移除");

	});

	SecondWindowList.push_back(Resource_path);
	SecondWindowList.push_back(Resource_add);
	SecondWindowList.push_back(Resource_remove);

}