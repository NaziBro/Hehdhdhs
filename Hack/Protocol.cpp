#include "Protocol.h"
#include "Utils.hpp"
#include <sstream>

uint32_t SettingsCommand::pid = 140;
uint32_t CommandBlockUpdate::pid = 78;
uint32_t RequestChunkRadius::pid = 0x45;
uint32_t PyRpc::pid = 0xC8;
uint32_t Text::pid = 9;
uint32_t LevelSoundEvent::pid = 0x7B;
uint32_t LevelSoundEventV1::pid = 0x18;
uint32_t LevelSoundEventV2::pid = 0x78;
uint32_t CommandRequest::pid = 0x4D;
uint32_t ContainerOpen::pid = 0x2E;
uint32_t ActorEvent::pid = 0x1B;
uint32_t Login::pid = 0x1;
uint32_t NeteaseJson::pid = 0xCB;
uint32_t CommandOutput::pid = 0x4F;
uint32_t MobEffect::pid = 0x1C;



BinaryPacket::BinaryPacket(uint32_t id)
{
	writeUnsignedVarInt(id);
}

void BinaryWriter::writeByte(uint8_t b)
{
	data.push_back(b);
}

void BinaryWriter::writeSignedShort(int16_t s)
{
	data.append((char*)&s, 2);
}

void BinaryWriter::writeUnsignedShort(uint16_t s)
{
	data.append((char*)&s, 2);
}

void BinaryWriter::writeSignedBigEndianInt(int i)
{
	char* p = (char*)&i;
	data.push_back(p[3]);
	data.push_back(p[2]);
	data.push_back(p[1]);
	data.push_back(p[0]);
}

void BinaryWriter::writeSignedInt(int i)
{
	data.append((char*)&i, 4);
}
void BinaryWriter::writeUnsignedInt(uint32_t i)
{
	data.append((char*)&i, 4);
}

void BinaryWriter::writeSignedInt64(int64_t i)
{
	data.append((char*)&i, 8);
}

void BinaryWriter::writeUnsignedInt64(uint64_t i)
{
	data.append((char*)&i, 8);
}

void BinaryWriter::writeDouble(double d)
{
	data.append((char*)&d, 8);
}

void BinaryWriter::writeFloat(float f)
{
	data.append((char*)&f, 4);
}

void BinaryWriter::writeUnsignedVarInt(uint32_t i)
{
	uint32_t v3 = 0;
	char v4 = 0;
	do
	{
		v3 = i >> 7;
		v4 = i & 0x7F;
		if (i >> 7)
			v4 = i | 0x80;
		data.push_back(v4);
		i = v3;
	} while (v3);
}

void BinaryWriter::writeUnsignedVarInt64(uint64_t i)
{
	uint64_t v3 = 0;
	char v4 = 0;
	do
	{
		v3 = i >> 7;
		v4 = i & 0x7F;
		if (i >> 7)
			v4 = i | 0x80;
		data.push_back(v4);
		i = v3;
	} while (v3);
}

void BinaryWriter::writeString(std::string const& str)
{
	writeUnsignedVarInt(str.size());
	data.append(str);
}

void BinaryWriter::writeVarInt(int i)
{
	unsigned int v3 = 0;
	unsigned int v4 = 0;
	char v5 = 0;
	char v6 = 0;
	v3 = (2 * i) ^ (i >> 31);
	do
	{
		v4 = v3 >> 7;
		v5 = v3 & 0x7F;
		v6 = v3 | 0x80;
		if (!v4)
			v6 = v5;
		data.push_back(v6);
		v3 = v4;
	} while (v4);
}

void BinaryWriter::writeVarInt64(int64_t i)
{
	uint64_t v3 = 0;
	uint64_t v4 = 0;
	char v5 = 0;
	char v6 = 0;
	v3 = (2 * i) ^ (i >> 63);
	do
	{
		v4 = v3 >> 7;
		v5 = v3 & 0x7F;
		v6 = v3 | 0x80;
		if (!v4)
			v6 = v5;
		data.push_back(v6);
		v3 = v4;
	} while (v4);
}

void BinaryWriter::writeVec3(Vec3 const& pos)
{
	writeFloat(pos.x);
	writeFloat(pos.y);
	writeFloat(pos.z);
}

