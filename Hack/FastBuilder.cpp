#include "FastBuilder.h"
#include "MCPE/SDK/mce.h"
#include "MCPE/GameData.h"
#include <fstream>
#include <string>
#include <list>
#include <mutex>
#include <sstream>
#include <thread>
#include "Utils.hpp"
#include "HackSDK.h"
#include "Chat.h"
#include "Netease.h"

#include "CBrotli/include/brotli-cpp.hpp"
#include "spot/spot.hpp"
#include "NBT/include/io/izlibstream.h"
#include "NBT/include/io/ozlibstream.h"
#include "NBT/include/io/stream_reader.h"
#include "NBT/include/io/stream_writer.h"
#include "NBT/include/nbt_tags.h"
#include "FastBuilderHelper/BlockRuntimeIDMap.h"


BlockRuntimeIDMap blockRTIDMap;


class BlockData {
public:
	int x;
	int y;
	int z;
	int id;
	int variant;
	BlockData(int a, int b, int c, int d, int e) {
		x = a;
		y = b;
		z = c;
		id = d;
		variant = e;
	}
	BlockData(void) {

	}
};

class BlockDataV2 {
public:
	int x;
	int y;
	int z;
	int id;
	std::string blockState;
	BlockDataV2() {

	}
	BlockDataV2(int a, int b, int c, int d, std::string state) {
		x = a;
		y = b;
		z = c;
		id = d;
		blockState = state;
	}
};


class MapData {
public:
	static int getChunkPos(int pos, int size) {
		if (pos >= 0) {
			return pos / size;
		}
		else {
			int s = 0;
			while (true) {
				if (s * 64 <= pos) {
					return s;
				}
				else {
					s = s - 1;
				}
			}
		}
	}

	void read(std::string path, int rx, int ry, int rz) {
		FILE* fp = fopen(path.c_str(), "rb");

		fseek(fp, 0, SEEK_SET);
		int blockNum = 0;
		fread(&blockNum, 1, 4, fp);
		for (int i = 0; i != blockNum; i = i + 1) {
			BlockData bd;
			fread(&bd, 1, sizeof(bd), fp);
			bd.x = bd.x + rx;
			bd.y = bd.y + ry;
			bd.z = bd.z + rz;

			pushData(bd);
		}
		std::string j;
		while (!feof(fp)) {
			char c;
			fread(&c, 1, 1, fp);
			j.push_back(c);
		}


		Json::Value idmap;
		Json::Reader reader;
		reader.parse(j, idmap);

		fclose(fp);


		for (auto i : idmap["BlockEntityData"]) {
			i["x"] = i["x"].asInt() + rx;
			i["y"] = i["y"].asInt() + ry;
			i["z"] = i["z"].asInt() + rz;
			addBlockEntity(i);
		}


		Json::Value::Members mem = idmap.getMemberNames();
		for (auto it = mem.begin(); it != mem.end(); it++) {
			if (idmap[*it].type() == Json::ValueType::intValue || idmap[*it].type() == Json::ValueType::uintValue) {
				idMapA.insert(std::pair<int, std::string>(idmap[*it].asInt(), *it));
			}
		}
	}

	void addBlockEntity(Json::Value const& data) {
		getArea(data["x"].asInt(), data["z"].asInt())->blockEntityData.append(data);
	}


	class IDMap :public std::map<int,std::string>{
	public:
		bool hasBlockID(std::string const& name) {
			for (auto i : *this) {
				if (i.second == name) {
					return true;
				}
			}
			return false;
		}

		int getBlockID(std::string const& name) {
			for (auto i : *this) {
				if (i.second == name) {
					return i.first;
				}
			}
			return -1;
		}
	};
	IDMap idMapA;

	class Area {
	public:

		class BlockContainer {
		public:
			BinaryReader reader;
			BinaryWriter data;
			int blockNum = 0;
			BlockContainer() {
				reader = BinaryReader(&data.data);
			}
		};

		bool fullyLoaded = false;
		int x, z;
		std::map<int, BlockContainer> data;
		Json::Value blockEntityData;
		std::vector<BlockData> state_blocks_;
		std::vector<BlockDataV2> state_blocks;


		void writeBlock(BlockData const& bd) {
			data[bd.y].blockNum = data[bd.y].blockNum + 1;
			data[bd.y].data.writeByte(bd.x - x * 64);
			data[bd.y].data.writeByte(bd.z - z * 64);
			data[bd.y].data.writeUnsignedVarInt(bd.id);
			data[bd.y].data.writeUnsignedVarInt(bd.variant);
		}


		int nextBlock(void** p) {
			if (data.begin() != data.end()) {
				if (data.begin()->second.blockNum == 0) {
					data.erase(data.begin());
					return nextBlock(p);
				}
				else {
					data.begin()->second.blockNum = data.begin()->second.blockNum - 1;
					BinaryReader& reader = data.begin()->second.reader;
					BlockData *bd = new BlockData({ (int)reader.getByte() + x * 64,data.begin()->first,(int)reader.getByte() + z * 64,(int)reader.getUnsignedVarInt(),(int)reader.getUnsignedVarInt() });
					*p = bd;
					return 1;
				}
			}
			else if(state_blocks.size() > 0){
				BlockDataV2 bd2 = state_blocks.back();
				*p = new BlockDataV2(bd2);
				state_blocks.pop_back();
				return 2;
			}
			else if (state_blocks_.size() > 0) {
				BlockData bd2 = state_blocks_.back();
				*p = new BlockData(bd2);
				state_blocks_.pop_back();
				return 3;
			}
			return 0;
		}

		int getBlockNum() {
			int t = 0;
			for (auto h = data.begin(); h != data.end(); h++) {
				t = t + h->second.blockNum;
			}
			return t + state_blocks.size() + state_blocks_.size();
		}


		Area(int x, int z) {
			this->x = x;
			this->z = z;

		}

		Area() {

		}
	};

	std::list<Area> areas;
	std::list<Area>::iterator currentArea;

	void newArea() {
		currentArea = areas.erase(currentArea);
	}

	int nextBlock(void** p) {
		return currentArea->nextBlock(p);
	}

	

	int getCurrentAreaBlockNum() {
		return currentArea->getBlockNum();
	}

	int getBlockNum() {
		int t = 0;
		for (auto it = areas.begin(); it != areas.end(); it++) {
			t = t + it->getBlockNum();
		}

		return t;
	}

	std::list<Area>::iterator getArea(int bx,int bz) {
		int x = getChunkPos(bx, 64);
		int z = getChunkPos(bz, 64);
		for (auto it = areas.begin(); it != areas.end(); it++) {
			if (x == it->x && z == it->z) {
				return it;
			}
		}

		Area area(x, z);
		areas.push_back(area);
		return getArea(bx, bz);
	}

	void pushData(BlockData const& bd) {
		getArea(bd.x,bd.z)->writeBlock(bd);
	}

	void pushBlockWithState(BlockDataV2 const& bd) {
		getArea(bd.x, bd.z)->state_blocks.push_back(bd);
	}

	void pushBlockWithState_(BlockData const& bd) {
		getArea(bd.x, bd.z)->state_blocks_.push_back(bd);
	}

	void prepare() {
		currentArea = areas.begin();
	}
};

namespace FBLib {
	int id = 0;
	Json::Value idmap;

	std::string getBlockName(Block* bl) {
		std::string n = bl->_BlockLegacy->name;
		std::string res;
		for (int i = 5; i != n.size(); i = i + 1) {
			res.push_back(n[i]);
		}
		return res;

	}

