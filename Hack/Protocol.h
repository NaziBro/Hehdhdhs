#pragma once

#include <vector>
#include <string>
#include <stdio.h>
#include "json/json.h"

#include "MCPE/types.h"
#include "MCPE/SDK/mce.h"



class BinaryWriter {
public:

	std::string data;

	void writeByte(uint8_t b);
	void writeSignedShort(int16_t s);
	void writeUnsignedShort(uint16_t s);
	void writeSignedBigEndianInt(int i);
	void writeSignedInt(int i);
	void writeUnsignedInt(uint32_t i);
	void writeSignedInt64(int64_t i);
	void writeUnsignedInt64(uint64_t i);
	void writeDouble(double d);
	void writeFloat(float f);
	void writeUnsignedVarInt(uint32_t i);
	void writeUnsignedVarInt64(uint64_t i);
	void writeString(std::string const& str);
	void writeVarInt(int i);
	void writeVarInt64(int64_t i);

	void writeVec3(Vec3 const& pos);
	void writeUUID(mce::UUID const& uuid);
	void appendData(std::string const& data);
};

class BinaryPacket:public BinaryWriter {
public:
	BinaryPacket(uint32_t id);

};

class BinaryReader {
	uint8_t readByte();
	void read(char* dst, int size);
	int index = 0;
public:
	BinaryReader(std::string const* ptr);
	BinaryReader();

	std::string const* ptr_data = nullptr;

	uint8_t getByte();
	std::string getString();
	int16_t getSignedShort();
	uint16_t getUnsignedShort();
	int getSignedBigEndianInt();
	int getSignedInt();
	uint32_t getUnsignedInt();
	int64_t getSignedInt64();
	uint64_t getUnsignedInt64();
	double getDouble();
	float getFloat();
	uint32_t getUnsignedVarInt();
	uint64_t getUnsignedVarInt64();
	int getVarInt();
	int64_t getVarInt64();
	Vec3 getVec3();
	mce::UUID getUUID();

	std::string getExtra();
	std::string get(uint64_t size);
	std::string getByteArray();

};

class BinaryPacketReader :public BinaryReader{
private:
	uint32_t id;

public:
	BinaryPacketReader(std::string const* data);


	uint32_t getId();
};

class OriginalPacket {
private:
	uint32_t packet_id;
public:
	OriginalPacket(uint32_t id);
	virtual BinaryPacket write() const;
	virtual bool read(BinaryPacketReader & reader);
	bool readFromString(std::string const* data);
	void dumpPacket(std::string const& path);
};

class SettingsCommand :public OriginalPacket {
public:
	static uint32_t pid;
	std::string cmd;
	bool b;
	SettingsCommand(std::string const& cmd, bool b);
	virtual BinaryPacket write() const override;
	virtual bool read(BinaryPacketReader& reader) override;
};

class MobEffect :public OriginalPacket {
public:
	static uint32_t pid;
	uint64_t eid;
	uint8_t type;
	int effectId, amplifier;
	bool particles;
	int duration;


	MobEffect(uint64_t eid, uint8_t type, int effectId, int amplifier, bool particles, int duration);
	virtual BinaryPacket write() const override;
};
class CommandBlockUpdate :public OriginalPacket {
public:
	static uint32_t pid;

	int x, y, z,mode, TickDelay;
	bool redstone, isConditional, TrackOutput, ExecuteOnFirstTick;
	std::string command, LastOutput, CustomName;
	CommandBlockUpdate(int x,int y,int z,int mode,int TickDelay,bool redstone,bool isConditional,bool TrackOutput,bool ExecuteOnFirstTick, std::string  const& command, std::string  const& LastOutput, std::string  const& CustomName);
	virtual BinaryPacket write() const override;
};

class RequestChunkRadius :public OriginalPacket {
public:
	static uint32_t pid;
	int radius;
	RequestChunkRadius(int radius);
	virtual BinaryPacket write() const override;
	virtual bool read(BinaryPacketReader& reader) override;

};
class Text :public OriginalPacket {
public:
	static uint32_t pid;
	uint8_t type;
	std::string source;
	std::string message;
	std::vector<std::string> parameters;
	bool isLocalized;
	std::string xboxUserId = "";
	std::string platformChatId = "";
	std::string packet_end;

	Text();
	Text(uint8_t type, std::string const& source, std::string const& message, std::vector<std::string> const& parameters, bool isLocalized, std::string const& xboxUserId, std::string const& platformChatId);
	virtual BinaryPacket write() const override;
	virtual bool read(BinaryPacketReader& reader) override;

	static Text createChat(std::string const& source, std::string const& message, std::string const& xboxUserId, std::string const& platformChatId);
};

