#include "Utils.hpp"
#include "Music.h"
#include "MCPE/GameData.h"
#include "HackSDK.h"


class RecordEvent {
public:
	std::string savePath;
	int tickTime = 0;
	Json::Value json;

	RecordEvent(std::string const& path) {
		savePath = path;
	}
	void tick() {
		tickTime = tickTime + 1;
	}
	void add(LevelSoundEvent const& packet) {
		Json::Value obj;
		obj["a"] = packet.sound;
		obj["b"] = packet.extraData;
		obj["c"] = packet.entityIdentifier;
		obj["d"] = packet.isBabyMob;
		obj["t"] = tickTime;
		obj["ver"] = "v0";
		json.append(obj);
	}
	void add(LevelSoundEventV1 const& packet) {
		Json::Value obj;
		obj["a"] = packet.sound;
		obj["b"] = packet.extraData;
		obj["c"] = packet.pitch;
		obj["d"] = packet.isBabyMob;
		obj["t"] = tickTime;
		obj["ver"] = "v1";
		json.append(obj);
	}
	void add(LevelSoundEventV2 const& packet) {
		Json::Value obj;
		obj["a"] = packet.sound;
		obj["b"] = packet.extraData;
		obj["c"] = packet.entityIdentifier;
		obj["d"] = packet.isBabyMob;
		obj["t"] = tickTime;
		obj["ver"] = "v2";
		json.append(obj);
	}
	void save() {
		std::string jsonstr = json.toStyledString();
		FILE* fp = fopen(savePath.c_str(), "wb+");
		fwrite(jsonstr.c_str(), 1, jsonstr.size(), fp);
		fclose(fp);
	}
};

class PlayEvent {
public:
	int tickTime = 0;
	Json::Value json;
	int maxTicks;
	bool loop = false;
	std::string source;

	void setSpeed(float speed) {
		for (int i = 0; i != json.size(); i = i + 1) {
			float m = (float)json[i]["ot"].asInt();
			m = m / speed;
			json[i]["t"] = (int)(roundf(m));
		}
		for (int i = 0; i != json.size(); i = i + 1) {
			float m = (float)json[i]["ot"].asInt();
			m = m / speed;
			json[i]["t"] = (int)(roundf(m));
		}
		for (int i = 0; i != json.size(); i = i + 1) {
			float m = (float)json[i]["ot"].asInt();
			m = m / speed;
			json[i]["t"] = (int)(roundf(m));
		}
		tickTime = roundf((float)tickTime / speed);
		maxTicks = json[json.size() - 1]["t"].asInt();
	}
	PlayEvent(std::string const& path,float speed) {
		source = path;
		Json::Reader reader;
		reader.parse(Utils::readFileIntoString(path.c_str()), json);

		for (int i = 0; i != json.size();i = i + 1) {
		
			json[i]["ot"] = json[i]["t"].asInt();
		}

		setSpeed(speed);
	}
	
	void tick() {
		LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
		for (auto i : json) {
			if (i["t"].asInt() == tickTime) {
				if (i["ver"] == "v0") {
					HackSDK::sendOriginalPacket(LevelSoundEvent(i["a"].asInt(), *lp->getPos(), i["b"].asInt(), i["c"].asString(), i["d"].asBool(), true));
				}
				else if (i["ver"] == "v1") {
					HackSDK::sendOriginalPacket(LevelSoundEventV1(i["a"].asUInt(), *lp->getPos(), i["b"].asInt(), i["c"].asInt(), i["d"].asBool(), true));
				}
				else if (i["ver"] == "v2") {
					HackSDK::sendOriginalPacket(LevelSoundEventV2(i["a"].asUInt(), *lp->getPos(), i["b"].asInt(), i["c"].asString(), i["d"].asBool(), true));
				}
			}
		}
		tickTime = tickTime + 1;
	}

	bool end() {
		if (tickTime > maxTicks) {
			return true;
		}
		else {
			return false;
		}
	}
	
	void restart() {
		tickTime = 0;
	}
};

Music::Music()
{
	addConfig(&playSpeed, "playSpeed");

	ModuleType = "World";
	UIType = 1;
}

const char* Music::GetName()
{
	return "Music";
}


void Music::OnCmd(std::vector<std::string>* cmd)
{

	if ((*cmd)[0] == ".StartRecordMusic") {
		if (cmd->size() > 1) {
			startRecord((*cmd)[1]);
			moduleManager->executedCMD = true;
		}
	}
	else if ((*cmd)[0] == ".StopRecordMusic") {
		stopRecord();
		moduleManager->executedCMD = true;
	}
	else if ((*cmd)[0] == ".PlayMusic") {
		if (cmd->size() > 1) {
			playMusic((*cmd)[1]);
			moduleManager->executedCMD = true;
		}
	}
	else if ((*cmd)[0] == ".StartLoopMusic") {
		if (cmd->size() > 1) {
			startLoopMusic((*cmd)[1]);
			moduleManager->executedCMD = true;
		}
	}
	else if ((*cmd)[0] == ".StopLoopMusic") {
		if (cmd->size() > 1) {
			stopLoopMusic((*cmd)[1]);
			moduleManager->executedCMD = true;
		}
	}
	else if ((*cmd)[0] == ".MusicPlaySpeed") {
		if (cmd->size() > 1) {
			playSpeed = atof((*cmd)[1].c_str());
			updatePlaySpeed();
			moduleManager->executedCMD = true;
		}
	}
}

