#include "Android.h"
#include "Utils.hpp"
#include "MCPE/GameData.h"
#include <stdio.h>
#include <jni.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stddef.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <string>
#include <mutex>
#include "json/json.h"
#include <sstream>
#include "MCPE/GameData.h"
#include "Utils.hpp"
#include "Android.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "Raknet/SuperFastHash.h"
#include <thread>

Android* mAndroid = nullptr;

class Interface {
public:
	bool ui_isStopped = false;

	void changeState(bool b) {
		
		ui_isStopped = b;
	}

	Interface(void) {
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		sockaddr_in  local_addr = { 0 };
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(11111);
		local_addr.sin_addr.s_addr = INADDR_ANY;
		int ret = ::bind(sockfd, (sockaddr*)&local_addr, sizeof(local_addr));
		if (ret == -1) {
			puts("UDPServer bind error");
		}
		DataBuffer = new char[MaxDataSize];

		std::thread([=]() {
			while (true) {
				usleep(100000);
				if (ui_isStopped == false) {
					Json::Value val;
					mtx.lock();
					if (l.size() == 0) {
						mtx.unlock();
						continue;
					}
					for (int i = 0; i < l.size(); i = i + 1) {
						val[i] = l[i];
					}
					l.clear();
					mtx.unlock();
					send(val);
				}
			}
		}).detach();

	}
	~Interface(void) {
		::close(sockfd);
		delete[] DataBuffer;
	}
	std::string recv(void) {
		sockaddr_in addr;
		memset(DataBuffer, 0, MaxDataSize);
		socklen_t len = sizeof(addr);
		int ret = ::recvfrom(sockfd, DataBuffer, MaxDataSize, 0, (sockaddr*)&addr, &len);
		if (ret == -1) {
			return "";
		}
		else {
			return DataBuffer;
		}
	}
	Json::Value recvJson(void) {
		Json::Reader reader;
		Json::Value value;
		reader.parse(recv(), value);
		return value;
	}
	void send(std::string s) {
		sockaddr_in src_addr;
		src_addr.sin_family = AF_INET;
		src_addr.sin_port = htons(11112);
		src_addr.sin_addr.s_addr = INADDR_ANY;
		src_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		socklen_t len = sizeof(src_addr);
		::sendto(sockfd, s.c_str(), s.size(), 0, (sockaddr*)&src_addr, len);
	}
	void send(Json::Value v) {
		send(v.toStyledString());
	}
	void add(Json::Value v) {
		mtx.lock();
		l.push_back(v);
		mtx.unlock();
	}
	std::mutex mtx;
	std::vector<Json::Value> l;
	char* DataBuffer;
	const int MaxDataSize = 1024 * 1024;
	int sockfd;

};


Android::Android(JavaVM* currentVM)
{
	vm = currentVM;

	interface = new Interface;
	std::thread([=]() {
		while (true) {
			Json::Value val = interface->recvJson();
			handleJson(val);
			usleep(2000);
		}
	}).detach();
}

void Android::loadLibrary(std::string const& libname)
{
	std::stringstream bufferPath;
	bufferPath << "/data/data/" << GameData::getPackageName() << "/MyLibs";
	if (opendir(bufferPath.str().c_str()) == 0) {
		Utils::folder_mkdirs(bufferPath.str().c_str());
	}

	auto sv = Utils::split(libname, "/");
	std::string fileName = sv[sv.size() - 1];

	std::stringstream realPath;
	realPath << bufferPath.str() << "/" << fileName;

	if (access(realPath.str().c_str(), F_OK) == 0) {
		remove(realPath.str().c_str());
	}

	Utils::copyFile(libname.c_str(), realPath.str().c_str());

	chmod(realPath.str().c_str(), 0777);

	JNIEnvManager manager(vm);
	by_dlopen(manager.getEnv(), realPath.str().c_str(),0);
	

	remove(realPath.str().c_str());

}


std::string Android::getHWId()
{
	FILE* res = popen("stat -f /", "r");
	char res_cache[8][1024];
	char res_final[1024];
	memset(res_final, 0, 1024);
	int line = 0;
	while (fgets(res_cache[line], 1024, res) != NULL) {
		line++;
	}
	pclose(res);
	sscanf(res_cache[1], " ID: %s ", &res_final);

	const char* uidPath = "/sdcard/nsc.dat";
	if (access(uidPath, F_OK) != 0) {
		srand(time(0));
		int uid = rand();
		Utils::WriteStringToFile(uidPath, Utils::asString(uid));
	}

	std::stringstream full;
	full << Utils::readFileIntoString("/sys/class/net/wlan0/address") << Utils::readFileIntoString("/proc/cmdline") << res_final << Utils::readFileIntoString(uidPath);
	uint32_t hash = SuperFastHash(full.str().c_str(), full.str().size());

	return Utils::asString(hash);
}