void BinaryWriter::writeUUID(mce::UUID const& uuid)
{
	writeUnsignedInt64(uuid.ab);
	writeUnsignedInt64(uuid.cd);
}

void BinaryWriter::appendData(std::string const& data)
{
	this->data.append(data);
}

uint8_t BinaryReader::readByte()
{
	if (index == ptr_data->size()) {
		return 0;
	}
	char c = (*ptr_data)[index];
	index = index + 1;
	return c;
}

void BinaryReader::read(char* dst, int size)
{
	for (int i = 0; i != size; i = i + 1) {
		dst[i] = readByte();
	}
}

BinaryPacketReader::BinaryPacketReader(std::string const* data)
{
	ptr_data = data;
	id = getUnsignedVarInt();
}

uint8_t BinaryReader::getByte()
{
	return readByte();
}

std::string BinaryReader::getString()
{
	uint32_t size = getUnsignedVarInt();
	std::string r;
	for (auto i = 0; i != size; i = i + 1) {
		r.push_back(readByte());
	}
	return r;
}

uint32_t BinaryReader::getUnsignedVarInt()
{
	int v2;
	unsigned int v3;
	char v5 = 0;

	v2 = 0;
	v3 = 0;
	do
	{
		v5 = readByte();
		v3 |= (v5 & 0x7F) << v2;
		v2 += 7;
	} while ((v5 & 0x80) != 0);
	return v3;
}

uint64_t BinaryReader::getUnsignedVarInt64()
{
	uint64_t v2;
	uint64_t v3;
	char v5 = 0;

	v2 = 0;
	v3 = 0;
	do
	{
		v5 = readByte();
		v3 |= (uint64_t)(v5 & 0x7F) << v2;
		v2 += 7;
	} while ((v5 & 0x80) != 0);
	return v3;
}

int BinaryReader::getVarInt()
{
	int v2;
	unsigned int v3;
	char v5 = 0;

	v2 = 0;
	v3 = 0;
	do
	{
		v5 = readByte();
		v3 |= (v5 & 0x7F) << v2;
		v2 += 7;
	} while ((v5 & 0x80) != 0);
	return ((int)(v3 << 31) >> 31) ^ (v3 >> 1);
}

int64_t BinaryReader::getVarInt64()
{
	uint64_t v2;
	uint64_t v3;
	char v5 = 0;

	v2 = 0;
	v3 = 0;
	do
	{
		v5 = readByte();
		v3 |= (uint64_t)(v5 & 0x7F) << v2;
		v2 += 7;
	} while ((v5 & 0x80) != 0);
	return ((int64_t)(v3 << 63) >> 63) ^ (v3 >> 1);
}

Vec3 BinaryReader::getVec3()
{
	return { getFloat(),getFloat(),getFloat() };
}

mce::UUID BinaryReader::getUUID()
{
	return { getUnsignedInt64(),getUnsignedInt64() };
}

std::string BinaryReader::getExtra()
{
	std::string r;
	for (int i = index; i != ptr_data->size(); i = i + 1) {
		r.push_back(getByte());
	}
	return r;
}

std::string BinaryReader::get(uint64_t size)
{
	std::string result;
	for (uint64_t i = 0; i != size; i = i + 1) {
		result.push_back(readByte());
	}
	return result;
}

std::string BinaryReader::getByteArray()
{
	return get(getUnsignedVarInt());
}

int16_t BinaryReader::getSignedShort()
{
	int16_t r;
	read((char*)&r, 2);
	return r;
}

uint16_t BinaryReader::getUnsignedShort()
{
	uint16_t r;
	read((char*)&r, 2);
	return r;
}

int BinaryReader::getSignedBigEndianInt()
{
	uint8_t t[4];
	t[3] = readByte();
	t[2] = readByte();
	t[1] = readByte();
	t[0] = readByte();
	return *(int*)t;
}

int BinaryReader::getSignedInt()
{

	int r;
	read((char*)&r, 4);
	return r;

}

uint32_t BinaryReader::getUnsignedInt()
{

	uint32_t r;
	read((char*)&r, 4);
	return r;
}

int64_t BinaryReader::getSignedInt64()
{
	int64_t r;
	read((char*)&r, 8);
	return r;
}