	BlockData getBlockData(BlockSource* bs, int x, int y, int z, int rx, int ry, int rz) {

		Block* block = bs->getBlock(x, y, z);

		if (getBlockName(block) == "air") {
			BlockData result(x - rx, y - ry, z - rz, 0, block->aux);
			return result;
		}
		if (idmap.isMember(getBlockName(block)) == false) {
			id = id + 1;
			idmap[getBlockName(block)] = id;
		}
		BlockData result(x - rx, y - ry, z - rz, idmap[getBlockName(block)].asInt(), block->aux);
		return result;
	}

	BlockData getExtraBlockData(BlockSource* bs, int x, int y, int z, int rx, int ry, int rz) {

		Block* block = bs->getExtraBlock({x,y,z});
		if (getBlockName(block) == "air") {
			BlockData result(x - rx, y - ry, z - rz, 0, block->aux);
			return result;
		}
		if (idmap.isMember(getBlockName(block)) == false) {
			id = id + 1;
			idmap[getBlockName(block)] = id;
		}
		BlockData result(x - rx, y - ry, z - rz, idmap[getBlockName(block)].asInt(), block->aux);
		return result;
	}

	void executeCommand(std::string const& cmd) {

		LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
		if (lp->getDimension()->getDimensionId() == 0 && strstr(cmd.c_str(), "setblock")) {
			SettingsCommand pa(cmd, true);
			HackSDK::sendOriginalPacket(pa);
		}
		else {
			std::stringstream os;
			os << "execute @a[name=\"" << lp->getName() << "\"] ~~~ " << cmd;
			SettingsCommand pa(os.str(), true);
			HackSDK::sendOriginalPacket(pa);
		}
	}

	void teleportTo(int x, int y, int z) {
		std::stringstream TPCommand;
		TPCommand << "/tp @p " << x << " " << y << " " << z << " ";

		executeCommand(TPCommand.str());
	}

	void setBlock(BlockData const& d, MapData& val) {
		std::stringstream SBCommand;
		if (d.variant == blockRTIDMap.rt_aux117) {
			SBCommand << "setblock " << d.x << " " << d.y << " " << d.z << " " << blockRTIDMap.ridMap117[d.id][0].asString() << " " << blockRTIDMap.ridMap117[d.id][1].asInt();
		}
		else if (d.variant == blockRTIDMap.rt_aux118) {
			SBCommand << "setblock " << d.x << " " << d.y << " " << d.z << " " << blockRTIDMap.ridMap118[d.id][0].asString() << " " << blockRTIDMap.ridMap118[d.id][1].asInt();
		}
		else if (d.variant == blockRTIDMap.rt_schemfile) {
			SBCommand << "setblock " << d.x << " " << d.y << " " << d.z << " " << val.idMapA[d.id];
		}
		else {
			SBCommand << "setblock " << d.x << " " << d.y << " " << d.z << " " << val.idMapA[d.id] << " " << (int)d.variant;
		}


		executeCommand(SBCommand.str());
	}

	void setBlock(BlockDataV2 const& d, MapData& val) {
		std::stringstream SBCommand;
		SBCommand << "setblock " << d.x << " " << d.y << " " << d.z << " " << val.idMapA[d.id] << " " << d.blockState;
		executeCommand(SBCommand.str());
	}

	bool hasChunk(BlockSource* bs, int x, int y, int z) {

		BlockPos bp(x, y, z);
		if (bs->hasChunksAt(bp, 1) == false) {
			return false;
		}
		LevelChunk* lc = bs->getChunkAt(x, y, z);
		if (lc == nullptr) {
			return false;
		}
		if (lc->getFinalize() != 2) {
			return false;
		}
		if (lc->isFullyLoaded() == false) {
			return false;
		}

		Block* bl = bs->getBlock(x, y, z);
		if (bl != nullptr) {
			if (getBlockName(bl) == "client_request_placeholder_block") {
				return false;
			}
		}
		else {
			return false;
		}
		
		bl = bs->getExtraBlock({x,y,z});
		if (bl != nullptr) {
			if (getBlockName(bl) == "client_request_placeholder_block") {
				return false;
			}
		}
		else {
			return false;
		}

		return true;
	}

}




class DumpEvent {
public:
	Json::Value blockEntityData;
	class Pos2 {
	public:
		int x;
		int z;
	};
	template<class T>
	void swap(T& a, T& b) {
		T temp = a;
		a = b;
		b = temp;
	}

	FILE* fp;

	DumpEvent(std::string path, int kx, int ky, int kz, int mx, int my, int mz, bool q) {
		auto res = Utils::split(path, ".");
		if (res[res.size() - 1] != "busj") {
			path = path + ".busj";
		}
		fp = fopen(path.c_str(), "wb+");
		fseek(fp, 4, SEEK_SET);
		filePath = path;
		if (kx > mx) {
			swap(kx, mx);
		}
		if (ky > my) {
			swap(ky, my);
		}
		if (kz > mz) {
			swap(kz, mz);
		}
		x = kx;
		y = ky;
		z = kz;
		dy = my;
		op = q;
		for (int ix = kx; ix <= mx; ix = ix + 1) {
			for (int iz = kz; iz <= mz; iz = iz + 1) {
				Pos2 p;
				p.x = ix;
				p.z = iz;
				Locate.push_back(p);
			}
		}
	}
	int num = 0;

	void add(BlockData const& bd) {
		fwrite(&bd, 1, sizeof(bd), fp);
		num = num + 1;
	}



	void addBlockEntityData(BlockPos const& bp, CompoundTag& tag, BlockSource* bs) {
		Json::Value val;
		val["x"] = bp.x - x;
		val["y"] = bp.y - y;
		val["z"] = bp.z - z;
		std::string blockid = tag.getString("id");
		val["id"] = blockid;
		if (blockid == "Sign") {
			val["Text"] = tag.getString("Text");
			blockEntityData.append(val);
		}
		else if (tag.contains("Items")) {
			ListTag* lt = tag.getList("Items");
			int c = lt->size();
			for (int i = 0; i != c; i = i + 1) {
				CompoundTag* item = lt->getCompound(i);
				val["Items"][i]["Slot"] = item->getByte("Slot");
				val["Items"][i]["Name"] = item->getString("Name");
				val["Items"][i]["Count"] = item->getByte("Count");

				ItemStackBase mcItem;
				
				mcItem.load(*item);
				val["Items"][i]["DataValue"] = mcItem.getDataValue();
				
			}
			blockEntityData.append(val);
		}
		else if (blockid == "CommandBlock") {
			CommandBlockActor* cba = (CommandBlockActor*)bs->getBlockEntity(bp);
			val["Mode"] = cba->getMode(*bs);
			val["redstone"] = tag.getBoolean("auto") == 0;
			val["isConditional"] = cba->getConditionalMode(*bs);
			val["Command"] = tag.getString("Command");
			val["LastOutput"] = tag.getString("LastOutput");
			val["CustomName"] = tag.getString("CustomName");
			val["TickDelay"] = tag.getInt("TickDelay");
			val["TrackOutput"] = tag.getBoolean("TrackOutput");
			val["ExecuteOnFirstTick"] = tag.getBoolean("ExecuteOnFirstTick");
			blockEntityData.append(val);
		}
		else if (tag.contains("RecordItem")) {
			CompoundTag* item = tag.getCompound("RecordItem");
			val["Items"][0]["Slot"] = 0;
			val["Items"][0]["Name"] = item->getString("Name");
			val["Items"][0]["Count"] = item->getByte("Count");

			ItemStackBase mcItem;
			mcItem.load(*item);
			val["Items"][0]["DataValue"] = mcItem.getDataValue();

			blockEntityData.append(val);
		}
	
	}