void Android::setMediaVolume(int v)
{
	Json::Value val;
	val["Action"] = "setMediaVolume";
	val["ID"] = v;
	interface->add(val);
}

void Android::playMusic(std::string const& path)
{
	Json::Value val;
	val["Action"] = "playMusic";
	val["ID"] = 0;
	val["Path"] = path;
	interface->add(val);
}

void Android::Toast(std::string s)
{
	Json::Value val;
	val["Action"] = "Toast";
	val["ID"] = 0;
	val["a0"] = s;
	interface->add(val);
}

void Android::handleJson(Json::Value const& val)
{

	uint64_t id = val["ID"].asInt64();
	if (val["Action"].asString() == "View_onClickListener") {
		View* view = (View*)id;
		if (view->onClickListener != nullptr) {
			view->onClickListener(view);
		}
	}
	else if (val["Action"].asString() == "View_onLongClickListener") {
		View* view = (View*)id;
		if (view->onLongClickListener != nullptr) {
			view->onLongClickListener(view);
		}
	}
	else if (val["Action"].asString() == "EditText_textChangedListener") {
		EditText* et = (EditText*)id;
		et->handle(val["a0"].asString());
	}
	else if (val["Action"].asString() == "Switch_onCheckedChangeListener") {
		Switch* sw = (Switch*)id;
		if (sw->onCheckedChangeListener != nullptr) {
			sw->onCheckedChangeListener(sw, val["State"].asBool());
		}
	}
}

JNIEnvManager::JNIEnvManager(JavaVM* vm)
{
	vm0 = vm;
}

JNIEnv* JNIEnvManager::getEnv()
{
	JNIEnv* jni_env;
	vm0->GetEnv((void**)&jni_env, JNI_VERSION_1_6);
	if (jni_env == nullptr) {
		need_detach = true;
		vm0->AttachCurrentThread(&jni_env, NULL);
	}
	return jni_env;
}

JNIEnvManager::~JNIEnvManager()
{
	if (need_detach) {
		vm0->DetachCurrentThread();
	}
}

Android::Window* Android::newWindow(void) {
	return new Window(this);
}

Android::Window::Window(Android* a)
{
	
	android = a;
	Json::Value val;
	val["Action"] = "Window_newWindow";
	val["ID"] = (int64_t)this;
	android->interface->add(val);
}

void Android::Window::noLimit(bool v) {
	Json::Value val;
	val["Action"] = "Window_noLimit";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}

void Android::Window::setCanMove(bool v) {
	Json::Value val;
	val["Action"] = "Window_setCanMove";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}
void Android::Window::addView(View const& view) {


	Json::Value val;
	val["Action"] = "Window_addView";
	val["ID"] =(int64_t)this;
	val["a0"] =(int64_t)&view;

	android->interface->add(val);
}
void Android::Window::removeView(View const& view) {


	Json::Value val;
	val["Action"] = "Window_removeView";
	val["ID"] =(int64_t)this;
	val["a0"] =(int64_t)&view;


	android->interface->add(val);
}
Android::Window::~Window()
{
	Json::Value val;
	val["Action"] = "Window_deleteWindow";
	val["ID"] = (int64_t)this;

	android->interface->add(val);
}
void Android::Window::deleteWindow(void) {
	delete this;
}
void Android::Window::setCanShowKeyboard(bool v) {
	Json::Value val;
	val["Action"] = "Window_setCanShowKeyboard";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}

void Android::Window::setTouchable(bool v)
{
	Json::Value val;
	val["Action"] = "Window_setTouchable";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}

void Android::Window::moveTo(int x, int y)
{
	Json::Value val;
	val["Action"] = "Window_moveTo";
	val["ID"] =(int64_t)this;
	val["X"] = x;
	val["Y"] = y;

	android->interface->add(val);
}

void Android::Window::setOrientation(int v) {
	Json::Value val;
	val["Action"] = "Window_setOrientation";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}


Android::Switch* Android::newSwitch(void)
{
	Switch* sw = new Switch(this);
	Json::Value val;
	val["Action"] = "Switch_newSwitch";
	val["ID"] =(int64_t)sw;

	sw->android->interface->add(val);
	return sw;
}

Android::Switch::Switch(Android* a)
{
	android = a;
}

void Android::Switch::updateData(Json::Value v)
{
	Json::Value val;
	val["Action"] = "Switch_updateData";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}