uint64_t BinaryReader::getUnsignedInt64()
{
	uint64_t r;
	read((char*)&r, 8);
	return r;
}

double BinaryReader::getDouble()
{
	double r;
	read((char*)&r, 8);
	return r;
}

float BinaryReader::getFloat()
{
	float r;
	read((char*)&r, 4);
	return r;
}

uint32_t BinaryPacketReader::getId()
{
	return id;
}

OriginalPacket::OriginalPacket(uint32_t id)
{
	packet_id = id;
}


BinaryPacket OriginalPacket::write() const
{
	return BinaryPacket(packet_id);
}

bool OriginalPacket::read(BinaryPacketReader & reader)
{
	if (packet_id != reader.getId()) {
		return false;
	}
	return true;
}

bool OriginalPacket::readFromString(std::string const* data)
{
	BinaryPacketReader reader(data);
	return read(reader);
}

void OriginalPacket::dumpPacket(std::string const& path)
{
	FILE* fp = fopen(path.c_str(), "ab+");

	std::string packetbuffer = write().data;
	const char* start = "BinaryPacketData[";
	fwrite(start, strlen(start), 1, fp);

	fwrite(packetbuffer.c_str(), packetbuffer.size(), 1, fp);


	const char* end = "]\n";
	fwrite(end, strlen(end), 1, fp);
	fclose(fp);
}


SettingsCommand::SettingsCommand(std::string const& cmd, bool b) :OriginalPacket(pid)
{
	this->cmd = cmd;
	this->b = b;
}

BinaryPacket SettingsCommand::write() const
{
	auto packet = OriginalPacket::write();
	packet.writeString(cmd);
	packet.writeByte(b);
	return packet;
}

bool SettingsCommand::read(BinaryPacketReader& reader)
{
	if (OriginalPacket::read(reader)) {
		cmd = reader.getString();
		b = reader.getByte();
		return true;
	}
	return false;
}


CommandBlockUpdate::CommandBlockUpdate(int x, int y, int z, int mode, int TickDelay, bool redstone, bool isConditional, bool TrackOutput, bool ExecuteOnFirstTick, std::string const& command, std::string const& LastOutput, std::string const& CustomName) : OriginalPacket(pid)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->mode = mode;
	this->TickDelay = TickDelay;
	this->redstone = redstone;
	this->isConditional = isConditional;
	this->TrackOutput = TrackOutput;
	this->ExecuteOnFirstTick = ExecuteOnFirstTick;
	this->command = command;
	this->LastOutput = LastOutput;
	this->CustomName = CustomName;
}

BinaryPacket CommandBlockUpdate::write() const
{
	auto packet = OriginalPacket::write();
	packet.writeByte(1);//isBlock
	packet.writeVarInt(x);
	packet.writeUnsignedVarInt(y);
	packet.writeVarInt(z);
	packet.writeUnsignedVarInt(mode);
	packet.writeByte(redstone);
	packet.writeByte(isConditional);
	packet.writeString(command);
	packet.writeString(LastOutput);
	packet.writeString(CustomName);
	packet.writeByte(TrackOutput);
	packet.writeSignedInt(TickDelay);
	packet.writeByte(ExecuteOnFirstTick);
	return packet;
}


RequestChunkRadius::RequestChunkRadius(int radius) : OriginalPacket(pid)
{
	this->radius = radius;
}

BinaryPacket RequestChunkRadius::write() const
{
	auto packet = OriginalPacket::write();
	packet.writeVarInt(radius);
	return packet;
}

bool RequestChunkRadius::read(BinaryPacketReader& reader)
{
	if (OriginalPacket::read(reader)) {
		radius = reader.getVarInt();
		return true;
	}
	return false;
}

PyRpc::PyRpc(std::string const& s,uint32_t u) : OriginalPacket(pid)
{
	this->data = s;
	this->unknown = u;
}

PyRpc::PyRpc() : OriginalPacket(pid)
{
}

Text::Text() : OriginalPacket(pid)
{
}


Text::Text(uint8_t type, std::string const& source, std::string const& message, std::vector<std::string> const& parameters, bool isLocalized, std::string const& xboxUserId, std::string const& platformChatId) : OriginalPacket(pid)
{
	this->type = type;
	this->source = source;
	this->message = message;
	this->isLocalized = isLocalized;
	this->xboxUserId = xboxUserId;
	this->platformChatId = platformChatId;
	this->parameters = parameters;
}