	void saveToFile(void) {
		fseek(fp, 0, SEEK_SET);
		fwrite(&num, 1, 4, fp);
		fseek(fp, 0, SEEK_END);
		Json::Value json = FBLib::idmap;
		json["BlockEntityData"] = blockEntityData;
		std::string jsonstr = json.toStyledString();
		fwrite(jsonstr.c_str(), 1, jsonstr.size(), fp);
		fclose(fp);
	}

	std::vector<Pos2> Locate;
	std::string filePath;
	int x;
	int y;
	int z;
	int dy;
	bool op;
};


class ImportEvent {
public:
	MapData data;
	int bps = 0;

	ImportEvent(int c) {
		bps = c;
	}
	ImportEvent(std::string filePath, int rx, int ry, int rz, int v) {
		data.read(filePath,rx,ry,rz);
		data.prepare();


		bps = v;
		FBLib::executeCommand("/gamerule sendcommandfeedback true");

	}
	~ImportEvent() {

		FBLib::executeCommand("/gamerule sendcommandfeedback false");
	}
};


class ImageParser {
public:
	int getColorDistance(int r1, int g1, int b1, int r2, int g2, int b2) {
		if (r1 + r2 > 256) {
			return 2*(r1 - r2) * (r1 - r2) + 4*(g1 - g2) * (g1 - g2) + 3*(b1 - b2) * (b1 - b2);
		}
		else {
			return 3 * (r1 - r2) * (r1 - r2) + 4 * (g1 - g2) * (g1 - g2) + 2 * (b1 - b2) * (b1 - b2);
		}
	}

	class ColorBlock {
	public:
		ColorBlock(std::string const& n, int a, uint8_t x, uint8_t y, uint8_t z) {
			blockName = n;
				aux = a;
				r = x;
				g = y;
				b = z;
		}
		ColorBlock(std::string const& n, int a, uint8_t x, uint8_t y, uint8_t z,int l) {
			blockName = n;
			aux = a;
			r = x;
			g = y;
			b = z;
			lvl = l;
		}
		std::string blockName;
		int aux;
		uint8_t r;
		uint8_t g;
		uint8_t b;
		int lvl = 1;
		ColorBlock getLight() {
			ColorBlock cb(blockName,aux,min((int)r * 255 / 220,255),min((int)g * 255 / 220,255),min((int)b * 255 / 220,255),2);
			return cb;
		}
		ColorBlock getDark() {
			ColorBlock cb(blockName, aux, r * 180 / 220, g * 180 / 220, b * 180 / 220, 0);
			return cb;
		}
	};

	class Color {
	public:
		Color(uint8_t x, uint8_t y, uint8_t z) {
			r = x;
			g = y;
			b = z;
		}
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};


	std::vector<ColorBlock> colorBlocks = { ColorBlock("stone", 0,89, 89, 89),ColorBlock("stone", 1,135, 102, 76),ColorBlock("stone", 3,237, 235, 229),ColorBlock("stone", 5,104, 104, 104),ColorBlock("grass", 0,144, 174, 94),ColorBlock("planks", 0,129, 112, 73),ColorBlock("planks", 1,114, 81, 51),ColorBlock("planks", 2,228, 217, 159),ColorBlock("planks", 4,71, 71, 71),ColorBlock("planks", 5,91, 72, 50),ColorBlock("leaves", 0,64, 85, 32),ColorBlock("leaves", 1,54, 75, 50),ColorBlock("leaves", 2,68, 83, 47),ColorBlock("leaves", 14,58, 71, 40),ColorBlock("leaves", 15,55, 73, 28),ColorBlock("sponge", 0,183, 183, 70),ColorBlock("lapis_block", 0,69, 101, 198),ColorBlock("noteblock", 0,111, 95, 63),ColorBlock("web", 0,159, 159, 159),ColorBlock("wool", 0,205, 205, 205),ColorBlock("wool", 1,163, 104, 54),ColorBlock("wool", 2,132, 65, 167),ColorBlock("wool", 3,91, 122, 169),ColorBlock("wool", 5,115, 162, 53),ColorBlock("wool", 6,182, 106, 131),ColorBlock("wool", 7,60, 60, 60),ColorBlock("wool", 8,123, 123, 123),ColorBlock("wool", 9,69, 100, 121),ColorBlock("wool", 10,94, 52, 137),ColorBlock("wool", 11,45, 59, 137),ColorBlock("wool", 12,78, 61, 43),ColorBlock("wool", 13,85, 100, 49),ColorBlock("wool", 14,113, 46, 44),ColorBlock("wool", 15,20, 20, 20),ColorBlock("gold_block", 0,198, 191, 84),ColorBlock("iron_block", 0,134, 134, 134),ColorBlock("double_stone_slab", 1,196, 187, 136),ColorBlock("double_stone_slab", 6,204, 202, 196),ColorBlock("double_stone_slab", 7,81, 11, 5),ColorBlock("tnt", 0,188, 39, 26),ColorBlock("mossy_cobblestone", 0,131, 134, 146),ColorBlock("diamond_block", 0,102, 173, 169),ColorBlock("farmland", 0,116, 88, 65),ColorBlock("ice", 0,149, 149, 231),ColorBlock("pumpkin", 1,189, 122, 62),ColorBlock("monster_egg", 1,153, 156, 169),ColorBlock("red_mushroom_block", 0,131, 53, 50),ColorBlock("vine", 1,68, 89, 34),ColorBlock("brewing_stand", 6,155, 155, 155),ColorBlock("double_wooden_slab", 1,98, 70, 44),ColorBlock("emerald_block", 0,77, 171, 67),ColorBlock("stained_hardened_clay", 0,237, 237, 237),ColorBlock("stained_hardened_clay", 2,154, 76, 194),ColorBlock("stained_hardened_clay", 4,213, 213, 82),ColorBlock("stained_hardened_clay", 6,211, 123, 153),ColorBlock("stained_hardened_clay", 8,142, 142, 142),ColorBlock("stained_hardened_clay", 10,110, 62, 160),ColorBlock("slime", 0,109, 141, 60),ColorBlock("packed_ice", 0,128, 128, 199),ColorBlock("repeating_command_block", 1,77, 43, 112),ColorBlock("chain_command_block", 1,70, 82, 40),ColorBlock("nether_wart_block", 0,93, 38, 36),ColorBlock("bone_block", 0,160, 153, 112) };
	int closest_color(Color & color,bool m) {

		int delta = INT_MAX;
		int index = 0;
		for (int i = 0; i != colorBlocks.size(); i = i + 1) {
			if (m == false && colorBlocks[i].lvl != 1)continue;
			int d = getColorDistance(color.r, color.g, color.b, colorBlocks[i].r, colorBlocks[i].g, colorBlocks[i].b);
			if (d < delta) {
				delta = d;
				index = i;
			}
			
		}
		return index;
	}
	