void Android::Switch::updateData(std::string s)
{
	Json::Reader reader;
	Json::Value value;
	reader.parse(s, value);
	updateData(value);
}

void Android::Switch::setShowText(bool v)
{
	Json::Value val;
	val["Action"] = "Switch_setShowText";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}

void Android::Switch::setOnCheckedChangeListener(std::function<void(Switch*, bool)> f)
{
	onCheckedChangeListener = f;
}


Android::Button* Android::newButton(void) {
	Button* btn = new Button(this);
	Json::Value val;
	val["Action"] = "Button_newButton";
	val["ID"] =(int64_t)btn;

	btn->android->interface->add(val);
	return btn;
}
void Android::View::setFocusable(bool v) {
	Json::Value val;
	val["Action"] = "View_setFocusable";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}
Android::Button::Button(Android* a)
{
	android = a;
}
void Android::Button::updateData(Json::Value v) {
	Json::Value val;
	val["Action"] = "Button_updateData";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}

void Android::Button::updateData(std::string s) {
	Json::Reader reader;
	Json::Value value;
	reader.parse(s, value);
	updateData(value);
}

Android::View::~View()
{
	Json::Value val;
	val["Action"] = "View_release";
	val["ID"] = (int64_t)this;

	android->interface->add(val);
}

void Android::View::setOnClickListener(std::function<void(View*)> f)
{
	onClickListener = f;
}

void Android::View::setOnLongClickListener(std::function<void(View*)> f)
{
	onLongClickListener = f;
}

void Android::View::release(void) {
	delete this;
}


Android::TextView* Android::newTextView(void) {
	return new TextView(this);
}

Android::TextView::TextView(Android* a)
{
	android = a;

	Json::Value val;
	val["Action"] = "TextView_newTextView";
	val["ID"] = (int64_t)this;

	android->interface->add(val);
}

void Android::TextView::updateData(Json::Value v) {
	Json::Value val;
	val["Action"] = "TextView_updateData";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}

void Android::TextView::updateData(std::string s) {
	Json::Reader reader;
	Json::Value value;
	reader.parse(s, value);
	updateData(value);
}

Android::EditText* Android::newEditText(void) {
	return new EditText(this);
}

Android::EditText::EditText(Android* a)
{
	android = a;

	Json::Value val;
	val["Action"] = "EditText_newEditText";
	val["ID"] = (int64_t)this;

	android->interface->add(val);
}

void Android::EditText::updateData(Json::Value v) {
	Json::Value val;
	val["Action"] = "EditText_updateData";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}

void Android::EditText::updateData(std::string s) {
	Json::Reader reader;
	Json::Value value;
	reader.parse(s, value);
	updateData(value);
}

void Android::EditText::setTextChangedListener(std::function<void(EditText*, std::string)> f)
{
	textChangedListener = f;
}

void Android::EditText::handle(std::string s)
{
	text = s;
	if (textChangedListener != nullptr) {
		textChangedListener(this, s);
	}
}


Android::ImageView* Android::newImageView(std::string Path) {
	return new ImageView(this,Path);
}


Android::LinearLayout* Android::newLinearLayout(void)
{
	return new LinearLayout(this);
}
Android::LinearLayout::LinearLayout(Android* a)
{
	android = a;
	Json::Value val;
	val["Action"] = "LinearLayout_newLinearLayout";
	val["ID"] = (int64_t)this;
	android->interface->add(val);
}

void Android::LinearLayout::addView(View const& view)
{
	Json::Value val;
	val["Action"] = "LinearLayout_addView";
	val["ID"] =(int64_t)this;
	val["a0"] =(int64_t)&view;

	android->interface->add(val);
}
void Android::LinearLayout::removeView(View const& view)
{
	Json::Value val;
	val["Action"] = "LinearLayout_removeView";
	val["ID"] =(int64_t)this;
	val["a0"] =(int64_t)&view;

	android->interface->add(val);
}
void Android::LinearLayout::setBackground(std::string const& path)
{
	Json::Value val;
	val["Action"] = "LinearLayout_setBackground";
	val["ID"] =(int64_t)this;
	val["a0"] = path;

	android->interface->add(val);
}
void Android::LinearLayout::setOrientation(int v)
{
	Json::Value val;
	val["Action"] = "LinearLayout_setOrientation";
	val["ID"] =(int64_t)this;
	val["a0"] = v;

	android->interface->add(val);
}

Android::ImageView::ImageView(Android* a, std::string const& path)
{
	android = a;

	Json::Value val;
	val["Action"] = "ImageView_newImageView";
	val["ID"] = (int64_t)this;
	val["a0"] = path;

	android->interface->add(val);
}