BinaryPacket Text::write() const
{
	auto packet = OriginalPacket::write();
	packet.writeByte(type);
	packet.writeByte(isLocalized || type == 2);
	switch (type) {
	case 1:
	case 7:
	case 8:
		packet.writeString(source);
		packet.writeString(message);
		break;
	case 0:
	case 5:
	case 6:
	case 9:
	case 10:
		packet.writeString(message);
		break;
	case 2:
	case 3:
	case 4:
		packet.writeString(message);
		packet.writeUnsignedVarInt(parameters.size());
		for (auto parameter : parameters) {
			packet.writeString(parameter);
		}
		break;
	}
	packet.writeString(xboxUserId);
	packet.writeString(platformChatId);
	packet.appendData(packet_end);
	return packet;
}

bool Text::read(BinaryPacketReader& reader)
{
	if (OriginalPacket::read(reader)) {
		type = reader.getByte();
		isLocalized = reader.getByte() || type == 2;
		switch (type) {
		case 1:
		case 7:
		case 8:
			source = reader.getString();
			message = reader.getString();
			break;
		case 0:
		case 5:
		case 6:
		case 9:
		case 10:
			message = reader.getString();
			break;
		case 2:
		case 3:
		case 4:
			message = reader.getString();
			uint32_t size = reader.getUnsignedVarInt();
			for (int i = 0; i != size; i = i + 1) {
				parameters.push_back(reader.getString());
			}
			break;
		}
		xboxUserId = reader.getString();
		platformChatId = reader.getString();
		packet_end = reader.getExtra();
		
		return true;
	}
	return false;
}

Text Text::createChat(std::string const& source, std::string const& message, std::string const& xboxUserId, std::string const& platformChatId)
{
	Text t(1, source, message, std::vector<std::string>(), false, xboxUserId, platformChatId);
	t.packet_end.push_back(0);
	return t;
}


BinaryPacket PyRpc::write() const
{
	auto packet = OriginalPacket::write();
	packet.writeString(data);
	packet.writeUnsignedInt(unknown);
	return packet;
}


bool PyRpc::read(BinaryPacketReader& reader)
{
	if (OriginalPacket::read(reader)) {
		data = reader.getString();
		unknown = reader.getUnsignedInt();
		return true;
	}
	return false;
}

LevelSoundEvent::LevelSoundEvent() : OriginalPacket(pid)
{
}
LevelSoundEvent::LevelSoundEvent(int sound, Vec3 const& pos, int extraData, std::string const& entityIdentifier, bool isBabyMob, bool isGlobal) : OriginalPacket(pid)
{
	this->sound = sound;
	this->pos = pos;
	this->extraData = extraData;
	this->entityIdentifier = entityIdentifier;
	this->isBabyMob = isBabyMob;
	this->isGlobal = isGlobal;
}


BinaryPacket LevelSoundEvent::write() const
{
	auto packet = OriginalPacket::write();
	packet.writeUnsignedVarInt(sound);
	packet.writeVec3(pos);
	packet.writeVarInt(extraData);
	packet.writeString(entityIdentifier);
	packet.writeByte(isBabyMob);
	packet.writeByte(isGlobal);
	return packet;
}

bool LevelSoundEvent::read(BinaryPacketReader& reader)
{
	if (OriginalPacket::read(reader)) {
		sound = reader.getUnsignedVarInt();
		pos = reader.getVec3();
		extraData = reader.getVarInt();
		entityIdentifier = reader.getString();
		isBabyMob = reader.getByte();
		isGlobal = reader.getByte();
		return true;
	}
	return false;
}

LevelSoundEventV1::LevelSoundEventV1() : OriginalPacket(pid)
{
}

LevelSoundEventV1::LevelSoundEventV1(uint8_t sound, Vec3 const& pos, int extraData, int pitch, bool isBabyMob, bool isGlobal) : OriginalPacket(pid)
{
	this->sound = sound;
	this->extraData = extraData;
	this->pitch = pitch;
	this->isBabyMob = isBabyMob;
	this->isGlobal = isGlobal;
}