	ImageParser(std::string const& filePath, int sx, int sy, int sz, MapData& data,bool m,int min_y) {
		
		
		std::vector<ColorBlock> light;
		std::vector<ColorBlock> dark;


		for (int i = 0; i != colorBlocks.size(); i = i + 1) {
			data.idMapA[i] = colorBlocks[i].blockName;
			light.push_back(colorBlocks[i].getLight());
			dark.push_back(colorBlocks[i].getDark());
		}
		data.idMapA[colorBlocks.size()] = "log";

		colorBlocks.insert(colorBlocks.begin(), light.begin(), light.end());
		colorBlocks.insert(colorBlocks.begin(), dark.begin(), dark.end());

		spot::image im(filePath);
		int width = im.w;
		int height = im.h;
		
		auto image = im.rgb();
		for (uint32_t x = 0; x != width; x = x + 1) {
			int cy = sy;
			for (uint32_t z = 0; z != height; z = z + 1) {
				BlockData bd;
				bd.x = x + sx;
				bd.z = z + sz;
				Color color(image[(x + z * width)*3], image[(x + z * width) * 3 + 1], image[(x + z * width) * 3 + 2]);
				int t = closest_color(color, m);

				int lvl = colorBlocks[t].lvl;

					if (lvl == 0) {
						cy = cy - 2;
					}
					else if (lvl == 2) {
						cy = cy + 2;
					}
					if (cy > 254) {
						cy = sy;
					}
					else if (cy < min_y) {
						cy = 254;
					}

				bd.y = cy;
				bd.id = data.idMapA.getBlockID(colorBlocks[t].blockName);
				bd.variant = colorBlocks[t].aux;
				data.pushData(bd);

				if (colorBlocks[t].blockName == "leaves") {
					BlockData log;
					log.x = bd.x;
					log.y = bd.y - 1;
					log.z = bd.z;
					log.variant = 0;
					log.id = data.idMapA.getBlockID("log");
					data.pushData(log);
				}

			}
		}
		data.prepare();

	}
};

class ImportEventProvider {
public:
	virtual ~ImportEventProvider() {

	}

	virtual float getProcess() = 0;

	virtual ImportEvent* getEvent() = 0;
};

class BigSchematicImporter :public ImportEventProvider{
public:
	int sx, sy, sz, dx, dy, dz, bps;
	std::pair<std::string, std::unique_ptr<nbt::tag_compound>> MainData;
	class Area {
	public:
		int x;
		int z;
	};

	virtual ~BigSchematicImporter() {

	}

	int area_num = 0;

	std::list<Area> AreaArray;

	virtual float getProcess() {
		return ((float)(area_num - AreaArray.size()) / (float)area_num) * 100;
	}

	BigSchematicImporter(std::string const& filePath, int sx, int sy, int sz, int bps) {
		std::ifstream file(filePath, std::ios::binary);
		zlib::izlibstream igzs(file);

		MainData = nbt::io::read_compound(igzs);

		nbt::tag_compound& schematic = *MainData.second;

		this->sx = sx;
		this->sy = sy;
		this->sz = sz;
		this->bps = bps;

		dx = schematic["Width"].as<nbt::tag_short>();
		dy = schematic["Height"].as<nbt::tag_short>();
		dz = schematic["Length"].as<nbt::tag_short>();


		int nx = MapData::getChunkPos(sx, 64);
		int nz = MapData::getChunkPos(sz, 64);
		int nx1 = MapData::getChunkPos((sx + dx) , 64);
		int nz1 = MapData::getChunkPos((sz + dz) , 64);

		for (int cx = nx; cx <= nx1; cx = cx + 1) {
			for (int cz = nz; cz <= nz1; cz = cz + 1) {
				Area area = { cx * 64,cz * 64 };
				AreaArray.push_back(area);
			}
		}

		area_num = AreaArray.size();
	}

	BlockData getBlockData(int x, int y, int z, nbt::tag_byte_array& blocks, nbt::tag_byte_array& blockData) {


		int index = x + z * dx + y * dz * dx;
		if (x >= dx || y >=  dy || z >= dz || x < 0 || y < 0 || z < 0) {
			BlockData bd;
			bd.id = 0;
			return bd;
		}

		BlockData bd;
		bd.id = (uint8_t)blocks[index];
		bd.x = sx + x;
		bd.y = sy + y;
		bd.z = sz + z;
		bd.variant = (uint8_t)blockData[index];
		return bd;
		
		
	}

	virtual ImportEvent* getEvent() {
		if (AreaArray.size() == 0)return nullptr;
		nbt::tag_compound& schematic = *MainData.second;

		ImportEvent* even = new ImportEvent(bps);

		nbt::tag_byte_array& blocks = schematic["Blocks"].as<nbt::tag_byte_array>();
		nbt::tag_byte_array& blockData = schematic["Data"].as<nbt::tag_byte_array>();

	
		for (int area_num = 0; area_num != 10; area_num = area_num + 1) {
			if(AreaArray.size() == 0)break;
			auto it = AreaArray.begin();
			for (int ix = 0; ix != 64; ix = ix + 1) {
				for (int iz = 0; iz != 64; iz = iz + 1) {
					for (int iy = 0; iy != dy; iy = iy + 1) {
						BlockData bd = getBlockData(it->x + ix - sx, iy, it->z + iz - sz, blocks, blockData);
						if (bd.id != 0) {
							even->data.pushData(bd);

						}
					}
				}
			}
			AreaArray.erase(it);
		}


		for (auto i : blockRTIDMap.schematicArray) {
			std::string fullName = i[2].asString();
			auto t = Utils::split(fullName, ":");
			auto p = Utils::split(t[1], "[");
			std::string bn = p[0];
			even->data.idMapA[i[0].asInt()] = bn;
		}
		

		even->data.prepare();
		return even;
	}
};


class SchematicParser {
public:
	SchematicParser(std::string const& filePath, int sx, int sy, int sz, MapData& data,std::string const& type) {
		std::ifstream file(filePath, std::ios::binary);
		zlib::izlibstream igzs(file);
		auto pair = nbt::io::read_compound(igzs);
		nbt::tag_compound& schematic = *pair.second;

		int dx = schematic["Width"].as<nbt::tag_short>();
		int dy = schematic["Height"].as<nbt::tag_short>();
		int dz = schematic["Length"].as<nbt::tag_short>();
		if (type == "schematic") {
			for (auto i : blockRTIDMap.schematicArray) {
				std::string fullName = i[2].asString();
				auto t = Utils::split(fullName, ":");
				auto p = Utils::split(t[1], "[");
				std::string bn = p[0];
				data.idMapA[i[0].asInt()] = bn;
			}


			auto blocks = schematic["Blocks"].as<nbt::tag_byte_array>();
				auto blockData = schematic["Data"].as<nbt::tag_byte_array>();
				for (int z = 0; z < dz; z++) {
					for (int x = 0; x < dx; x++) {
						for (int y = 0; y < dy; y++) {
							int index = x + z * dx + y * dz * dx;
							uint8_t bid = (uint8_t)blocks[index];
							if (bid != 0) {
								BlockData bd;
								bd.id = bid;
								bd.x = sx + x;
								bd.y = sy + y;
								bd.z = sz + z;
								bd.variant = (uint8_t)blockData[index];
								data.pushData(bd);
							}
						}
					}
				}
			
		}
		else if (type == "schem") {
			auto palette = schematic["Palette"].as<nbt::tag_compound>();
			for (auto it = palette.begin(); it != palette.end(); it++) {
				std::string fullName = it->first;
				if (strstr(fullName.c_str(), "minecraft:") == nullptr) {
					fullName = "minecraft:" + fullName;
				}
				std::stringstream cmdName;

				Json::Value javaBlock;
				if (blockRTIDMap.schemArray.isMember(fullName)) {
					javaBlock = blockRTIDMap.schemArray[fullName];
				}
				else {
					Json::Value::Members mem = blockRTIDMap.schemArray.getMemberNames();
					auto sv = Utils::split(fullName, "[");
					if (sv.size() == 1) {
						data.idMapA[it->second.as<nbt::tag_int>().get()] = fullName;
						continue;
					}

					std::string states = sv[1];
					states.pop_back();

					auto state_list = Utils::split(states, ",");


					int max = 0;

					for (auto it = mem.begin(); it != mem.end(); it++) {
						if (strstr(it->c_str(), sv[0].c_str())) {
							int i = 1;
							for (auto state : state_list) {
								if (strstr(it->c_str(), state.c_str())) {
									i = i + 1;
								}
							}
							if (max < i) {
								max = i;
								javaBlock = blockRTIDMap.schemArray[*it];
							}
						}
					}
					if (max == 0) {
						data.idMapA[it->second.as<nbt::tag_int>().get()] = fullName;
						continue;
					}
				}

				std::string blockState = "";
				if (javaBlock.isMember("bedrock_states")) {
					Json::FastWriter writer;
					writer.omitEndingLineFeed();
					blockState = writer.write(javaBlock["bedrock_states"]);
					for (auto it = blockState.begin(); it != blockState.end(); it = it + 1) {
						if (*it == '{') {
							*it = '[';
						}
						else if (*it == '}') {
							*it = ']';
						}
					}
				}
				cmdName << javaBlock["bedrock_identifier"].asString() << " " << blockState;

				data.idMapA[it->second.as<nbt::tag_int>().get()] = cmdName.str();
			}

			auto blocks = schematic["BlockData"].as<nbt::tag_byte_array>();

			int block_index = 0;

			uint32_t varInt = 0;
			uint32_t varIntLength = 0;
			for (int i = 0; i < blocks.size(); i++) {
				varInt |= ((uint8_t)blocks[i] & 127) << (varIntLength++ * 7);
				if (((uint8_t)blocks[i] & 128) == 128) {
					continue;
				}

				if (strstr(data.idMapA[varInt].c_str(),"minecraft:air") == nullptr) {
					BlockData bd;
					bd.id = varInt;
					bd.variant = blockRTIDMap.rt_schemfile;
					bd.x = sx + block_index % dx;
					bd.y = sy + block_index / (dz * dx);
					bd.z = sz + (block_index % (dz * dx)) / dx;
					data.pushData(bd);
				}
				varInt = 0;
				varIntLength = 0;
				block_index = block_index + 1;
			}


		}
		data.prepare();
	}
};
class BDXParser {
public:
	std::string bdx;
	int b_index = -1;

