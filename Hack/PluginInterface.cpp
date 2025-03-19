#include "PluginInterface.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"
#include "Script.h"
#include "Utils.hpp"
#include "MCPE/SDK/mce.h"
#include "HackSDK.h"
#include "Script.h"

namespace PluginProxy {
	namespace ScriptP {
		void runString(std::string const& code,std::string const& id) {
			if(moduleManager->getModule<Script>())moduleManager->getModule<Script>()->runString(code,id);
		}
	}
	namespace AndroidP {
		void Toast(std::string s) {
			mAndroid->Toast(s);
		}

		void loadLibrary(std::string const& libname) {
			mAndroid->loadLibrary(libname);
		}


		void setMediaVolume(int v) {
			mAndroid->setMediaVolume(v);
		}
		void playMusic(std::string const& path) {
			mAndroid->playMusic(path);
		}

		namespace ViewP {
			void release(Android::View* view) {
				view->release();
			}

			void setFocusable(Android::View* view,bool v){
				view->setFocusable(v);
			}

			namespace ButtonP {

				Android::Button* newButton() {
					return mAndroid->newButton();
				}

				void updateData(Android::Button* btn,std::string s) {
					btn->updateData(s);
				}
			}

			namespace TextViewP {
				Android::TextView* newTextView() {
					return mAndroid->newTextView();
				}

				void updateData(Android::TextView* tv, std::string s) {
					tv->updateData(s);
				}
			}

			namespace ImageViewP {
				Android::ImageView* newImageView(std::string const& path) {
					return mAndroid->newImageView(path);
				}

			}

			namespace EditTextP {
				Android::EditText* newEditText() {
					return mAndroid->newEditText();
				}

				void updateData(Android::EditText* et, std::string s) {
					et->updateData(s);
				}
			}
		}

		namespace WindowP {

			Android::Window* newWindow() {
				return mAndroid->newWindow();
			}

			void setCanMove(Android::Window* win, bool v) {
				win->setCanMove(v);
			}

			void addView(Android::Window* win, Android::View const& v) {
				win->addView(v);
			}

			void removeView(Android::Window* win, Android::View const& v) {
				win->removeView(v);
			}

			void deleteWindow(Android::Window* win) {
				win->deleteWindow();
			}

			void setCanShowKeyboard(Android::Window* win, bool v) {
				win->setCanShowKeyboard(v);
			}

			void setOrientation(Android::Window* win, int v) {
				win->setOrientation(v);
			}

			void setTouchable(Android::Window* win, bool v) {
				win->setTouchable(v);
			}

			void moveTo(Android::Window* win, int x,int y) {
				win->moveTo(x,y);
			}

		}
	}
	namespace MCPE {
		LocalPlayer* fetchLocalPlayer() {
			return mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
		}

		std::string getPlayerName(Player* p) {
			return p->getName();
		}

		void print(std::string const& txt) {
			mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(txt, "");
		}


		std::vector<Actor*> getActorList() {
			return HackSDK::getActorList();
		}

		bool isPlayer(Actor* act) {
			if (ActorClassTree::isInstanceOf(*act, 63) == true) {
				return true;
			}
			else {
				return false;
			}
		}

		Vec2* getRot(Actor* act) {
			return &act->rot;
		}

		Vec2* getHitbox(Actor* act) {
			return (Vec2*)&act->weidth;
		}

		AABB* getPosData(Actor* act) {
			return (AABB*)&act->PosData;
		}

		void registerOnTick(std::function<void(Actor*)> f) {
			moduleManager->getModule<PluginHelper>()->OnTick_callBack.push_back(f);
		}


		void registerOnSendChat(std::function<void(std::string const&)> f) {
			moduleManager->getModule<PluginHelper>()->OnSendChat_callBack.push_back(f);
		}

		void registerOnNewGame(std::function<void(std::string const&)> f) {
			moduleManager->getModule<PluginHelper>()->OnNewGame_callBack.push_back(f);
		}

		void registerOnCmd(std::function<void(std::vector<std::string>* cmd)> f) {
			moduleManager->getModule<PluginHelper>()->OnCmd_callBack.push_back(f);
		}

		void registerOnAttack(std::function<void(Actor*)> f) {
			moduleManager->getModule<PluginHelper>()->OnAttack_callBack.push_back(f);
		}

		void registerOnBuild(std::function<void(BlockPos const&,int)> f) {
			moduleManager->getModule<PluginHelper>()->OnBuild_callBack.push_back(f);
		}

		void cmdSuccess() {
			moduleManager->executedCMD = true;
		}

		void createBlockPos(BlockPos* bp, Vec3 const& pos) {
			BlockPos a(pos);
			*bp = a;
		}

		void noBuildThisTime() {
			MCHook::noBuildThisTime = true;
		}

		void noPacketThisTime() {
			MCHook::noPacketThisTime = true;
		}

		void registerOnRender(std::function<void(MinecraftUIRenderContext*)> f) {
			moduleManager->getModule<PluginHelper>()->OnRender_callBack.push_back(f);
		}