BinaryPacket LevelSoundEventV1::write() const
{
	auto packet = OriginalPacket::write();
	packet.writeByte(sound);
	packet.writeVec3(pos);
	packet.writeVarInt(extraData);
	packet.writeVarInt(pitch);
	packet.writeByte(isBabyMob);
	packet.writeByte(isGlobal);
	return packet;
}

bool LevelSoundEventV1::read(BinaryPacketReader& reader)
{
	if (OriginalPacket::read(reader)) {
		sound = reader.getByte();
		pos = reader.getVec3();
		extraData = reader.getVarInt();
		pitch = reader.getVarInt();
		isBabyMob = reader.getByte();
		isGlobal = reader.getByte();
		return true;
	}
	return false;
}


LevelSoundEventV2::LevelSoundEventV2() : OriginalPacket(pid)
{
}
LevelSoundEventV2::LevelSoundEventV2(uint8_t sound, Vec3 const& pos, int extraData, std::string const& entityIdentifier, bool isBabyMob, bool isGlobal) : OriginalPacket(pid)
{
	this->sound = sound;
	this->pos = pos;
	this->extraData = extraData;
	this->entityIdentifier = entityIdentifier;
	this->isBabyMob = isBabyMob;
	this->isGlobal = isGlobal;
}

BinaryPacket LevelSoundEventV2::write() const
{
	auto packet = OriginalPacket::write();
	packet.writeByte(sound);
	packet.writeVec3(pos);
	packet.writeVarInt(extraData);
	packet.writeString(entityIdentifier);
	packet.writeByte(isBabyMob);
	packet.writeByte(isGlobal);
	return packet;
}

bool LevelSoundEventV2::read(BinaryPacketReader& reader)
{
	if (OriginalPacket::read(reader)) {
		sound = reader.getByte();
		pos = reader.getVec3();
		extraData = reader.getVarInt();
		entityIdentifier = reader.getString();
		isBabyMob = reader.getByte();
		isGlobal = reader.getByte();
		return true;
	}
	return false;
}

CommandRequest::CommandRequest(mce::UUID const& uuid, std::string const& str , CommandOrigin Origin,uint64_t playerUniqueID, bool Internal, bool UnLimited) : OriginalPacket(pid)
{
	this->uuid = uuid;
	this->cmdline = str;
	this->Origin = Origin;
	this->playerUniqueID = playerUniqueID;
	this->UnLimited = UnLimited;
	this->Internal = Internal;
}

BinaryPacket CommandRequest::write() const
{
	auto packet = OriginalPacket::write();
	packet.writeString(cmdline);
	packet.writeUnsignedVarInt(Origin);
	packet.writeUUID(uuid);
	packet.writeString("96045347-a6a3-4114-94c0-1bc4cc561694");
	if (Origin == CommandOrigin::CommandOriginDevConsole || Origin == CommandOrigin::CommandOriginTest) {
		packet.writeVarInt64(playerUniqueID);
	}
	packet.writeByte(Internal);
	packet.writeByte(UnLimited);
	return packet;
}
Login::Login() : OriginalPacket(pid)
{
}

bool Login::read(BinaryPacketReader& reader)
{
	if (OriginalPacket::read(reader)) {
		protocol = reader.getSignedBigEndianInt();

		auto conn = reader.getByteArray();

		BinaryReader mData(&conn);

		std::string chain = mData.get(mData.getUnsignedInt());

		Json::Reader r;
		r.parse(chain, chainData);
		for (int i = 0; i != chainData["chain"].size(); i = i + 1) {
			std::string data = chainData["chain"][i].asString();
			Json::Value arr;
			auto jv = decodeToken(data);
			for (auto i : jv) {
				arr.append(i);
			}
			chainData["chain"][i] = arr;
		}

		std::string client = mData.get(mData.getUnsignedInt());

		auto jv = decodeToken(client);
		for (auto i : jv) {
			clientData.append(i);
		}

		return true;
	}
	return false;
}


ContainerOpen::ContainerOpen() : OriginalPacket(pid)
{
}