	int px;
	int py;
	int pz;
	int rt_aux = 0;

	void readHeader() {
		b_index = b_index + 4;
	}
	int8_t readInt8() {
		int8_t r;
		b_index = b_index + 1;
		if (b_index >= bdx.size()) {
			r = 0;
		}
		else {
			r = bdx[b_index];
		}
		return r;
	}

	uint8_t readUInt8() {
		return (uint8_t)readInt8();
	}

	char readCMD() {
		return readUInt8();
	}
	std::string readString() {
		std::string result;
		while (true) {
			char c = readUInt8();
			if (c == 0) {
				break;
			}
			else {
				result.push_back(c);
			}

		}

		return result;
	}



	short readShort() {
		char buf[2];
		char a = readUInt8();
		char b = readUInt8();
		buf[0] = b;
		buf[1] = a;
		return *(short*)buf;
	}

	unsigned short readUShort() {
		return (unsigned short)readShort();
	}

	int readInt() {
		char buf[4];
		char a = readUInt8();
		char b = readUInt8();
		char c = readUInt8();
		char d = readUInt8();
		buf[0] = d;
		buf[1] = c;
		buf[2] = b;
		buf[3] = a;

		return *(int*)buf;
	}

	unsigned int readUInt() {
		return (unsigned int)readInt();
	}

	void readPlayerName() {
		readString();
	}

	void initBDXData() {
		readHeader();
		readString();

		/*
		uint8_t t = bdx.back();
		if (t == 90) {
			bdx.pop_back();
			uint16_t len = bdx.back();
			bdx.pop_back();
			if (len == 255) {
				char K[2];
				K[1] = bdx.back();
				bdx.pop_back();
				K[0] = bdx.back();
				bdx.pop_back();

				len = *(uint16_t*)K;
			}
			for (uint32_t i = 0; i != len; i = i + 1) {
				bdx.pop_back();
			}
		}
		*/
	}

	Json::Value readCommandData() {
		Json::Value cmdData;
		cmdData["x"] = px;
		cmdData["y"] = py;
		cmdData["z"] = pz;
		cmdData["id"] = "CommandBlock";
		cmdData["Mode"] = readUInt();
		cmdData["Command"] = readString();
		cmdData["CustomName"] = readString();
		cmdData["LastOutput"] = readString();
		cmdData["TickDelay"] = readInt();
		cmdData["ExecuteOnFirstTick"] = readUInt8();
		cmdData["TrackOutput"] = readUInt8();
		cmdData["isConditional"] = readUInt8();
		cmdData["redstone"] = readUInt8();
		return cmdData;
	}

	Json::Value readItem() {
		std::string itemName = readString();
		uint8_t count = readUInt8();
		unsigned short DataValue = readUShort();
		uint8_t slot = readUInt8();
		Json::Value Item;
		Item["Slot"] = slot;
		Item["Name"] = itemName;
		Item["Count"] = count;
		Item["DataValue"] = DataValue;
		return Item;
	}

	Json::Value readChest() {
		Json::Value chestData;
		chestData["x"] = px;
		chestData["y"] = py;
		chestData["z"] = pz;
		uint8_t slotCount = readUInt8();
		for (int i = 0; i != slotCount; i = i + 1) {
			chestData["Items"][i] = readItem();
		}
		return chestData;
	}

	BlockData readBlockData() {
		auto bid = readUShort();
		BlockData bd;
		bd.id = bid;
		bd.variant = readUShort();
		bd.x = px;
		bd.y = py;
		bd.z = pz;
		return bd;
	}

	BlockDataV2 readBlockDataV2() {
		auto bid = readUShort();
		BlockDataV2 bd;
		bd.id = bid;
		bd.blockState = readString();
		bd.x = px;
		bd.y = py;
		bd.z = pz;
		return bd;
	}

	BlockData readRT_S() {
		BlockData bd;
		bd.x = px;
		bd.y = py;
		bd.z = pz;
		bd.variant = rt_aux;
		bd.id = readUShort();
		return bd;
	}
	BlockData readRT_I() {
		BlockData bd;
		bd.x = px;
		bd.y = py;
		bd.z = pz;
		bd.variant = rt_aux;
		bd.id = readUInt();
		return bd;
	}