		void registerOnSendPacket(std::function<void(Packet*)> f) {
			moduleManager->getModule<PluginHelper>()->OnSendPacket_callBack.push_back(f);
		}

		void drawText(MinecraftUIRenderContext* ctx, vec2_t const& pos, std::string const& text, float size) {
			HackSDK::drawText(*ctx, pos,text, mce::Color(0.3f, 0.3f, 0.3f), mGameData.getNowMinecraftGame()->getUnicodeFontHandle()->getFont(), size);
		}

		void drawBox3D(MinecraftUIRenderContext* ctx,vec3_t const& from,vec3_t const& to,float lineWidth,mce::Color const& color) {
			LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
			glmatrixf* matrix = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix();
			vec3_t cam((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
			vec2_t fov = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getFov();
			float screenWidth = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenWdith;
			float screenHeight = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenHeight;
			vec2_t display(screenWidth, screenHeight);
			HackSDK::drawBox3D(*ctx, matrix, cam, fov, display, from, to, lineWidth, color);
		}

		void drawLine3D(MinecraftUIRenderContext* ctx, vec3_t const& start, vec3_t const& end, float lineWidth, mce::Color const& color){
			LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
			glmatrixf* matrix = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix();
			vec3_t cam((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
			vec2_t fov = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getFov();
			float screenWidth = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenWdith;
			float screenHeight = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenHeight;
			vec2_t display(screenWidth, screenHeight);

			HackSDK::drawLine3D(*ctx, matrix, cam, fov, display, start, end, lineWidth, color);
		}

		vec2_t worldToScreen(vec3_t const& pos) {
			float screenWidth = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenWdith;
			float screenHeight = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenHeight;
			vec2_t display(screenWidth, screenHeight);
			vec2_t fov = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getFov();
			vec2_t screenPos;
			LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
			vec3_t origin((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
			mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix()->OWorldToScreen(origin, pos, screenPos, fov, display);
			return screenPos;
		}

		vec2_t getScreenSize() {
			float screenWidth = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenWdith;
			float screenHeight = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenHeight;
			return vec2_t(screenWidth,screenHeight);
		}

		std::string getPacketName(Packet* p) {
			return p->getName();
		}

		void attack(LocalPlayer* lp, Actor* obj) {
			lp->gamemode->attack(*obj);
		}
	}
}



PluginInterface::PluginInterface()
{

	HackSDK::setGlobalPtr("mPluginInterface", this);
	registerFunctions();
}

void PluginInterface::addFunction(std::string const& name, void* ptr)
{
	std::stringstream fullName;
	for (auto it = registerNamespace.begin(); it != registerNamespace.end(); it = it + 1) {
		fullName << *it << "::";
	}
	fullName << name;
	functions.insert(std::pair<std::string, void*>(fullName.str(), ptr));
}

void PluginInterface::registerFunctions()
{
	addFunction("LuaLoader::runString",(void*)PluginProxy::ScriptP::runString);

	registerNamespace = { "Android" };
	addFunction("loadLibrary", (void*)PluginProxy::AndroidP::loadLibrary);
	addFunction("setMediaVolume", (void*)PluginProxy::AndroidP::setMediaVolume);
	addFunction("playMusic", (void*)PluginProxy::AndroidP::playMusic);
	addFunction("Toast", (void*)PluginProxy::AndroidP::Toast);

	registerNamespace = { "Android","View"};
	addFunction("release", (void*)PluginProxy::AndroidP::ViewP::release);
	addFunction("setFocusable", (void*)PluginProxy::AndroidP::ViewP::setFocusable);

	registerNamespace = { "Android","View" ,"Button"};
	addFunction("newButton", (void*)PluginProxy::AndroidP::ViewP::ButtonP::newButton);
	addFunction("updateData", (void*)PluginProxy::AndroidP::ViewP::ButtonP::updateData);


	registerNamespace = { "Android","View" ,"TextView" };
	addFunction("newTextView", (void*)PluginProxy::AndroidP::ViewP::TextViewP::newTextView);
	addFunction("updateData", (void*)PluginProxy::AndroidP::ViewP::TextViewP::updateData);


	registerNamespace = { "Android","View" ,"EditText" };
	addFunction("newEditText", (void*)PluginProxy::AndroidP::ViewP::EditTextP::newEditText);
	addFunction("updateData", (void*)PluginProxy::AndroidP::ViewP::EditTextP::updateData);


	registerNamespace = { "Android","View" ,"ImageView" };
	addFunction("newImageView", (void*)PluginProxy::AndroidP::ViewP::ImageViewP::newImageView);

	registerNamespace = { "Android","Window" };
	addFunction("newWindow", (void*)PluginProxy::AndroidP::WindowP::newWindow);
	addFunction("setCanMove", (void*)PluginProxy::AndroidP::WindowP::setCanMove);
	addFunction("addView", (void*)PluginProxy::AndroidP::WindowP::addView);
	addFunction("removeView", (void*)PluginProxy::AndroidP::WindowP::removeView);
	addFunction("deleteWindow", (void*)PluginProxy::AndroidP::WindowP::deleteWindow);
	addFunction("setCanShowKeyboard", (void*)PluginProxy::AndroidP::WindowP::setCanShowKeyboard);
	addFunction("setOrientation", (void*)PluginProxy::AndroidP::WindowP::setOrientation);
	addFunction("setTouchable", (void*)PluginProxy::AndroidP::WindowP::setTouchable);
	addFunction("moveTo", (void*)PluginProxy::AndroidP::WindowP::setCanMove);


	registerNamespace = { "MCPE" };
	addFunction("fetchLocalPlayer", (void*)PluginProxy::MCPE::fetchLocalPlayer);
	addFunction("getPlayerName", (void*)PluginProxy::MCPE::getPlayerName);
	addFunction("print", (void*)PluginProxy::MCPE::print);
	addFunction("getActorList", (void*)PluginProxy::MCPE::getActorList);
	addFunction("isPlayer", (void*)PluginProxy::MCPE::isPlayer);
	addFunction("getHitbox", (void*)PluginProxy::MCPE::getHitbox);
	addFunction("getRot", (void*)PluginProxy::MCPE::getRot);
	addFunction("getPosData", (void*)PluginProxy::MCPE::getPosData);
	addFunction("drawText", (void*)PluginProxy::MCPE::drawText);
	addFunction("drawLine", (void*)HackSDK::drawLine);
	addFunction("drawBox3D", (void*)PluginProxy::MCPE::drawBox3D);
	addFunction("drawLine3D", (void*)PluginProxy::MCPE::drawLine3D);
	addFunction("worldToScreen", (void*)PluginProxy::MCPE::worldToScreen);
	addFunction("getScreenSize", (void*)PluginProxy::MCPE::getScreenSize);
	addFunction("BlockPos::BlockPos", (void*)PluginProxy::MCPE::createBlockPos);
	addFunction("Packet::getName", (void*)PluginProxy::MCPE::getPacketName);
	addFunction("LocalPlayer::attack", (void*)PluginProxy::MCPE::attack);
	addFunction("register(OnTick)", (void*)PluginProxy::MCPE::registerOnTick);
	addFunction("register(OnSendChat)", (void*)PluginProxy::MCPE::registerOnSendChat);
	addFunction("register(OnNewGame)", (void*)PluginProxy::MCPE::registerOnNewGame);
	addFunction("register(OnCmd)", (void*)PluginProxy::MCPE::registerOnCmd);
	addFunction("cmdSuccess", (void*)PluginProxy::MCPE::cmdSuccess);
	addFunction("register(OnAttack)", (void*)PluginProxy::MCPE::registerOnAttack);
	addFunction("register(OnBuild)", (void*)PluginProxy::MCPE::registerOnBuild);
	addFunction("noBuildThisTime", (void*)PluginProxy::MCPE::noBuildThisTime);
	addFunction("register(OnRender)", (void*)PluginProxy::MCPE::registerOnRender);
	addFunction("register(OnSendPacket)", (void*)PluginProxy::MCPE::registerOnSendPacket);
	addFunction("noPacketThisTime", (void*)PluginProxy::MCPE::noPacketThisTime);



}

PluginInterface* mPluginInterface = new PluginInterface();

void PluginHelper::OnTick(Actor* act)
{
	for (auto it = OnTick_callBack.begin(); it != OnTick_callBack.end(); it = it + 1) {
		(*it)(act);
	}
}

void PluginHelper::OnSendPacket(Packet* packet)
{
	for (auto it = OnSendPacket_callBack.begin(); it != OnSendPacket_callBack.end(); it = it + 1) {
		(*it)(packet);
	}
}

void PluginHelper::OnRender(MinecraftUIRenderContext* ctx)
{
	for (auto it = OnRender_callBack.begin(); it != OnRender_callBack.end(); it = it + 1) {
		(*it)(ctx);
	}
}

void PluginHelper::OnCmd(std::vector<std::string>* cmd)
{
	for (auto it = OnCmd_callBack.begin(); it != OnCmd_callBack.end(); it = it + 1) {
		(*it)(cmd);
	}
}

void PluginHelper::OnAttack(GameMode* object, Actor* act)
{
	if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->gamemode == object) {
		for (auto it = OnAttack_callBack.begin(); it != OnAttack_callBack.end(); it = it + 1) {
			(*it)(act);
		}
	}
}

void PluginHelper::OnBuild(GameMode* object, BlockPos const& pos, int rot)
{
	if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->gamemode == object) {
		for (auto it = OnBuild_callBack.begin(); it != OnBuild_callBack.end(); it = it + 1) {
			(*it)(pos,rot);
		}
	}
}

void PluginHelper::OnNewGame(std::string const& ip)
{
	for (auto it = OnNewGame_callBack.begin(); it != OnNewGame_callBack.end(); it = it + 1) {
		(*it)(ip);
	}
}

void PluginHelper::OnSendChat(std::string const& msg)
{
	for (auto it = OnSendChat_callBack.begin(); it != OnSendChat_callBack.end(); it = it + 1) {
		(*it)(msg);
	}
}
