#include "Netease.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "Utils.hpp"
#include "Network.h"
#include "HackSDK.h"
#include <thread>
#include "pyport.h"
#include "object.h"
#include "Python.h"

void Delete(void) {
	std::string path1 = "/sdcard/netease";
	std::string path2 = "/data/data/" + mGameData.getPackageName() + "/shared_prefs";
	std::string path4 = "/data/data/" + mGameData.getPackageName() + "/databases";
	std::string path3 = "/sdcard/Android/data/" + mGameData.getPackageName() + "/files/netease";

	Utils::DeleteFile(path1.c_str());
	Utils::DeleteFile(path2.c_str());
	Utils::DeleteFile(path3.c_str());
	Utils::DeleteFile(path4.c_str());
}

Netease::Netease()
{
	ModuleType = "Other";
	UIType = 1;
}


const char* Netease::GetName()
{
	return "Netease";
}



void Netease::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".Delete") {
		Delete();
		moduleManager->executedCMD = true;
	}
	else if ((*cmd)[0] == ".GetIP") {
		moduleManager->executedCMD = true;
		mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(currentIP, "");
	}
	else if ((*cmd)[0] == ".CallPy") {
		if (cmd->size() < 2)return;
		moduleManager->executedCMD = true;
		mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(mGameData.rnCallPython((*cmd)[1]), "");
	}
	else if ((*cmd)[0] == ".LogRnPyMessage") {
		if (cmd->size() < 2)return;
		moduleManager->executedCMD = true;
		if ((*cmd)[1] == "true") {
			hookRnPyMessage = true;
		}
		else if ((*cmd)[1] == "false") {
			hookRnPyMessage = false;
		}
	}
}


int (*old_callPy)(const char* a, const char* b, const char* c, PyObject** str, void* t, const char* d, int, int, int, int);
int now_callPy(const char* a, const char* b, const char* c, PyObject** str, void* t, const char* d, int M, int N, int T, int X) {

	int r = (*old_callPy)(a, b, c, str, t, d, M, N, T, X);
	if (moduleManager->getModule<Netease>()->hookRnPyMessage) {
		if (b == std::string("on_webview_call_rn")) {

			std::stringstream os;
			if (r == 0) {
				os << PyString_AsString(*str) << " " << d;
			}
			else {
				os << "Error:" << r << " " << d;
			}
			Utils::WriteStringToFile("/sdcard/RnCallPython.txt", os.str());
		}
	}
	return r;
}

void Netease::MinecraftInit()
{
	HackSDK::FastHook(mGameData.basePtr.PythonRuntime_xcallFunc, (void*)now_callPy, (void**)&old_callPy);

	FunctionTool PhoneFun;
	PhoneFun.addFunction(mGameData.libinfo.head + mGameData.basePtr.DataUtils_MiscData_getMobileAuthenticated);
	PhoneFun.killAll();
}

void Netease::initViews()
{
	ip = mAndroid->newTextView();
	UIUtils::updateTextViewData(ip, "CurrentIP:", "#FF0000", 19);

	Android::TextView* Netease_delete = mAndroid->newTextView();
	UIUtils::updateTextViewData(Netease_delete, "DeleteUserData", "#FF0000", 19);
	Netease_delete->setOnClickListener([=](Android::View*) {
		if (k == 0) {
			k = 1;
			time_v = time(0);
			mAndroid->Toast("再次点击确认删除!");
		}
		else if (k == 1) {
			if (time(0) - time_v < 4) {
				k = 0;
				mAndroid->Toast("正在删除中");
				std::thread thread([=]() {
					Delete();
					mAndroid->Toast("已删除游客数据");
				});
				thread.detach();
			}
			else {
				time_v = time(0);
				mAndroid->Toast("再次点击确认删除!");
			}
		}
	});

	Android::EditText* Netease_Chat = mAndroid->newEditText();
	UIUtils::updateEditTextData(Netease_Chat, "ChatMessage");

	Android::TextView* Netease_send = mAndroid->newTextView();
	UIUtils::updateTextViewData(Netease_send, "SendChat", "#FF0000", 19);
	Netease_send->setOnClickListener([=](Android::View*) {
		
		if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer() == nullptr)return;
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			std::string message = Netease_Chat->text;
			if (message.size() > 0) {
				if (message[0] == '/') {
					HackSDK::sendOriginalPacket(CommandRequest({ 0,0 }, message, CommandOrigin::CommandOriginPlayer,0, false, false));
				}
				else if (message[0] == '.') {
					moduleManager->executeModuleCommand(message);
				}
				else {
					HackSDK::sendOriginalPacket(Text::createChat(mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getName(), message, "1234567", "666"));
					if (moduleManager->getModule<Network>())moduleManager->getModule<Network>()->OnSendChat(message);
				}
			}
		});
	});

	Android::EditText* Netease_PyMessage = mAndroid->newEditText();
	UIUtils::updateEditTextData(Netease_PyMessage, "PyMessage");

	Android::TextView* Netease_CallPy = mAndroid->newTextView();
	UIUtils::updateTextViewData(Netease_CallPy, "CallPy", "#FF0000", 19);
	Netease_CallPy->setOnClickListener([=](Android::View*) {
		mAndroid->Toast(mGameData.rnCallPython(Netease_PyMessage->text));
	});

	Android::TextView* Netease_LogRnCallPyMessage = mAndroid->newTextView();
	UIUtils::updateTextViewData(Netease_LogRnCallPyMessage, "LogRnCallPyMessage", "#FF0000", 19);
	Netease_LogRnCallPyMessage->setOnClickListener([=](Android::View*) {
		if (hookRnPyMessage) {
			hookRnPyMessage = false;
			mAndroid->Toast("LogRnCallPyMessage disabled!");
		}
		else {
			hookRnPyMessage = true;
			mAndroid->Toast("LogRnCallPyMessage Path : /sdcard/RnCallPython.txt");
		}
	});

	SecondWindowList.push_back(ip);
	SecondWindowList.push_back(Netease_delete);
	SecondWindowList.push_back(Netease_Chat);
	SecondWindowList.push_back(Netease_send);
	SecondWindowList.push_back(Netease_PyMessage);
	SecondWindowList.push_back(Netease_CallPy);
	SecondWindowList.push_back(Netease_LogRnCallPyMessage);
}

void Netease::OnNewGame(std::string const& serverIP)
{
	currentIP = "CurrentIP:" + serverIP;
	UIUtils::updateTextViewData(ip, currentIP, "#FF0000", 19);
}