	BDXParser(std::string const& filePath,int sx,int sy,int sz,MapData & data) {
		FILE* fp = fopen(filePath.c_str(), "rb");
		std::string header = "BD@";
		fseek(fp, header.size(), SEEK_SET);
		struct stat fstat;
		stat(filePath.c_str(), &fstat);
		int fsize = fstat.st_size - header.size();
		uint8_t* buf = new uint8_t[fsize];
		fread(buf, fsize, 1, fp);
		delete[] buf;
		fclose(fp);

		bdx = brotli::decompress(buf, fsize);

		initBDXData();

		px = sx;
		py = sy;
		pz = sz;


		class CommandBlockNew {
		public:
			int aux;
			int x;
			int y;
			int z;
		};
		std::vector<CommandBlockNew> cmds;

		int blockID = 0;
		while (1) {
			int cmd = readUInt8();

			if (cmd == 1) {
				std::string blockName = readString();
				data.idMapA[blockID] = blockName;
				blockID = blockID + 1;
			}
			else if (cmd == 2) {
				px = px + readUShort();
				py = sy;
				pz = sz;
			}
			else if (cmd == 3) {
				px = px + 1;
				py = sy;
				pz = sz;
			}
			else if (cmd == 4) {
				py = py + readUShort();
				pz = sz;
			}
			else if (cmd == 5) {
				auto bd = readBlockData();
				data.pushBlockWithState_(bd);
			}
			else if (cmd == 6) {
				pz = pz + readUShort();
			}
			else if (cmd == 7) {
				auto bd = readBlockData();
				data.pushData(bd);
			}
			else if (cmd == 8) {
				pz = pz + 1;
			}
			else if (cmd == 9) {
				//NOP
			}
			else if (cmd == 0xA) {
				px = px + readUInt();
				py = sy;
				pz = sz;
			}
			else if (cmd == 0xB) {
				py = py + readUInt();
				pz = sz;
			}
			else if (cmd == 0xC) {
				pz = pz + readUInt();
			}
			else if (cmd == 0xD) {
				auto bd = readBlockDataV2();
				data.pushBlockWithState(bd);
			}
			else if (cmd == 0xE) {
				px = px + 1;
			}
			else if (cmd == 0xF) {
				px = px - 1;
			}
			else if (cmd == 0x10) {
				py = py + 1;
			}
			else if (cmd == 0x11) {
				py = py - 1;
			}
			else if (cmd == 0x12) {
				pz = pz + 1;
			}
			else if (cmd == 0x13) {
				pz = pz - 1;
			}
			else if (cmd == 0x14) {
				px = px + readShort();
			}
			else if (cmd == 0x15) {
				px = px + readInt();
			}
			else if (cmd == 0x16) {
				py = py + readShort();
			}
			else if (cmd == 0x17) {
				py = py + readInt();
			}
			else if (cmd == 0x18) {
				pz = pz + readShort();
			}
			else if (cmd == 0x19) {
				pz = pz + readInt();
			}
			else if (cmd == 0x1A) {
				data.addBlockEntity(readCommandData());
			}
			else if (cmd == 0x1B) {
				data.pushData(readBlockData());
				data.addBlockEntity(readCommandData());
			}
			else if (cmd == 0x1C) {
				px = px + readInt8();
			}
			else if (cmd == 0x1D) {
				py = py + readInt8();
			}
			else if (cmd == 0x1E) {
				pz = pz + readInt8();
			}
			else if (cmd == 0x1F) {
				uint8_t v = readUInt8();

				if (v == 117) {
					rt_aux = blockRTIDMap.rt_aux117;
				}
				else if (v == 118) {
					rt_aux = blockRTIDMap.rt_aux118;
				}
			}
			else if (cmd == 0x20) {
				data.pushData(readRT_S());
			}
			else if (cmd == 0x21) {
				data.pushData(readRT_I());
			}
			else if (cmd == 0x22) {
				data.pushData(readRT_S());
				data.addBlockEntity(readCommandData());
			}
			else if (cmd == 0x22) {
				data.pushData(readRT_I());
				data.addBlockEntity(readCommandData());
			}
			else if (cmd == 0x24) {
				CommandBlockNew cmd{ readUShort(),px,py,pz };
				data.addBlockEntity(readCommandData());
				cmds.push_back(cmd);
			}
			else if (cmd == 0x25) {
				data.pushData(readRT_S());
				data.addBlockEntity(readChest());
			}
			else if (cmd == 0x26) {
				data.pushData(readRT_I());
				data.addBlockEntity(readChest());
			}
			else if (cmd == 0x27) {
				uint32_t len = readUInt();
				for (uint32_t i = 0; i != len; i = i + 1) {
					readUInt8();
				}
			}
			else if (cmd == 0x28) {
				auto bd = readBlockData();
				data.pushData(bd);
				data.addBlockEntity(readChest());
			}
			else if (cmd == 0x29) {
				//nbt(not supported)
			}
			else if (cmd == 0x58) {
				break;
			}
			else {
				break;
			}		
		}

		int cmd_bid = blockID + 1;
		if (data.idMapA.hasBlockID("command_block")) {
			cmd_bid = data.idMapA.getBlockID("command_block");
		}
		else if (data.idMapA.hasBlockID("minecraft:command_block")) {
			cmd_bid = data.idMapA.getBlockID("minecraft:command_block");
		}
		else {
			data.idMapA[cmd_bid] = "command_block";
		}
		for (auto i : cmds) {
			BlockData bd(i.x,i.y,i.z,cmd_bid,i.aux);
			data.pushData(bd);
		}
		data.prepare();
	}
};

FastBuilder::FastBuilder() {
	
}

class AreaEvent {
public:
	std::list<MapData::Area>::iterator area;
	int tick = 0;
	std::vector<mce::UUID> uuidMap;

	AreaEvent(std::list<MapData::Area>::iterator area) {
		this->area = area;
	}

	
	void sendCheckCommand(LocalPlayer* lp) {

		std::stringstream testforblocks;
		if (lp->getDimension()->getDimensionId() == 0) {
			testforblocks << "testforblocks " << area->x * 64 << " -64 " << area->z * 64 << " " << area->x * 64 + 63 << " 319 " << area->z * 64 + 63<<" "<<area->x*64 << " -64 " << area->z * 64;
		}
		else if (lp->getDimension()->getDimensionId() == 1) {
			testforblocks << "testforblocks " << area->x * 64 << " 0 " << area->z * 64 << " " << area->x * 64 + 63 << " 127 " << area->z * 64 + 63 << " " << area->x * 64 << " 0 " << area->z * 64;
		}
		else if (lp->getDimension()->getDimensionId() == 2) {
			testforblocks << "testforblocks " << area->x * 64 << " 0 " << area->z * 64 << " " << area->x * 64 + 63 << " 255 " << area->z * 64 + 63 << " " << area->x * 64 << " 0 " << area->z * 64;
		}
		
		auto uuid = mce::UUID::random();
		uuidMap.push_back(uuid);

		CommandRequest request(uuid, testforblocks.str(), CommandOrigin::CommandOriginAutomationPlayer, 0, false, false);
		HackSDK::sendOriginalPacket(request);
	}
	
	bool PlayerTick(LocalPlayer* lp) {
		if (area->fullyLoaded) {
			return true;
		}
		if (tick == 0) {
			FBLib::teleportTo(area->x * 64 + 32, 100, area->z * 64 + 32);
			sendCheckCommand(lp);
		}


		if (tick == 100) {
			tick = 0;
		}
		tick = tick + 1;

		return false;
	}

	void onPacket(CommandOutput const& packet) {
		bool find = false;
		for (auto i : uuidMap) {
			if (i.ab == packet.uuid.ab && i.cd == packet.uuid.cd) {
				find = true;
				break;
			}
		}
		if(find){
			if (packet.CommandOutputMessage[0].message == "commands.compare.tooManyBlocks") {
				area->fullyLoaded = true;
			}
			else {
				FBLib::teleportTo(area->x * 64 + 32, 100, area->z * 64 + 32);
				sendCheckCommand(mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer());
			}
		}
	}
};

class PosEvent {
public:
	int x, y, z;
	bool op;
	int tick = 0;
	PosEvent(LocalPlayer * lp,int x, int y, int z,bool op) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->op = op;
	}

	bool PlayerTick(LocalPlayer * lp) {
		if(FBLib::hasChunk(lp->getRegion(),x,y,z)){
			return true;
		}
		if (tick == 0 && op) {
			FBLib::teleportTo(x + 100, y, z + 100);
		}
		if (tick == 5 && op) {
			FBLib::teleportTo(x, y, z);
		}
		tick = tick + 1;

		if (tick == 60) {
			tick = 0;
		}

		return false;
	}
};