class PyRpc :public OriginalPacket {
public:
	static uint32_t pid;
	std::string  data;
	uint32_t unknown;
	PyRpc(std::string const& s,uint32_t u = 98247598);
	PyRpc();
	virtual BinaryPacket write() const override;
	virtual bool read(BinaryPacketReader& reader) override;

};

class ActorEvent :public OriginalPacket {
public:
	static uint32_t pid;
	uint64_t eid;
	uint8_t event; 
	int data;
	ActorEvent(uint64_t eid,uint8_t event,int data);
	virtual BinaryPacket write() const override;
};

class LevelSoundEvent :public OriginalPacket {
public:
	static uint32_t pid;
	int sound;
	Vec3 pos;
	int extraData;
	std::string entityIdentifier;
	bool isBabyMob;
	bool isGlobal;
	LevelSoundEvent();
	LevelSoundEvent(int sound, Vec3 const& pos, int extraData, std::string const& entityIdentifier, bool isBabyMob, bool isGlobal);

	virtual BinaryPacket write() const override;
	virtual bool read(BinaryPacketReader& reader) override;
};


class LevelSoundEventV1 :public OriginalPacket {
public:
	static uint32_t pid;
	LevelSoundEventV1();
	LevelSoundEventV1(uint8_t sound, Vec3 const& pos, int extraData, int pitch, bool isBabyMob, bool isGlobal);
	uint8_t sound;
	Vec3 pos;
	int extraData = -1;
	int pitch = 1;
	bool isBabyMob;
	bool isGlobal;
	virtual BinaryPacket write() const override;
	virtual bool read(BinaryPacketReader& reader) override;
};

class LevelSoundEventV2 :public OriginalPacket {
public:
	static uint32_t pid;
	uint8_t sound;
	Vec3 pos;
	int extraData;
	std::string entityIdentifier;
	bool isBabyMob;
	bool isGlobal;
	LevelSoundEventV2();
	LevelSoundEventV2(uint8_t sound, Vec3 const& pos, int extraData, std::string const& entityIdentifier, bool isBabyMob, bool isGlobal);

	virtual BinaryPacket write() const override;
	virtual bool read(BinaryPacketReader& reader) override;
};

enum CommandOrigin {
	CommandOriginPlayer = 0,
	CommandOriginBlock,
	CommandOriginMinecartBlock,
	CommandOriginDevConsole,
	CommandOriginTest,
	CommandOriginAutomationPlayer,
	CommandOriginClientAutomation,
	CommandOriginDedicatedServer,
	CommandOriginEntity,
	CommandOriginVirtual,
	CommandOriginGameArgument,
	CommandOriginEntityServer,
	CommandOriginPrecompiled,
	CommandOriginGameDirectorEntityServer,
	CommandOriginScript,
	CommandOriginExecutor
};

class CommandOutput :public OriginalPacket {
public:
	class CommandMessage {
	public:
		bool success;
		std::string message;
		std::vector<std::string> Parameters;
		void read(BinaryReader& reader);
	};
	static uint32_t pid;
	CommandOrigin Origin;
	mce::UUID uuid;
	std::string requestId;
	uint64_t playerUniqueId = 0;
	uint8_t OutputType;
	uint32_t successCount;
	std::vector<CommandMessage> CommandOutputMessage;
	std::string dataSet;

	CommandOutput();
	virtual bool read(BinaryPacketReader& reader) override;
};

class CommandRequest :public OriginalPacket {
public:
	static uint32_t pid;
	mce::UUID uuid;
	std::string cmdline;
	CommandOrigin Origin;
	uint64_t playerUniqueID;
	bool UnLimited;
	bool Internal;
	CommandRequest(mce::UUID const& uuid, std::string const& str, CommandOrigin Origin, uint64_t playerUniqueID,bool Internal,bool UnLimited);
	virtual BinaryPacket write() const override;
};

class ContainerOpen :public OriginalPacket {
public:
	static uint32_t pid;
	uint8_t	 windowId;
	uint8_t type;
	int x;
	int y;
	int z;
	uint64_t entityId;
	ContainerOpen();
	virtual bool read(BinaryPacketReader& reader) override;
};

class Login :public OriginalPacket {
public:
	static uint32_t pid;
	int protocol;
	Json::Value chainData;
	Json::Value clientData;

	Login();
	virtual bool read(BinaryPacketReader& reader) override;
	std::vector<Json::Value> decodeToken(std::string const& token) const;
	std::string getServerIP();
	std::string getPlayerName();
};

class NeteaseJson :public OriginalPacket {
public:

	static uint32_t pid;
	Json::Value data;
	NeteaseJson(Json::Value const& d);
	NeteaseJson();

	virtual BinaryPacket write() const override;
	virtual bool read(BinaryPacketReader& reader) override;

};