void Music::OnTick(Actor* act)
{
	if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer() != act)return;
	mtx.lock();
	if (record) {
		record->tick();
	}
	for (auto it = play_list.begin(); it != play_list.end();) {
		if ((*it)->end()) {
			if ((*it)->loop) {
				(*it)->restart();
			}
			else {
				play_list.erase(it);
				delete *it;
				continue;
			}
		}		
		(*it)->tick();
		it = it + 1;
		
	}
	mtx.unlock();
}

void Music::initViews()
{

	Android::EditText* Path = mAndroid->newEditText();
	UIUtils::updateEditTextData(Path, "Path");


	Android::TextView* StartRecordMusic = mAndroid->newTextView();
	UIUtils::updateTextViewData(StartRecordMusic, "StartRecordMusic", "#FF0000", 19);
	StartRecordMusic->setOnClickListener([=](Android::View*) {
		if (startRecord(Path->text)) {
			mAndroid->Toast("开始录制音乐!");
		}
	});

	Android::TextView* StopRecordMusic = mAndroid->newTextView();
	UIUtils::updateTextViewData(StopRecordMusic, "StopRecordMusic", "#FF0000", 19);
	StopRecordMusic->setOnClickListener([=](Android::View*) {
		stopRecord();
		mAndroid->Toast("停止录制音乐!");
	});


	Android::TextView* PlayMusic = mAndroid->newTextView();
	UIUtils::updateTextViewData(PlayMusic, "PlayMusic", "#FF0000", 19);
	PlayMusic->setOnClickListener([=](Android::View*) {
		playMusic(Path->text);
		mAndroid->Toast("开始播放音乐!");
		});


	Android::TextView* LoopMusic = mAndroid->newTextView();
	UIUtils::updateTextViewData(LoopMusic, "StartLoopMusic", "#FF0000", 19);
	LoopMusic->setOnClickListener([=](Android::View*) {
		startLoopMusic(Path->text);
		mAndroid->Toast("开始循环播放音乐!");
		});

	Android::TextView* StopLoopMusic = mAndroid->newTextView();
	UIUtils::updateTextViewData(StopLoopMusic, "StopLoopMusic", "#FF0000", 19);
	StopLoopMusic->setOnClickListener([=](Android::View*) {
		stopLoopMusic(Path->text);
		mAndroid->Toast("停止循环播放音乐!");
		});


	Android::EditText* PlaySpeed = mAndroid->newEditText();
	UIUtils::updateEditTextData(PlaySpeed, "PlaySpeed");

	Android::TextView* setPlaySpeed = mAndroid->newTextView();
	UIUtils::updateTextViewData(setPlaySpeed, "SetPlaySpeed", "#FF0000", 19);
	setPlaySpeed->setOnClickListener([=](Android::View*) {
		playSpeed = UIUtils::et_getFloat(PlaySpeed);
		updatePlaySpeed();
		mAndroid->Toast("修改音乐播放倍速成功!");
		});


	SecondWindowList.push_back(Path);
	SecondWindowList.push_back(StartRecordMusic);
	SecondWindowList.push_back(StopRecordMusic);
	SecondWindowList.push_back(PlayMusic);
	SecondWindowList.push_back(LoopMusic);
	SecondWindowList.push_back(StopLoopMusic);
	SecondWindowList.push_back(PlaySpeed);
	SecondWindowList.push_back(setPlaySpeed);
}

void Music::OnReceiveData(std::string* data)
{
	if (record) {
		LevelSoundEvent packet;
		BinaryPacketReader reader(data);
		if (packet.read(reader)) {
			mtx.lock();
			record->add(packet);
			mtx.unlock();
			return;
		}
		LevelSoundEventV1 packet1;
		BinaryPacketReader reader1(data);
		if (packet1.read(reader1)) {
			mtx.lock();
			record->add(packet1);
			mtx.unlock();
			return;
		}
		LevelSoundEventV2 packet2;
		BinaryPacketReader reader2(data);
		if (packet1.read(reader2)) {
			mtx.lock();
			record->add(packet2);
			mtx.unlock();
			return;
		}
	}
}

bool Music::startRecord(std::string const& path)
{
	if (record)return false;
	mtx.lock();
	record = new RecordEvent(path);
	mtx.unlock();
	return true;
}

void Music::stopRecord()
{

	mtx.lock();
	if (record) {
		record->save();
		delete record;
		record = nullptr;
	}
	mtx.unlock();
}

void Music::playMusic(std::string const& path)
{
	mtx.lock();
	PlayEvent* play = new PlayEvent(path,playSpeed);
	play_list.push_back(play);
	mtx.unlock();
}

void Music::startLoopMusic(std::string const& path)
{

	mtx.lock();
	PlayEvent* play = new PlayEvent(path,playSpeed);
	play->loop = true;
	play_list.push_back(play);
	mtx.unlock();
}


void Music::stopLoopMusic(std::string const& path)
{
	mtx.lock();
	for (auto it = play_list.begin(); it != play_list.end(); it = it + 1) {
		auto music = *it;
		if (music->loop && music->source == path) {
			play_list.erase(it);
			it = it - 1;
			delete music;
		}
	}
	mtx.unlock();
}

void Music::updatePlaySpeed()
{
	mtx.lock();

	for (auto it = play_list.begin(); it != play_list.end(); it = it + 1) {
		auto music = *it;
		music->setSpeed(playSpeed);
	}
	mtx.unlock();
}