void FastBuilder::OnTick(Actor* act)
{
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if (act != lp)return;

	if (posEvent) {
		if (posEvent->PlayerTick(lp) == true) {
			delete posEvent;
			posEvent = nullptr;
		}
		else {
			return;
		}
	}
	if (areaEvent) {
		if (areaEvent->PlayerTick(lp) == true) {
			delete areaEvent;
			areaEvent = nullptr;
		}
		else {
			return;
		}
	}
	std::lock_guard<std::mutex> lk(mtx);

	if (event) {
		BlockSource* bs = lp->getRegion();
		int lpx = (int)lp->PosData.from.x;
		int lpz = (int)lp->PosData.from.z;
		int fetched = 0;
		for (auto it = event->Locate.begin(); it != event->Locate.end(); it = it + 1) {
			bool full = true;
			for (int iy = event->y; iy <= event->dy; iy = iy + 1) {
				if (FBLib::hasChunk(bs, it->x, iy, it->z) == false) {
					full = false;
					break;
				}
			}
			if (full) {
				fetched = fetched + 1;
				if (fetched == 100000 / (event->dy - event->y)) {
					return;
				}
				for (int iy = event->y; iy <= event->dy; iy = iy + 1) {
					if (FBLib::hasChunk(bs, it->x, iy, it->z)) {
						BlockPos bp(it->x, iy, it->z);

						BlockData extraBlockData = FBLib::getExtraBlockData(bs, it->x, iy, it->z, event->x, event->y, event->z);
						if (extraBlockData.id != 0) {
							event->add(extraBlockData);
						}

						BlockData bd = FBLib::getBlockData(bs, it->x, iy, it->z, event->x, event->y, event->z);
						if (bd.id != 0) {

							event->add(bd);
							BlockActor* be = bs->getBlockEntity(bp);
							if (be) {
								CompoundTag tag;
								be->save(tag);
								event->addBlockEntityData(bp, tag, bs);
							}
						}
					}
				}
				event->Locate.erase(it);
				it = it - 1;
			}
		}

		if (event->Locate.size() == 0) {
			event->saveToFile();
			delete event;
			event = nullptr;
			moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
				mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("建筑已保存!", "");
			});
		}
		else {
			for (auto it = event->Locate.begin(); it != event->Locate.end(); it = it + 1) {
				for (int iy = event->y; iy <= event->dy; iy = iy + 1) {
					if (FBLib::hasChunk(bs, it->x, iy, it->z) == false) {
						posEvent = new PosEvent(lp,it->x, iy, it->z,event->op);
						return;
					}
				}
			}
		}
	}
	if (importEvent) {
		entityCheckTick = entityCheckTick + 1;
		if (entityCheckTick > 8) {
			entityCheckTick = 0; 
			if (HackSDK::getActorList().size() > 50) {
				FBLib::executeCommand("/kill @e[type=item]");
			}
		}
		
		
	}
}

void FastBuilder::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".DumpBlockData") {
		moduleManager->executedCMD = true;

		if (event != nullptr) {
			mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("请等待当前保存建筑完成!", "");
		}
		else if (cmd->size() > 7) {
			bool op = true;

			if (cmd->size() > 8 && (*cmd)[8] == "false") {
				op = false;
			}
			event = new DumpEvent((*cmd)[1], atoi((*cmd)[2].c_str()), atoi((*cmd)[3].c_str()), atoi((*cmd)[4].c_str()), atoi((*cmd)[5].c_str()), atoi((*cmd)[6].c_str()), atoi((*cmd)[7].c_str()),op);
			mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("准备保存建筑!", "");

			return;
		}
	}
	if ((*cmd)[0] == ".ImportBigSchematicData") {
		moduleManager->executedCMD = true;
		if (importEventProvider || importEvent) {
			mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("请等待当前导入建筑完成!", "");
		}
		else if (cmd->size() > 4) {
			if (moduleManager->getModule<Chat>())moduleManager->getModule<Chat>()->HideChat = true;
			auto res = Utils::split((*cmd)[1], ".");
			std::string fileType = res[res.size() - 1];
			if (fileType == "schematic") {
				int v = 5000;
				if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getDimension()->getDimensionId() != 0) {
					v = 20;
				}
				if (cmd->size() > 5) {
					v = atoi((*cmd)[5].c_str());
				}
				auto cmd2 = *cmd;
				startImport();
				std::thread t([=]() {
					importEventProvider = new BigSchematicImporter(cmd2[1], atoi(cmd2[2].c_str()), atoi(cmd2[3].c_str()), atoi(cmd2[4].c_str()), v);
					});
				t.detach();
			}
			else {
				mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("非建筑文件!", "");
			}
		}
	}
	if ((*cmd)[0] == ".ImportBlockData") {


		moduleManager->executedCMD = true;

		if (importEventProvider || importEvent) {
			mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("请等待当前导入建筑完成!", "");
		}
		else if (cmd->size() > 4) {
			if(moduleManager->getModule<Chat>())moduleManager->getModule<Chat>()->HideChat = true;

			auto res = Utils::split((*cmd)[1], ".");
			std::string fileType = res[res.size() - 1];

			int v = 5000;

			if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getDimension()->getDimensionId() != 0) {
				v = 2000;
			}

			if (cmd->size() > 5) {
				v = atoi((*cmd)[5].c_str());
			}
			auto cmd2 = *cmd;
			if (fileType == "busj") {
				startImport();
				std::thread t([=]() {
					importEvent = new ImportEvent(cmd2[1], atoi(cmd2[2].c_str()), atoi(cmd2[3].c_str()), atoi(cmd2[4].c_str()), v);

					});
				t.detach();
			}
			else if (fileType == "bdx") {
				startImport();
				std::thread t([=]() {
					auto ie = new ImportEvent(v);
					BDXParser parser(cmd2[1], atoi(cmd2[2].c_str()), atoi(cmd2[3].c_str()), atoi(cmd2[4].c_str()),ie->data);

					importEvent = ie;
				});
				t.detach();
				
			}
			else if (fileType == "png" || fileType == "jpg" || fileType == "jpeg" || fileType == "bmp" || fileType == "webp") {
				startImport();
				std::thread t([=]() {
					auto ie = new ImportEvent(v);
					bool m = true;
					int min_y = atoi(cmd2[3].c_str());

					if (cmd2.size() > 6) {
						if (cmd2[6] == "false") {
							m = false;
						}
					}
					if (cmd2.size() > 7) {
						min_y = atoi(cmd2[7].c_str());
					}

					ImageParser parser(cmd2[1], atoi(cmd2[2].c_str()), min_y, atoi(cmd2[4].c_str()), ie->data,m,min_y);
					importEvent = ie;
				});
				t.detach();
			}
			else if (fileType == "schematic" || fileType == "schem") {
				startImport();
				std::thread t([=]() {
					auto ie = new ImportEvent(v);
					SchematicParser parser(cmd2[1], atoi(cmd2[2].c_str()), atoi(cmd2[3].c_str()), atoi(cmd2[4].c_str()), ie->data,fileType);
					importEvent = ie;
					});
				t.detach();
			}
			else {
				mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("非建筑文件!", "");
			}
		}
	}
}


void FastBuilder::OnMCMainThreadTick(MinecraftGame* m)
{
	if (importEventProvider) {
		if (importEvent == nullptr) {
			importEvent = importEventProvider->getEvent();
			if (importEvent == nullptr) {
				delete importEventProvider;
				importEventProvider = nullptr;
				m->getPrimaryGuiData()->displaySystemMessage("大型建筑导入完毕", "");
			}
		}
	}

	if (importEvent) {
		importTick();
	}
}