bool ContainerOpen::read(BinaryPacketReader& reader)
{
	if (OriginalPacket::read(reader)) {
		windowId = reader.getByte();
		type = reader.getByte();
		x = reader.getVarInt();
		y = reader.getUnsignedVarInt();
		z = reader.getVarInt();
		entityId = reader.getVarInt64();
		return true;
	}
	return false;
}

ActorEvent::ActorEvent(uint64_t eid, uint8_t event, int data) : OriginalPacket(pid)
{
	this->eid = eid;
	this->event = event;
	this->data = data;
}


BinaryPacket ActorEvent::write() const
{
	auto packet = OriginalPacket::write();
	packet.writeUnsignedVarInt64(eid);
	packet.writeByte(event);
	packet.writeVarInt(data);
	return packet;
}


BinaryReader::BinaryReader(std::string const* ptr)
{
	ptr_data = ptr;
}

BinaryReader::BinaryReader()
{
}


std::vector<Json::Value> Login::decodeToken(std::string const& token) const
{
	auto sv = Utils::split(token, ".");
	std::vector<Json::Value> result;
	for (int i = 0; i != sv.size() - 1; i = i + 1) {
		std::string s = Utils::base64_decode(sv[i], false);


		Json::Reader reader;
		Json::Value value;
		reader.parse(s, value);

		result.push_back(value);
	}
	result.push_back(sv[sv.size() - 1]);


	return result;
}

std::string Login::getServerIP() {
	return clientData[1]["ServerAddress"].asString();
}

std::string Login::getPlayerName() {
	Json::Value chain = chainData["chain"];
	for (auto i : chain) {
		for (auto t : i) {
			if (t.type() == Json::ValueType::objectValue && t.isMember("extraData")) {
				return t["extraData"]["displayName"].asString();
			}
		}
	}
	return "null";
}


NeteaseJson::NeteaseJson() : OriginalPacket(pid)
{
}

NeteaseJson::NeteaseJson(Json::Value const& d) : OriginalPacket(pid)
{
	data = d;
}

BinaryPacket NeteaseJson::write() const
{
	auto packet = OriginalPacket::write();
	Json::FastWriter writer;
	packet.writeString(writer.write(data));
	return packet;
}

bool NeteaseJson::read(BinaryPacketReader& reader)
{
	if (OriginalPacket::read(reader)) {
		std::string s = reader.getString();
		Json::Reader reader;
		reader.parse(s, data);
		return true;
	}
	return false;
}

CommandOutput::CommandOutput():OriginalPacket(pid)
{
}



bool CommandOutput::read(BinaryPacketReader& reader)
{
	if (OriginalPacket::read(reader)) {
		Origin = (CommandOrigin)reader.getUnsignedVarInt();
		uuid = reader.getUUID();
		requestId = reader.getString();
		if (Origin == CommandOrigin::CommandOriginDevConsole || Origin == CommandOrigin::CommandOriginTest) {
			playerUniqueId = reader.getUnsignedVarInt64();
		}
		OutputType = reader.getByte();
		successCount = reader.getUnsignedVarInt();
		uint32_t count = reader.getUnsignedVarInt();
		for (uint32_t i = 0; i != count; i = i + 1) {
			CommandMessage message;
			message.read(reader);
			CommandOutputMessage.push_back(message);
		}
		if (OutputType == 4) {
			dataSet = reader.getString();
		}
		return true;
	}
	return false;
}

void CommandOutput::CommandMessage::read(BinaryReader& reader)
{
	success = reader.getByte();
	message = reader.getString();
	uint32_t count = reader.getUnsignedVarInt();
	for (uint32_t i = 0; i != count; i = i + 1) {
		Parameters.push_back(reader.getString());
	}
}

MobEffect::MobEffect(uint64_t eid, uint8_t type, int effectId, int amplifier, bool particles, int duration):OriginalPacket(pid)
{
	this->eid = eid;
	this->type = type;
	this->effectId = effectId;
	this->amplifier = amplifier;
	this->particles = particles;
	this->duration = duration;
}

BinaryPacket MobEffect::write() const
{
	BinaryPacket packet = OriginalPacket::write();
	packet.writeUnsignedVarInt64(eid);
	packet.writeByte(type);
	packet.writeVarInt(effectId);
	packet.writeVarInt(amplifier);
	packet.writeByte(particles);
	packet.writeVarInt(duration);
	return packet;
}