void FastBuilder::OnRender(MinecraftUIRenderContext* ctx)
{
	std::lock_guard<std::mutex> lk(mtx);
	if (event) {
		std::stringstream os;
		os << "§l§4正在导出建筑，剩余" << event->Locate.size() * abs(event->y - event->dy) << "个方块";
		HackSDK::drawText(*ctx, vec2_t(10.0f, 60.0f), os.str(), mce::Color(0, 0, 0), mGameData.getNowMinecraftGame()->getUnicodeFontHandle()->getFont());
	}
	else if (importEvent) {
		std::stringstream os;
		os << "§l§4正在导入建筑，剩余" << importEvent->data.getBlockNum()<< "个方块";
		HackSDK::drawText(*ctx, vec2_t(10.0f, 60.0f), os.str(), mce::Color(0, 0, 0), mGameData.getNowMinecraftGame()->getUnicodeFontHandle()->getFont());
	}
	if (importEventProvider) {
		std::stringstream os;
		os << "§l§4正在导入大型建筑，当前进度" << importEventProvider->getProcess() <<"%";
		HackSDK::drawText(*ctx, vec2_t(10.0f, 80.0f), os.str(), mce::Color(0, 0, 0), mGameData.getNowMinecraftGame()->getUnicodeFontHandle()->getFont());
	}
	if (posEvent) {
		std::stringstream os;
		os << "§l§b请前往 X:" << (int)posEvent->x << " Y:" << (int)posEvent->y << " Z:" << (int)posEvent->z << " 附近";
		LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
		HackSDK::drawText(*ctx, vec2_t(10.0f, 70.0f), os.str(), mce::Color(0, 0, 0), mGameData.getNowMinecraftGame()->getUnicodeFontHandle()->getFont());
		glmatrixf* matrix = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix();
		vec3_t cam((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
		vec2_t fov = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getFov();
		float screenWidth = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenWdith;
		float screenHeight = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenHeight;
		vec2_t display(screenWidth, screenHeight);
		vec2_t start(screenWidth / 2.0f, screenHeight / 2.0f);
		vec2_t end;
		vec3_t origin((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
		mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix()->OWorldToScreen(origin, vec3_t(posEvent->x, posEvent->y, posEvent->z), end, fov, display);
		HackSDK::drawLine(*ctx, start, end, 0.75f, mce::Color(0, 1.0f, 0));
	}
}

void FastBuilder::MinecraftInit()
{
	std::thread t([=]() {
		std::stringstream path;
		path << "/data/data/" << mGameData.getPackageName() << "/files/games/com.netease/minecraftpe/options.txt";
		while (1) {
			usleep(100);
			std::string s = Utils::readFileIntoString(path.str().c_str());
			if (strstr(s.c_str(), "gfx_viewdistance")) {
				if (strstr(s.c_str(), "gfx_viewdistance:144")) {
					return;
				}
				else {
					auto sv = Utils::split(s, "\n");
					for (int i = 0; i != sv.size();i = i + 1) {
						if (strstr(sv[i].c_str(), "gfx_viewdistance")) {
							sv[i] = "gfx_viewdistance:144";
						}
						else if (strstr(sv[i].c_str(), "gfx_graphics_level_user_define")) {
							sv[i] = "gfx_graphics_level_user_define:1";
						}
					}
					Utils::DeleteFile(path.str().c_str());
					for (auto i : sv) {
						Utils::WriteStringToFile(path.str().c_str(), i);
					}
					abort();
				}
			}
		}
	});
	t.detach();
}


void FastBuilder::importTick()
{
	if (shouldInitTime) {
		gettimeofday(&lastTime, nullptr);
		shouldInitTime = false;
		return;
	}
	struct timeval time;
	gettimeofday(&time, nullptr);

	long us = (time.tv_sec - lastTime.tv_sec) * 1000000 + time.tv_usec - lastTime.tv_usec;
	if (us < 1000) {
		return;
	}
	else {
		float ms = us / 1000.0f;
		float p = 1000.0f / (float)importEvent->bps;
		if (p > ms) {
			return;
		}


		int b = ms / p;


		updateBlock(b);
		lastTime = time;
		
	}
}

void FastBuilder::updateBlock(int n)
{
	if (areaEvent) {
		return;
	}

	if (importEvent->data.getBlockNum() == 0) {
		delete importEvent;
		importEvent = nullptr;
		mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("建筑已导入!", "");
		finishImport();
		return;
	}

	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	BlockSource* bs = lp->getRegion();
	

	int imported = 0;

	if (importEvent->data.getCurrentAreaBlockNum() == 0) {
		importEvent->data.newArea();
	}


	if (importEvent->data.currentArea->fullyLoaded) {
		while (true) {
			void* data = nullptr;
			int  type = importEvent->data.nextBlock(&data);
			if (type == 1) {
				BlockData* data_ = (BlockData*)data;
				FBLib::setBlock(*data_, importEvent->data);
				delete data_;
			}
			else if (type == 2) {
				BlockDataV2* data_ = (BlockDataV2*)data;
				FBLib::setBlock(*data_, importEvent->data);
				delete data_;
			}
			else if (type == 3) {
				BlockData* data_ = (BlockData*)data;
				BlockDataV2 data__;
				data__.x = data_->x;
				data__.y = data_->y;
				data__.z = data_->z;
				data__.id = data_->id;
				data__.blockState = importEvent->data.idMapA[data_->variant];
				FBLib::setBlock(data__, importEvent->data);

				delete data_;
			}

			if (importEvent->data.getCurrentAreaBlockNum() == 0) {

				for (auto i : importEvent->data.currentArea->blockEntityData) {
					int x = i["x"].asInt();
					int y = i["y"].asInt();
					int z = i["z"].asInt();

					auto id = i["id"].asString();
					if (id == "Sign") {
						CompoundTag tag;
						tag.putString("Text", i["Text"].asString());
						tag.putInt("x", x);
						tag.putInt("y", y);
						tag.putInt("z", z);
						tag.putBoolean("isMovable", true);
						tag.putString("id", "Sign");
						tag.putString("TextOwner", "VS");
						tag.putBoolean("IgnoreLighting", false);
						tag.putInt("SignTextColor", -16777216);
						BlockPos bp(x, y, z);
						BlockActorDataPacket packet(bp, tag);
						lp->sendNetworkPacket(packet);
					}
					else if (i.isMember("Items")) {
						Json::Value Items = i["Items"];
						for (auto t : Items) {
							std::stringstream os;
							os << "replaceitem block " << x << " " << y << " " << z << " slot.container " << t["Slot"].asInt() << " " << t["Name"].asString() << " " << t["Count"].asInt() << " " << t["DataValue"].asInt();
							FBLib::executeCommand(os.str());
						}
					}
					else if (id == "CommandBlock") {
						CommandBlockUpdate update(x, y, z, i["Mode"].asInt(), i["TickDelay"].asInt(), i["redstone"].asBool(), i["isConditional"].asBool(), i["TrackOutput"].asBool(), i["ExecuteOnFirstTick"].asBool(), i["Command"].asString(), i["LastOutput"].asString(), i["CustomName"].asString());
						HackSDK::sendOriginalPacket(update);
					}
				}

				break;
			}

			imported = imported + 1;

			if (imported >= n) {
				return;
			}
		}

	}
	else {
		areaEvent = new AreaEvent(importEvent->data.currentArea);
	}

	

}

void FastBuilder::startImport()
{
	shouldInitTime = true;
	mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("准备导入建筑,请耐心等待,请勿移动直至导入完成!", "");
}

void FastBuilder::finishImport()
{

}

void FastBuilder::OnReceiveData(std::string* data)
{
	CommandOutput out;
	BinaryPacketReader reader(data);
	if (out.read(reader)) {
		if (areaEvent) {
			areaEvent->onPacket(out);
		}
	}
}

void FastBuilder::OnSendPacket(Packet* packet)
{

	if (importEvent) {
		if (packet->getName() == "PlayerAuthInputPacket" || packet->getName() == "MovePlayerPacket") {
			MCHook::noPacketThisTime = true;
		}
	}
}
