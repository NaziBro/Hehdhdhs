#include "Script.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "Network.h"
#include "HackSDK.h"
#include "Utils.hpp"
#include "ResourceManager.h"
#include <thread>
#include "MCPE/SDK/ModProxy.h"
#include "Zip/unzip.h"
#include "PyManager.h"


class ScriptRuntime {
public:
	static void PCALL(lua_State* L,int n,int r,int f) {
		if (lua_pcall(L,n,r,f) != 0) {
			lua_pop(L, 1);
		}
	}
	std::recursive_mutex thread_mutex;

	int lastProcID = 0;

	class LuaEntry {
	public:
		inline LuaEntry(lua_State* v1, std::string v2, bool show = false) {
			ScriptRuntime* current = moduleManager->getModule<Script>()->runtime;
			L = v1;
			id = v2;
			
			procID = current->lastProcID + 1;
			current->lastProcID = procID;
			canShow = show;
		}
		lua_State* L;
		std::string id;
		int procID;
		bool canShow;
	};

	std::vector<LuaEntry> lua_list;

	class UIData {
	public:
		class View_data {
		public:
			lua_State* L;
			Android::View* v;
		};

		class Window_data {
		public:
			lua_State* L;
			Android::Window* w;
		};


		std::vector<View_data> View_list;
		std::vector<Window_data> Window_list;
		std::recursive_mutex View_mutex;
		std::recursive_mutex Window_mutex;

	};

	UIData uiData;

	void deleteLuaUI(lua_State* L) {
		uiData.View_mutex.lock();
		for (auto it = uiData.View_list.begin(); it != uiData.View_list.end();) {
			if (it->L == L) {
				it->v->release();
				uiData.View_list.erase(it);
			}
			else {
				it = it + 1;

			}
		}
		uiData.View_mutex.unlock();

		uiData.Window_mutex.lock();
		for (auto it = uiData.Window_list.begin(); it != uiData.Window_list.end();) {
			if (it->L == L) {
				it->w->deleteWindow();
				uiData.Window_list.erase(it);
			}
			else {
				it = it + 1;
			}
		}
		uiData.Window_mutex.unlock();
	}

	void exit(int id) {
		std::lock_guard<std::recursive_mutex> lock(thread_mutex);
		for (auto it = lua_list.begin(); it != lua_list.end();) {
			if (it->procID == id) {
				lua_getglobal(it->L, "LuaEvent_OnClose");
				PCALL(it->L, 0, 0, 0);

				lua_close(it->L);
				deleteLuaUI(it->L);
				lua_list.erase(it);
			}
			else {
				it = it + 1;
			}
		}
	}

	class LuaFunction {
	public:
		class LuaUI {
		public:
			

			static int newEditText(lua_State* L) {
				ScriptRuntime* current = moduleManager->getModule<Script>()->runtime;

				Android::EditText* et = mAndroid->newEditText();
				lua_pushlightuserdata(L, et);
				
				current->uiData.View_mutex.lock();
				UIData::View_data d;
				d.L = L;
				d.v = et;
				current->uiData.View_list.push_back(d);
				current->uiData.View_mutex.unlock();

				return 1;
			}

			static int EditText_updateData(lua_State* L) {
				Android::EditText* et = (Android::EditText*)lua_touserdata(L, 1);
				std::string s = lua_tostring(L, 2);
				et->updateData(s);
				return 0;
			}

			static int newTextView(lua_State* L) {
				ScriptRuntime* current = moduleManager->getModule<Script>()->runtime;

				Android::TextView* tv = mAndroid->newTextView();


				lua_pushlightuserdata(L, tv);

				current->uiData.View_mutex.lock();
				UIData::View_data d;
				d.L = L;
				d.v = tv;
				current->uiData.View_list.push_back(d);
				current->uiData.View_mutex.unlock();

				return 1;
			}

			static int TextView_updateData(lua_State* L) {
				Android::TextView* tv = (Android::TextView*)lua_touserdata(L, 1);
				std::string s = lua_tostring(L, 2);
				tv->updateData(s);
				return 0;
			}

			static int newImageView(lua_State* L) {
				ScriptRuntime* current = moduleManager->getModule<Script>()->runtime;

				std::string s = lua_tostring(L, 1);
				Android::ImageView* im = mAndroid->newImageView(s);



				lua_pushlightuserdata(L, im);

				current->uiData.View_mutex.lock();
				UIData::View_data d;
				d.L = L;
				d.v = im;
				current->uiData.View_list.push_back(d);
				current->uiData.View_mutex.unlock();

				return 1;
			}

			static int newButton(lua_State* L) {
				ScriptRuntime* current = moduleManager->getModule<Script>()->runtime;

				Android::Button* btn = mAndroid->newButton();

				lua_pushlightuserdata(L, btn);

				current->uiData.View_mutex.lock();
				UIData::View_data d;
				d.L = L;
				d.v = btn;
				current->uiData.View_list.push_back(d);
				current->uiData.View_mutex.unlock();

				return 1;
			}

			static int Button_updateData(lua_State* L) {
				Android::Button* btn = (Android::Button*)lua_touserdata(L, 1);
				std::string s = lua_tostring(L, 2);
				btn->updateData(s);
				return 0;
			}

			static int EditText_getText(lua_State* L) {
				Android::EditText* et = (Android::EditText*)lua_touserdata(L, 1);
				lua_pushstring(L, et->text.c_str());
				return 1;
			}

			static int newWindow(lua_State* L) {
				ScriptRuntime* current = moduleManager->getModule<Script>()->runtime;

				Android::Window* win = mAndroid->newWindow();
				lua_pushlightuserdata(L, win);

				current->uiData.Window_mutex.lock();
				UIData::Window_data d;
				d.L = L;
				d.w = win;
				current->uiData.Window_list.push_back(d);
				current->uiData.Window_mutex.unlock();

				return 1;
			}

			static int Window_moveTo(lua_State* L) {
				Android::Window* win = (Android::Window*)lua_touserdata(L, 1);
				int x = lua_tointeger(L, 2);
				int y = lua_tointeger(L, 3);
				win->moveTo(x,y);
				return 0;
			}

			static int Window_setCanShowKeyboard(lua_State* L) {
				Android::Window* win = (Android::Window*)lua_touserdata(L, 1);
				bool v = lua_toboolean(L, 2);
				win->setCanShowKeyboard(v);
				return 0;
			}

			static int Window_setCanMove(lua_State* L) {
				Android::Window* win = (Android::Window*)lua_touserdata(L, 1);
				bool v = lua_toboolean(L, 2);
				win->setCanMove(v);
				return 0;
			}


			static int Window_noLimit(lua_State* L) {
				Android::Window* win = (Android::Window*)lua_touserdata(L, 1);
				bool v = lua_toboolean(L, 2);
				win->noLimit(v);
				return 0;
			}


			static int Window_setTouchable(lua_State* L) {
				Android::Window* win = (Android::Window*)lua_touserdata(L, 1);
				bool v = lua_toboolean(L, 2);
				win->setTouchable(v);
				return 0;
			}

			static int Window_setOrientation(lua_State* L) {
				Android::Window* win = (Android::Window*)lua_touserdata(L, 1);
				int v = (int)lua_tonumber(L, 2);
				win->setOrientation(v);
				return 0;
			}

			static int Window_addView(lua_State* L) {
				Android::Window* win = (Android::Window*)lua_touserdata(L, 1);
				Android::View* view = (Android::View*)lua_touserdata(L, 2);
				win->addView(*view);
				return 0;
			}

			static int Window_removeView(lua_State* L) {
				Android::Window* win = (Android::Window*)lua_touserdata(L, 1);
				Android::View* view = (Android::View*)lua_touserdata(L, 2);
				win->removeView(*view);
				return 0;
			}

			static int Window_deleteWindow(lua_State* L) {
				ScriptRuntime* current = moduleManager->getModule<Script>()->runtime;

				Android::Window* win = (Android::Window*)lua_touserdata(L, 1);
				win->deleteWindow();

				current->uiData.Window_mutex.lock();
				for (auto it = current->uiData.Window_list.begin(); it != current->uiData.Window_list.end();) {
					if (it->L == L && it->w == win) {
						current->uiData.Window_list.erase(it);
					}
					else {
						it = it + 1;
					}
				}
				current->uiData.Window_mutex.unlock();

				return 0;
			}

			static int View_setFocusable(lua_State* L) {
				Android::View* view = (Android::View*)lua_touserdata(L, 1);
				bool v = lua_toboolean(L, 2);
				view->setFocusable(v);
				return 0;
			}


			static int Toast(lua_State* L) {
				mAndroid->Toast(lua_tostring(L, 1));
				return 0;
			}

			

			static int View_setOnClickListener(lua_State* L) {
				Android::View* view = (Android::View*)lua_touserdata(L, 1);
				std::string fName = lua_tostring(L, 2);
				view->setOnClickListener([=](Android::View* v) {
					std::lock_guard<std::recursive_mutex> lock(moduleManager->getModule<Script>()->runtime->thread_mutex);
					lua_getglobal(L, fName.c_str());
					lua_pushlightuserdata(L, v);
					PCALL(L, 1, 0, 0);

				});
				
				return 0;
			}

			
			static int View_setOnLongClickListener(lua_State* L) {
				Android::View* view = (Android::View*)lua_touserdata(L, 1);
				std::string fName = lua_tostring(L, 2);
				view->setOnLongClickListener([=](Android::View* v) {
					std::lock_guard<std::recursive_mutex> lock(moduleManager->getModule<Script>()->runtime->thread_mutex);

					lua_getglobal(L, fName.c_str());
					lua_pushlightuserdata(L, v);
					PCALL(L, 1, 0, 0);

				});

				return 0;
			}

			static int View_release(lua_State* L) {
				ScriptRuntime* current = moduleManager->getModule<Script>()->runtime;

				Android::View* view = (Android::View*)lua_touserdata(L, 1);
				view->release();
				
				current->uiData.View_mutex.lock();
				for (auto it = current->uiData.View_list.begin(); it != current->uiData.View_list.end();) {
					if (it->L == L && it->v == view) {
						current->uiData.View_list.erase(it);
					}
					else {
						it = it + 1;
					}
				}
				current->uiData.View_mutex.unlock();
				return 0;
			}

			

			static int EditText_setTextChangedListener(lua_State* L) {
				Android::EditText* et = (Android::EditText*)lua_touserdata(L, 1);
				std::string fName = lua_tostring(L, 2);
				et->setTextChangedListener([=](Android::EditText* v,std::string s) {
					std::lock_guard<std::recursive_mutex> lock(moduleManager->getModule<Script>()->runtime->thread_mutex);

					lua_getglobal(L, fName.c_str());
					lua_pushlightuserdata(L, et);
					lua_pushstring(L, s.c_str());
					PCALL(L, 2, 0, 0);

				});

				return 0;
			}

			static Android::Window* LoadWindowFromJsonStr(lua_State* L, std::string str) {

				ScriptRuntime* current = moduleManager->getModule<Script>()->runtime;

				Android::Window* win = mAndroid->newWindow();

				Json::Reader reader;
				Json::Value value;
				reader.parse(str, value);

				current->uiData.Window_mutex.lock();
				UIData::Window_data d;
				d.L = L;
				d.w = win;
				current->uiData.Window_list.push_back(d);
				current->uiData.Window_mutex.unlock();

				if (value.isMember("CanMove"))win->setCanMove(value["CanMove"].asBool());
				if (value.isMember("NoLimit"))win->noLimit(value["NoLimit"].asBool());
				if (value.isMember("Orientation"))win->setOrientation(value["Orientation"].asInt());
				if (value.isMember("CanShowKeyboard"))win->setCanShowKeyboard(value["CanShowKeyboard"].asBool());
				if (value.isMember("Touchable"))win->setTouchable(value["Touchable"].asBool());
				if (value.isMember("Views")) {
					Json::Value Views = value["Views"];
					for (int i = 0; i < Views.size(); i = i + 1) {
						Json::Value view = Views[i];
						std::string type = view["Type"].asString();
						Android::View* v = nullptr;
						if (type == "TextView") {
							Android::TextView* tv = mAndroid->newTextView();
							v = tv;
							if (view.isMember("Data"))tv->updateData(view["Data"]);
						}
						else if (type == "Button") {
							Android::Button* btn = mAndroid->newButton();
							v = btn;
							if (view.isMember("Data"))btn->updateData(view["Data"]);
						}
						else if (type == "ImageView") {
							std::string Path = "";
							if (view.isMember("Path"))Path = view["Path"].asString();
							Android::ImageView* im = mAndroid->newImageView(Path);
							v = im;
						}
						else if (type == "EditText") {
							Android::EditText* et = mAndroid->newEditText();

							v = et;
							if (view.isMember("Data"))et->updateData(view["Data"]);
							if (view.isMember("TextChangedListener")) {

								et->setTextChangedListener([=](Android::EditText* et, std::string s) {
									std::lock_guard<std::recursive_mutex> lock(moduleManager->getModule<Script>()->runtime->thread_mutex);

									lua_getglobal(L, view["TextChangedListener"].asString().c_str());
									lua_pushlightuserdata(L, et);
									lua_pushstring(L, s.c_str());
									PCALL(L, 2, 0, 0);

								});
							}
						}
						if (v != nullptr) {
							win->addView(*v);
							current->uiData.View_mutex.lock();
							UIData::View_data d;
							d.L = L;
							d.v = v;
							current->uiData.View_list.push_back(d);
							current->uiData.View_mutex.unlock();

							if (view.isMember("Focusable"))v->setFocusable(view["Focusable"].asBool());
							if (view.isMember("BindGlobalValue")) {
								lua_pushlightuserdata(L, v);
								lua_setglobal(L, view["BindGlobalValue"].asString().c_str());
							}
							if (view.isMember("OnClickListener")) {
								v->setOnClickListener([=](Android::View* v) {
									std::lock_guard<std::recursive_mutex> lock(moduleManager->getModule<Script>()->runtime->thread_mutex);
									lua_getglobal(L, view["OnClickListener"].asString().c_str());
									lua_pushlightuserdata(L, v);
									PCALL(L, 1, 0, 0);

								});
							}
							if (view.isMember("OnLongClickListener")) {
								v->setOnLongClickListener([=](Android::View* v) {
									std::lock_guard<std::recursive_mutex> lock(moduleManager->getModule<Script>()->runtime->thread_mutex);

									lua_getglobal(L, view["OnLongClickListener"].asString().c_str());
									lua_pushlightuserdata(L, v);
									PCALL(L, 1, 0, 0);

									});

							}
						}
					}
				}
				return win;
			}

			static int LoadWindowFromJsonBase64(lua_State* L) {
				const char* base64 = lua_tostring(L, 1);
				std::string data = Utils::base64_decode(base64, false);
				Android::Window* win = LoadWindowFromJsonStr(L, data);
				lua_pushlightuserdata(L, win);
				return 1;
			}

			static int LoadWindowFromJsonFile(lua_State* L) {
				Android::Window* win = LoadWindowFromJsonStr(L, lua_tostring(L, 1));
				lua_pushlightuserdata(L, win);
				return 1;
			}
		};

		class MC {
		public:
			static int SetItemData(lua_State* L) {
				int id = lua_tointeger(L, 1);
				int aux = lua_tointeger(L, 2);
				int count = lua_tointeger(L, 3);
				CompoundTag* nbt_old = (CompoundTag*)lua_touserdata(L, 4);
				bool v = lua_toboolean(L, 5);

				ItemStack* item = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItem();
				uint32_t slot = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItemSlot();

				CompoundTag* nbt = new CompoundTag;
				nbt->deepCopy(*nbt_old);

				if (item->getId() == 0) {
					ItemInstance new_item(id, count, aux);
					if (new_item.userData) {
						delete new_item.userData;
					}
					new_item.userData = nbt;

					ItemStack k(new_item);
					if (v) {
						moduleManager->getModule<HackSDK>()->transferItem(*item, new_item, slot);
					}
				}
				else {
					ItemStack old_data(*item);
					item->_setItem(id);
					item->setAux(aux);
					item->set(count);
					if (item->userData) {
						delete item->userData;
					}
					item->userData = nbt;

					if (v) {
						moduleManager->getModule<HackSDK>()->transferItem(old_data, *item, slot);
					}
				}

				return 0;
			}

			static int Enchant(lua_State* L) {
				int id = (int)lua_tonumber(L, 1);
				int lvl = (int)lua_tonumber(L, 2);
				bool a = lua_toboolean(L, 3);
				ItemStack* item = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItem();
				if (item->getId() == 0)return 0;
				ItemStack old_data(*item);

				HackSDK::Enchant(item, id, lvl);

				if (a == true) {
					uint32_t slot = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItemSlot();
					moduleManager->getModule<HackSDK>()->transferItem(old_data, *item, slot);
				}
				return 0;
			}

			static int ActorClassTree_isInstanceOf(lua_State* L) {
				Actor* a = (Actor*)lua_touserdata(L, 1);
				int t = (int)lua_tonumber(L, 2);
				bool v = ActorClassTree::isInstanceOf(*a, t);
				lua_pushboolean(L, v);
				return 1;
			}

			static int SetHitBox(lua_State* L) {
				Actor* a = (Actor*)lua_touserdata(L, 1);
				float x = (float)lua_tonumber(L, 2);
				float y = (float)lua_tonumber(L, 3);
				a->weidth = x;
				a->height = y;
				return 0;
			}

			static int SetBlock(lua_State* L) {
				int x = (int)lua_tonumber(L, 1);
				int y = (int)lua_tonumber(L, 2);
				int z = (int)lua_tonumber(L, 3);
				int blockID = (int)lua_tonumber(L, 4);
				Item* item = Item::getItem(20);
				BlockLegacy* block_l = **(item->getLegacyBlock());
				Block* block = block_l->getStateFromLegacyData(0);
				BlockSource* bs = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getRegion();
				bs->setBlock(x, y, z, *block, 3);
				return 0;
			}

			static int AddMobEffect(lua_State* L) {
				Actor* a = (Actor*)lua_touserdata(L, 1);
				int Id = (int)lua_tonumber(L, 2);
				int Amplifier = (int)lua_tonumber(L, 3);
				int Time = (int)lua_tonumber(L, 4);
				bool ShowParticles = lua_toboolean(L, 5);
				HackSDK::addServerPacket(MobEffect(a->getRuntimeID(), 1, Id, Amplifier, ShowParticles, Time));
				return 0;
			}

			static int GetMobHealth(lua_State* L) {
				Actor* a = (Actor*)lua_touserdata(L, 1);
				lua_pushinteger(L, a->getHealth());
				return 1;
			}

			static int GetBlockIDFromPos(lua_State* L) {
				BlockSource* bs = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getRegion();
				int x = (int)lua_tonumber(L, 1);
				int y = (int)lua_tonumber(L, 2);
				int z = (int)lua_tonumber(L, 3);

				Block* block = bs->getBlock(x, y, z);
				int id = block->_BlockLegacy->getBlockItemId();
				lua_pushnumber(L, id);
				return 1;
			}

			static int GetBlockVariantFromPos(lua_State* L) {
				BlockSource* bs = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getRegion();
				int x = (int)lua_tonumber(L, 1);
				int y = (int)lua_tonumber(L, 2);
				int z = (int)lua_tonumber(L, 3);

				Block* block = bs->getBlock(x, y, z);
				int var = block->getVariant();
				lua_pushnumber(L, var);
				return 1;
			}

			static int GetBlockNameFromPos(lua_State* L) {
				BlockSource* bs = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getRegion();
				int x = (int)lua_tonumber(L, 1);
				int y = (int)lua_tonumber(L, 2);
				int z = (int)lua_tonumber(L, 3);

				Block* block = bs->getBlock(x, y, z);
				lua_pushstring(L, block->_BlockLegacy->name.c_str());
				return 1;
			}

			static int SwingArm(lua_State* L) {
				mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->swing();
				return 0;
			}

			static int Actor_distanceTo(lua_State* L) {
				Actor* a1 = (Actor*)lua_touserdata(L, 1);
				Actor* a2 = (Actor*)lua_touserdata(L, 2);
				lua_pushnumber(L, a1->distanceTo(*a2));
				return 1;
			}

			static int GetMobAABB(lua_State* L) {
				Actor* a1 = (Actor*)lua_touserdata(L, 1);
				lua_pushnumber(L, a1->PosData.from.x);
				lua_pushnumber(L, a1->PosData.from.y);
				lua_pushnumber(L, a1->PosData.from.z);
				lua_pushnumber(L, a1->PosData.to.x);
				lua_pushnumber(L, a1->PosData.to.y);
				lua_pushnumber(L, a1->PosData.to.z);
				return 6;
			}

			static int SetMobAABB(lua_State* L) {
				Actor* a1 = (Actor*)lua_touserdata(L, 1);
				a1->PosData.from.x = lua_tonumber(L, 2);
				a1->PosData.from.y = lua_tonumber(L, 3);
				a1->PosData.from.z = lua_tonumber(L, 4);
				a1->PosData.to.x = lua_tonumber(L, 5);
				a1->PosData.to.y = lua_tonumber(L, 6);
				a1->PosData.to.z = lua_tonumber(L, 7);
				return 0;
			}

			static int GetMobRot(lua_State* L) {
				Actor* a1 = (Actor*)lua_touserdata(L, 1);
				lua_pushnumber(L, a1->rot.yaw);
				lua_pushnumber(L, a1->rot.pitch);
				return 2;
			}

			static int DestroyBlock(lua_State* L) {
				int x = (int)lua_tonumber(L, 1);
				int y = (int)lua_tonumber(L, 2);
				int z = (int)lua_tonumber(L, 3);
				int rot = (int)lua_tonumber(L, 4);
				BlockPos p;
				p.x = x;
				p.y = y;
				p.z = z;
				mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->gamemode->destroyBlock(p, rot);
				return 0;
			}

			static int SetMobMotion(lua_State* L) {
				Actor* a1 = (Actor*)lua_touserdata(L, 1);
				float x = lua_tonumber(L, 2);
				float y = lua_tonumber(L, 3);
				float z = lua_tonumber(L, 4);
				a1->motion.x = x;
				a1->motion.y = y;
				a1->motion.z = z;

				return 0;
			}

			static int ForEachMob(lua_State* L) {
				std::vector<Actor*> all;
				if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer() == nullptr)return 0;
				Dimension* dim = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getDimension();
				dim->forEachPlayer([&](Player& a) {
					all.push_back(&a);
					return true;
					});
				if (dim->getEntityIdMap() == nullptr)return 0;
				dim->getEntityIdMap()->forEachEntity([&](Actor& a) {
					all.push_back(&a);
					});

				for (auto it = all.begin(); it != all.end(); it = it + 1) {
					if (*it != nullptr) {
						lua_getglobal(L, lua_tostring(L, 1));
						lua_pushlightuserdata(L, *it);
						PCALL(L, 1, 0, 0);
					}
				}
				return 0;
			}


			static int AttackMob(lua_State* L) {
				mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->gamemode->attack(*(Actor*)lua_touserdata(L, 1));
				return 0;
			}

			static int PrintText(lua_State* L) {
				mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(lua_tostring(L, 1), "");
				return 0;
			}

			static int ClickScreen(lua_State* L) {
				mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->handleBuildOrAttackButtonPress();
				return 0;
			}

			static int GetLocalPlayer(lua_State* L) {
				lua_pushlightuserdata(L, mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer());
				return 1;
			}

			static int IsInGame(lua_State* L) {
				lua_pushboolean(L, mGameData.getNowMinecraftGame()->isInGame());
				return 1;
			}

			static int SendChatMessage(lua_State* L) {
				std::string chat = lua_tostring(L, 1);
				LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();

				HackSDK::sendOriginalPacket(Text::createChat(lp->getName(), chat, "1234567", "666"));
				return 0;
			}

			static int CommandParams_size(lua_State* L) {
				auto cmd = (std::vector<std::string>*)lua_touserdata(L, 1);
				lua_pushinteger(L, cmd->size());
				return 1;
			}

			static int CommandParams_getString(lua_State* L) {
				auto cmd = (std::vector<std::string>*)lua_touserdata(L, 1);
				int index = lua_tointeger(L, 2);
				lua_pushstring(L, (*cmd)[index].c_str());
				return 1;
			}

			static int Command_executed(lua_State* L) {
				moduleManager->executedCMD = true;
				return 0;
			}

			static int CompoundTag_new(lua_State* L) {
				CompoundTag* tag = new CompoundTag;
				lua_pushlightuserdata(L, tag);
				return 1;
			}

			static int CompoundTag_delete(lua_State* L) {
				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				delete tag;
				return 0;
			}

			static int CompoundTag_putShort(lua_State* L) {
				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				std::string name = lua_tostring(L, 2);
				short v = (short)lua_tointeger(L, 3);
				tag->putShort(name, v);
				return 0;
			}

			static int CompoundTag_putInt64(lua_State* L) {
				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				std::string name = lua_tostring(L, 2);
				int64_t v = lua_tointeger(L, 3);
				tag->putInt64(name, v);
				return 0;
			}

			static int CompoundTag_putFloat(lua_State* L) {
				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				std::string name = lua_tostring(L, 2);
				float v = (float)lua_tonumber(L, 3);
				tag->putFloat(name, v);
				return 0;
			}

			static int CompoundTag_putDouble(lua_State* L) {
				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				std::string name = lua_tostring(L, 2);
				double v = (lua_tonumber(L, 3));
				tag->putDouble(name, v);
				return 0;
			}

			static int CompoundTag_putString(lua_State* L) {
				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				std::string name = lua_tostring(L, 2);
				std::string v = lua_tostring(L, 3);
				tag->putString(name, v);
				return 0;
			}

			static int GetItemInfo(lua_State* L) {
				ItemStack* item = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItem();
				if (item) {
					lua_pushinteger(L, item->getId());
					lua_pushinteger(L, item->getDataValue());
					lua_pushinteger(L, item->count());
					lua_pushlightuserdata(L, item->userData);

				}
				else {
					lua_pushinteger(L, 0);
					lua_pushinteger(L, 0);
					lua_pushinteger(L, 0);
					lua_pushlightuserdata(L, 0);
				}
				return 4;
			}

			static int RequestCommand(lua_State* L) {
				HackSDK::sendOriginalPacket(CommandRequest({ 0,0 }, lua_tostring(L, 1),CommandOrigin::CommandOriginPlayer, 0, false, false));
				return 0;
			}

			static int CompoundTag_putBoolean(lua_State* L) {
				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				std::string name = lua_tostring(L, 2);
				bool v = lua_toboolean(L, 3);
				tag->putBoolean(name, v);
				return 0;
			}

			static int CompoundTag_putInt(lua_State* L) {
				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				std::string name = lua_tostring(L, 2);
				int v = lua_tointeger(L, 3);
				tag->putInt(name, v);
				return 0;
			}

			static int CompoundTag_putByte(lua_State* L) {
				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				std::string name = lua_tostring(L, 2);
				char v = (char)lua_tointeger(L, 3);
				tag->putByte(name, v);
				return 0;
			}

			static int CompoundTag_putCommpound(lua_State* L) {
				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				std::string name = lua_tostring(L, 2);
				CompoundTag* v = (CompoundTag*)lua_touserdata(L, 3);
				std::unique_ptr<CompoundTag> k(new CompoundTag);
				k->deepCopy(*v);
				tag->put(name, std::move(k));

				return 0;
			}

			static int CompoundTag_putList(lua_State* L) {
				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				std::string name = lua_tostring(L, 2);
				ListTag* v = (ListTag*)lua_touserdata(L, 3);
				tag->put(name, std::move(v->copy()));

				return 0;
			}

			static int ListTag_new(lua_State* L) {
				ListTag* tag = new ListTag;
				lua_pushlightuserdata(L, tag);
				return 1;
			}

			static int ListTag_delete(lua_State* L) {
				ListTag* tag = (ListTag*)lua_touserdata(L, 1);
				delete tag;
				return 0;
			}

			static int Teleport(lua_State* L) {
				float x = lua_tonumber(L, 1);
				float y = lua_tonumber(L, 2);
				float z = lua_tonumber(L, 3);
				LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
				Vec3 pos(x, y, z);
				lp->teleportTo(pos, true, 0, 1);
				return 0;
			}

			static int ListTag_addCompound(lua_State* L) {
				ListTag* tag = (ListTag*)lua_touserdata(L, 1);
				CompoundTag* v = (CompoundTag*)lua_touserdata(L, 2);
				std::unique_ptr<CompoundTag> k(new CompoundTag);
				k->deepCopy(*v);
				tag->add(std::move(k));

				return 0;
			}

			static int ListTag_addString(lua_State* L) {
				ListTag* tag = (ListTag*)lua_touserdata(L, 1);
				std::unique_ptr<StringTag> k(new StringTag(lua_tostring(L, 2)));
				tag->add(std::move(k));

				return 0;
			}

			static int ListTag_addFloat(lua_State* L) {
				ListTag* tag = (ListTag*)lua_touserdata(L, 1);
				std::unique_ptr<FloatTag> k(new FloatTag((float)lua_tonumber(L, 2)));
				tag->add(std::move(k));

				return 0;
			}

			static int ListTag_addDouble(lua_State* L) {
				ListTag* tag = (ListTag*)lua_touserdata(L, 1);
				std::unique_ptr<DoubleTag> k(new DoubleTag((float)lua_tonumber(L, 2)));
				tag->add(std::move(k));

				return 0;
			}

			static int ListTag_addInt64(lua_State* L) {
				ListTag* tag = (ListTag*)lua_touserdata(L, 1);
				std::unique_ptr<Int64Tag> k(new Int64Tag(lua_tointeger(L, 2)));
				tag->add(std::move(k));

				return 0;
			}

			static int ListTag_addInt(lua_State* L) {
				ListTag* tag = (ListTag*)lua_touserdata(L, 1);
				std::unique_ptr<IntTag> k(new IntTag(lua_tointeger(L, 2)));
				tag->add(std::move(k));

				return 0;
			}

			static int SetPlayerAbility(lua_State* L) {
				Player* p = (Player*)lua_touserdata(L, 1);
				int index = lua_tointeger(L, 2);
				std::string type = lua_tostring(L, 3);
				if (type == "float") {
					float v = lua_tonumber(L, 4);
					p->getAbilities()->setAbility(index, v);
				}
				else if (type == "bool") {

					bool v = lua_toboolean(L, 4);
					p->getAbilities()->setAbility(index, v);
				}
				return 0;
			}

			static int Actor_setOnGround(lua_State* L) {
				Actor* a = (Actor*)lua_touserdata(L, 1);
				a->onGround = lua_toboolean(L, 2);
				return 0;
			}

			static int Jump(lua_State* L) {
				mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->jumpFromGround();
				return 0;
			}

			static int DrawLine(lua_State* L) {
				MinecraftUIRenderContext* ctx = (MinecraftUIRenderContext*)lua_touserdata(L, 1);
				HackSDK::drawLine(*ctx, vec2_t((float)lua_tonumber(L, 2), (float)lua_tonumber(L, 3)), vec2_t((float)lua_tonumber(L, 4), (float)lua_tonumber(L, 5)), (float)lua_tonumber(L, 6), mce::Color((float)lua_tonumber(L, 7), (float)lua_tonumber(L, 8), (float)lua_tonumber(L, 9)));
				return 0;
			}

			static int DrawLine3D(lua_State* L) {
				LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
				glmatrixf* matrix = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix();
				vec3_t cam((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
				vec2_t fov = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getFov();
				float screenWidth = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenWdith;
				float screenHeight = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenHeight;
				vec2_t display(screenWidth, screenHeight);
				MinecraftUIRenderContext* ctx = (MinecraftUIRenderContext*)lua_touserdata(L, 1);
				vec3_t start((float)lua_tonumber(L, 2), (float)lua_tonumber(L, 3), (float)lua_tonumber(L, 4));
				vec3_t end((float)lua_tonumber(L, 5), (float)lua_tonumber(L, 6), (float)lua_tonumber(L, 7));
				mce::Color color((float)lua_tonumber(L, 9), (float)lua_tonumber(L, 10), (float)lua_tonumber(L, 11));
				HackSDK::drawLine3D(*ctx, matrix, cam, fov, display, start, end, (float)lua_tonumber(L, 8), color);
				return 0;
			}

			static int DrawBox3D(lua_State* L) {
				LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
				glmatrixf* matrix = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix();
				vec3_t cam((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
				vec2_t fov = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getFov();
				float screenWidth = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenWdith;
				float screenHeight = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenHeight;
				vec2_t display(screenWidth, screenHeight);
				MinecraftUIRenderContext* ctx = (MinecraftUIRenderContext*)lua_touserdata(L, 1);
				vec3_t start((float)lua_tonumber(L, 2), (float)lua_tonumber(L, 3), (float)lua_tonumber(L, 4));
				vec3_t end((float)lua_tonumber(L, 5), (float)lua_tonumber(L, 6), (float)lua_tonumber(L, 7));
				mce::Color color((float)lua_tonumber(L, 9), (float)lua_tonumber(L, 10), (float)lua_tonumber(L, 11));
				HackSDK::drawBox3D(*ctx, matrix, cam, fov, display, start, end, (float)lua_tonumber(L, 8), color);
				return 0;
			}

			static int DrawText(lua_State* L) {
				MinecraftUIRenderContext* ctx = (MinecraftUIRenderContext*)lua_touserdata(L, 1);
				vec2_t pos((float)lua_tonumber(L, 2), (float)lua_tonumber(L, 3));
				HackSDK::drawText(*ctx, pos, lua_tostring(L, 4), mce::Color(0.3f, 0.3f, 0.3f), mGameData.getNowMinecraftGame()->getUnicodeFontHandle()->getFont(), (float)lua_tonumber(L, 5));
				return 0;
			}

			static int GetScreenData(lua_State* L) {
				float screenWidth = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenWdith;
				float screenHeight = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenHeight;
				lua_pushnumber(L, screenWidth);
				lua_pushnumber(L, screenHeight);
				return 2;
			}

			static int WorldToScreen(lua_State* L) {
				float screenWidth = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenWdith;
				float screenHeight = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenHeight;
				vec2_t display(screenWidth, screenHeight);
				vec2_t fov = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getFov();
				vec2_t screenPos;
				LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
				vec3_t origin((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
				mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix()->OWorldToScreen(origin, vec3_t((float)lua_tonumber(L, 1), (float)lua_tonumber(L, 2), (float)lua_tonumber(L, 3)), screenPos, fov, display);
				lua_pushnumber(L, screenPos.x);
				lua_pushnumber(L, screenPos.y);
				return 2;
			}

			static int GetViewportInfo(lua_State* L) {
				lua_pushnumber(L, mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getViewportInfo()->yaw);
				lua_pushnumber(L, mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getViewportInfo()->pitch);
				return 2;
			}

			static int SetViewportInfo(lua_State* L) {
				mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getViewportInfo()->yaw = lua_tonumber(L, 1);
				mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getViewportInfo()->pitch = lua_tonumber(L, 2);
				return 0;
			}

			static int LookAt(lua_State* L) {
				Vec3 pos(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
				HackSDK::lookAt(pos);
				return 0;
			}

			static int LoadItemRootNBT(lua_State* L) {

				CompoundTag* tag = (CompoundTag*)lua_touserdata(L, 1);
				bool a = lua_toboolean(L, 2);
				ItemStack* item = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItem();
				if (item->getId() == 0)return 0;
				ItemStack old_data(*item);

				item->load(*tag);

				if (a == true) {
					uint32_t slot = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItemSlot();
					moduleManager->getModule<HackSDK>()->transferItem(old_data, *item, slot);
				}
				return 0;
			}

			static int TryBuildBlock(lua_State* L) {
				BlockPos bp(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tointeger(L, 3));
				HackSDK::TryBuildBlock(bp);
				return 0;
			}

			static int ModSpeed(lua_State* L) {
				if (lua_toboolean(L, 1)) {
					moduleManager->getModule<Script>()->runtime->modSpeed = true;
					moduleManager->getModule<Script>()->runtime->speed = lua_tonumber(L, 2);
				}
				else {
					moduleManager->getModule<Script>()->runtime->modSpeed = false;
				}
				return 0;
			}

			static int ModFancySkies(lua_State* L) {
				bool v1 = lua_toboolean(L, 1);
				bool v2 = lua_toboolean(L, 2);
				moduleManager->getModule<Script>()->runtime->modFancySkies = v1;
				moduleManager->getModule<Script>()->runtime->fancySkies = v2;
				return 0;
			}

			static int AddResourcePack(lua_State* L) {
				std::string Path = lua_tostring(L, 1);
				if (lua_toboolean(L, 2)) {
					std::thread thread([=]() {
						if(moduleManager->getModule<ResourceManager>())moduleManager->getModule<ResourceManager>()->addPack(Path);
					});
					thread.detach();
				}
				else {
					if (moduleManager->getModule<ResourceManager>())moduleManager->getModule<ResourceManager>()->addPack(Path);
				}
				return 0;
			}
			static int RemoveResourcePack(lua_State* L) {
				std::string Path = lua_tostring(L, 1);
				if (moduleManager->getModule<ResourceManager>())moduleManager->getModule<ResourceManager>()->removePack(Path);
				return 0;
			}

			static int SetTime(lua_State* L) {
				mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getLevel()->setTime(lua_tointeger(L, 1));
				return 0;
			}

			static int ModFogColor(lua_State* L) {
				bool b = lua_toboolean(L, 1);
				if (b) {
					mce::Color color(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
					ModProxy::clientSingleton()->modFogColor(true, &color);
				}
				else {
					ModProxy::clientSingleton()->modFogColor(false);
				}
				return 0;
			}

		};

		class Lua {
		public:
			class WaitCallThreadStack {
			public:
				lua_State* L;
				std::string f;
				std::string type;
				int sleeptime;
			};

			static void* CallThread(void* o) {
				WaitCallThreadStack* stack = (WaitCallThreadStack*)o;
				if (stack->type == "sleep") {
					sleep(stack->sleeptime);
				}
				else if (stack->type == "usleep") {
					usleep(stack->sleeptime);
				}
				std::lock_guard<std::recursive_mutex> lock(moduleManager->getModule<Script>()->runtime->thread_mutex);
				std::vector<lua_State*> list_b;
				ScriptRuntime* runtime = moduleManager->getModule<Script>()->runtime;
				for (auto it = runtime->lua_list.begin(); it != runtime->lua_list.end(); it = it + 1) {
					if (it->L == stack->L) {
						lua_getglobal(stack->L, stack->f.c_str());
						PCALL(stack->L, 0, 0, 0);
						break;
					}
				}
				delete stack;
				return nullptr;
			}

			static int ThreadCall(lua_State* L) {
				pthread_t t1;
				WaitCallThreadStack* stack = new WaitCallThreadStack;
				stack->L = L;
				stack->f = lua_tostring(L, 1);
				stack->type = lua_tostring(L, 2);
				stack->sleeptime = lua_tointeger(L, 3);
				pthread_create(&t1, 0, CallThread, stack);
				return 1;
			}

			static int LoadPlugin(lua_State* L) {
				moduleManager->getModule<Script>()->load(lua_tostring(L, 1));
				return 0;
			}

			static int GetNetworkDataPath(lua_State* L) {
				if (moduleManager->getModule<Network>())lua_pushstring(L, moduleManager->getModule<Network>()->unPackPath.c_str());
				else lua_pushstring(L,"");
				return 1;
			}

			static int SendMessageToServer(lua_State* L) {
				std::string msg = lua_tostring(L, 1);
				if(moduleManager->getModule<Network>())moduleManager->getModule<Network>()->sendMessageToServer(msg);
				return 0;
			}

			


			static int Exit(lua_State* L) {

				ScriptRuntime* current = moduleManager->getModule<Script>()->runtime;

				int procID = 0;
				std::lock_guard<std::recursive_mutex> lock(current->thread_mutex);

				for (auto it = current->lua_list.begin(); it != current->lua_list.end(); it = it + 1) {
					if (it->L == L) {
						procID = it->procID;
						break;
					}
				}
				current->exit(procID);
				return 0;
			}
		};

		class System {
		public:
			static int setMediaVolume(lua_State* L) {
				mAndroid->setMediaVolume(lua_tointeger(L, 1));
				return 0;
			}

			static int playMusic(lua_State* L) {
				mAndroid->playMusic(lua_tostring(L, 1));
				return 0;
			}

			static int abort(lua_State* L) {
				::abort();
				return 0;
			}

			static int removePath(lua_State* L) {
				Utils::DeleteFile(lua_tostring(L, 1));
				return 0;
			}

			static int getUserID(lua_State* L) {
				lua_pushstring(L, Android::getHWId().c_str());
				return 1;
			}

			static int unZip(lua_State* L) {

				HZIP hz = OpenZip(lua_tostring(L,1), NULL);
				SetUnzipBaseDir(hz, lua_tostring(L,2));
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
				return 0;
			}
		};

		class CPP {
		public:
			static int string_get(lua_State* L) {
				std::string* str = (std::string*)lua_touserdata(L, 1);
				lua_pushstring(L,str->c_str());
				return 1;
			}

			static int string_set(lua_State* L) {
				std::string* str = (std::string*)lua_touserdata(L, 1);
				(*str) = lua_tostring(L, 2);
				return 1;
			}
		};
	};

	static void registerFunctions(lua_State* L) {

		lua_register(L, "CPP_string_get", LuaFunction::CPP::string_get);
		lua_register(L, "CPP_string_set", LuaFunction::CPP::string_set);
		lua_register(L, "System_playMusic", LuaFunction::System::playMusic);
		lua_register(L, "System_unZip", LuaFunction::System::unZip);
		lua_register(L, "System_setMediaVolume", LuaFunction::System::setMediaVolume);
		lua_register(L, "System_getUserID", LuaFunction::System::getUserID);
		lua_register(L, "System_playMusic", LuaFunction::System::playMusic);
		lua_register(L, "System_abort", LuaFunction::System::abort);
		lua_register(L, "System_removePath", LuaFunction::System::removePath);
		lua_register(L, "Lua_Exit", LuaFunction::Lua::Exit);
		lua_register(L, "Lua_LoadPlugin", LuaFunction::Lua::LoadPlugin);
		lua_register(L, "Lua_GetNetworkDataPath", LuaFunction::Lua::GetNetworkDataPath);
		lua_register(L, "Lua_ThreadCall", LuaFunction::Lua::ThreadCall);
		lua_register(L, "Lua_SendMessageToServer", LuaFunction::Lua::SendMessageToServer);
		lua_register(L, "MC_Actor_setOnGround", LuaFunction::MC::Actor_setOnGround);
		lua_register(L, "MC_SetPlayerAbility", LuaFunction::MC::SetPlayerAbility);
		lua_register(L, "MC_RequestCommand", LuaFunction::MC::RequestCommand);
		lua_register(L, "MC_GetItemInfo", LuaFunction::MC::GetItemInfo);
		lua_register(L, "MC_TryBuildBlock", LuaFunction::MC::TryBuildBlock);
		lua_register(L, "MC_ListTag_addCompound", LuaFunction::MC::ListTag_addCompound);
		lua_register(L, "MC_ListTag_addString", LuaFunction::MC::ListTag_addString);
		lua_register(L, "MC_ListTag_addFloat", LuaFunction::MC::ListTag_addFloat);
		lua_register(L, "MC_ListTag_addDouble", LuaFunction::MC::ListTag_addDouble);
		lua_register(L, "MC_ListTag_addInt64", LuaFunction::MC::ListTag_addInt64);
		lua_register(L, "MC_ListTag_addInt", LuaFunction::MC::ListTag_addInt);
		lua_register(L, "MC_CompoundTag_putList", LuaFunction::MC::CompoundTag_putList);
		lua_register(L, "MC_CompoundTag_putInt", LuaFunction::MC::CompoundTag_putInt);
		lua_register(L, "MC_CompoundTag_putByte", LuaFunction::MC::CompoundTag_putByte);
		lua_register(L, "MC_CompoundTag_putBoolean", LuaFunction::MC::CompoundTag_putBoolean);
		lua_register(L, "MC_CompoundTag_putFloat", LuaFunction::MC::CompoundTag_putFloat);
		lua_register(L, "MC_CompoundTag_putDouble", LuaFunction::MC::CompoundTag_putDouble);
		lua_register(L, "MC_CompoundTag_putString", LuaFunction::MC::CompoundTag_putString);
		lua_register(L, "MC_CompoundTag_putShort", LuaFunction::MC::CompoundTag_putShort);
		lua_register(L, "MC_CompoundTag_putInt64", LuaFunction::MC::CompoundTag_putInt64);
		lua_register(L, "MC_CompoundTag_putCompound", LuaFunction::MC::CompoundTag_putCommpound);
		lua_register(L, "MC_CompoundTag_delete", LuaFunction::MC::CompoundTag_delete);
		lua_register(L, "MC_CompoundTag_new", LuaFunction::MC::CompoundTag_new);
		lua_register(L, "MC_ListTag_delete", LuaFunction::MC::ListTag_delete);
		lua_register(L, "MC_ListTag_new", LuaFunction::MC::ListTag_new);
		lua_register(L, "MC_Command_executed", LuaFunction::MC::Command_executed);
		lua_register(L, "MC_CommandParams_size", LuaFunction::MC::CommandParams_size);
		lua_register(L, "MC_CommandParams_getString", LuaFunction::MC::CommandParams_getString);
		lua_register(L, "MC_SendChatMessage", LuaFunction::MC::SendChatMessage);
		lua_register(L, "MC_ActorClassTree_isInstanceOf", LuaFunction::MC::ActorClassTree_isInstanceOf);
		lua_register(L, "MC_SetHitBox", LuaFunction::MC::SetHitBox);
		lua_register(L, "MC_SetItemData", LuaFunction::MC::SetItemData);
		lua_register(L, "MC_Enchant", LuaFunction::MC::Enchant);
		lua_register(L, "MC_Teleport", LuaFunction::MC::Teleport);
		lua_register(L, "MC_SetTime", LuaFunction::MC::SetTime);
		lua_register(L, "MC_SetBlock", LuaFunction::MC::SetBlock);
		lua_register(L, "MC_GetBlockIDFromPos", LuaFunction::MC::GetBlockIDFromPos);
		lua_register(L, "MC_GetBlockVariantFromPos", LuaFunction::MC::GetBlockVariantFromPos);
		lua_register(L, "MC_GetBlockNameFromPos", LuaFunction::MC::GetBlockNameFromPos);
		lua_register(L, "MC_DestroyBlock", LuaFunction::MC::DestroyBlock);
		lua_register(L, "MC_AddMobEffect", LuaFunction::MC::AddMobEffect);
		lua_register(L, "MC_SetMobMotion", LuaFunction::MC::SetMobMotion);
		lua_register(L, "MC_LookAt", LuaFunction::MC::LookAt);
		lua_register(L, "MC_GetMobHealth", LuaFunction::MC::GetMobHealth);
		lua_register(L, "MC_GetMobRot", LuaFunction::MC::GetMobRot);
		lua_register(L, "MC_GetMobAABB", LuaFunction::MC::GetMobAABB);
		lua_register(L, "MC_SetMobAABB", LuaFunction::MC::SetMobAABB);
		lua_register(L, "MC_SwingArm", LuaFunction::MC::SwingArm);
		lua_register(L, "MC_Actor_distanceTo", LuaFunction::MC::Actor_distanceTo);
		lua_register(L, "MC_ForEachMob", LuaFunction::MC::ForEachMob);
		lua_register(L, "MC_AddResourcePack", LuaFunction::MC::AddResourcePack);
		lua_register(L, "MC_RemoveResourcePack", LuaFunction::MC::RemoveResourcePack);
		lua_register(L, "MC_PrintText", LuaFunction::MC::PrintText);
		lua_register(L, "MC_AttackMob", LuaFunction::MC::AttackMob);
		lua_register(L, "MC_ClickScreen", LuaFunction::MC::ClickScreen);
		lua_register(L, "MC_GetLocalPlayer", LuaFunction::MC::GetLocalPlayer);
		lua_register(L, "MC_LoadItemRootNBT", LuaFunction::MC::LoadItemRootNBT);
		lua_register(L, "MC_GetViewportInfo", LuaFunction::MC::GetViewportInfo);
		lua_register(L, "MC_SetViewportInfo", LuaFunction::MC::SetViewportInfo);
		lua_register(L, "MC_ModSpeed", LuaFunction::MC::ModSpeed);
		lua_register(L, "MC_ModFogColor", LuaFunction::MC::ModFogColor);
		lua_register(L, "MC_IsInGame", LuaFunction::MC::IsInGame);
		lua_register(L, "MC_Jump", LuaFunction::MC::Jump);
		lua_register(L, "MC_DrawBox3D", LuaFunction::MC::DrawBox3D);
		lua_register(L, "MC_DrawLine3D", LuaFunction::MC::DrawLine3D);
		lua_register(L, "MC_DrawText", LuaFunction::MC::DrawText);
		lua_register(L, "MC_ModFancySkies", LuaFunction::MC::ModFancySkies);
		lua_register(L, "MC_GetScreenData", LuaFunction::MC::GetScreenData);
		lua_register(L, "MC_DrawLine", LuaFunction::MC::DrawLine);
		lua_register(L, "MC_WorldToScreen", LuaFunction::MC::WorldToScreen);
		lua_register(L, "UI_Toast", LuaFunction::LuaUI::Toast);
		lua_register(L, "UI_Window_setCanShowKeyboard", LuaFunction::LuaUI::Window_setCanShowKeyboard);
		lua_register(L, "UI_Window_newWindow", LuaFunction::LuaUI::newWindow);
		lua_register(L, "UI_Window_moveTo", LuaFunction::LuaUI::Window_moveTo);
		lua_register(L, "UI_Window_setOrientation", LuaFunction::LuaUI::Window_setOrientation);
		lua_register(L, "UI_Window_setCanMove", LuaFunction::LuaUI::Window_setCanMove);
		lua_register(L, "UI_Window_noLimit", LuaFunction::LuaUI::Window_noLimit);
		lua_register(L, "UI_Window_setTouchable", LuaFunction::LuaUI::Window_setTouchable);
		lua_register(L, "UI_Window_addView", LuaFunction::LuaUI::Window_addView);
		lua_register(L, "UI_Window_removeView", LuaFunction::LuaUI::Window_removeView);
		lua_register(L, "UI_Window_deleteWindow", LuaFunction::LuaUI::Window_deleteWindow);
		lua_register(L, "UI_View_setOnClickListener", LuaFunction::LuaUI::View_setOnClickListener);
		lua_register(L, "UI_View_setOnLongClickListener", LuaFunction::LuaUI::View_setOnLongClickListener);
		lua_register(L, "UI_View_release", LuaFunction::LuaUI::View_release);
		lua_register(L, "UI_View_setFocusable", LuaFunction::LuaUI::View_setFocusable);
		lua_register(L, "UI_Button_newButton", LuaFunction::LuaUI::newButton);
		lua_register(L, "UI_ImageView_newImageView", LuaFunction::LuaUI::newImageView);
		lua_register(L, "UI_Button_updateData", LuaFunction::LuaUI::Button_updateData);
		lua_register(L, "UI_TextView_newTextView", LuaFunction::LuaUI::newTextView);
		lua_register(L, "UI_EditText_getText", LuaFunction::LuaUI::EditText_getText);
		lua_register(L, "UI_TextView_updateData", LuaFunction::LuaUI::TextView_updateData);
		lua_register(L, "UI_EditText_newEditText", LuaFunction::LuaUI::newEditText);
		lua_register(L, "UI_EditText_updateData", LuaFunction::LuaUI::EditText_updateData);
		lua_register(L, "UI_EditText_setTextChangedListener", LuaFunction::LuaUI::EditText_setTextChangedListener);
		lua_register(L, "UI_LoadWindowFromJsonFile", LuaFunction::LuaUI::LoadWindowFromJsonFile);
		lua_register(L, "UI_LoadWindowFromJsonBase64", LuaFunction::LuaUI::LoadWindowFromJsonBase64);
	}

	bool modSpeed = false;
	float speed = 0.1f;

	bool modFancySkies = false;
	bool fancySkies = false;

	static const char KEY = 17;

	static bool isProtected(std::string const& Code) {
		const char* k = "AABBCCDD";
		if (Code.compare(0, 8, k, 0, 8) == 0) {
			return true;
		}
		else {
			return false;
		}
	}

	

	static void encrypt(std::string filePath) {
		std::string out = filePath + ".K";
		std::string Code = "AABBCCDD" + Utils::readFileIntoString(filePath.c_str());
		FILE* fp = fopen(out.c_str(), "wb+");
		for (int i = 8; i != Code.size(); i = i + 1) {
			Code[i] = Code[i] + KEY;
		}
		fwrite(Code.c_str(), 1, Code.size(), fp);
		fclose(fp);

	}
	
	void runString(std::string Code, std::string luaID) {
		

		std::lock_guard<std::recursive_mutex> lock(thread_mutex);

		lua_State* L = luaL_newstate();
		LuaEntry entry(L, luaID);
		lua_list.push_back(entry);


		luaL_openlibs(L);
		registerFunctions(L);
		if (isProtected(Code) == false) {
			if (luaL_dostring(L, Code.c_str()) == 1) {
				Utils::WriteStringToFile("/sdcard/LuaError", lua_tostring(L, -1));
			}
		}
		else {
			Code.erase(Code.begin(), Code.begin() + 8);

			for (auto it = Code.begin(); it != Code.end(); it = it + 1) {
				*it = *it - KEY;
			}

			runString(Code, luaID);
		}

	}

	void runFile(std::string path) {
		auto sv = Utils::split(path, "/");
		std::string fileName = sv[sv.size() - 1];

		runString(Utils::readFileIntoString(path.c_str()), fileName);
	}

	void runBase64Lua(const char* b64) {
		std::string code = Utils::base64_decode(b64, false);
		runString(code, "BASE64");
	}

	void onActorTick(Actor* actor) {

		std::lock_guard<std::recursive_mutex> lock(thread_mutex);
		for (auto it1 = lua_list.begin(); it1 != lua_list.end(); it1 = it1 + 1) {
			lua_State* L = it1->L;
			lua_getglobal(L, "MC_ActorTickEvent");
			lua_pushlightuserdata(L, actor);
			PCALL(L, 1, 0, 0);
		}
	}

	void onBuild(GameMode* object,BlockPos const& buildPos,int8_t rot) {
		if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->gamemode != object)return;
		std::lock_guard<std::recursive_mutex> lock(thread_mutex);
		for (auto it1 = lua_list.begin(); it1 != lua_list.end(); it1 = it1 + 1) {
			lua_State* L = it1->L;
				lua_getglobal(L, "MC_LocalPlayerClickedEvent");
				lua_pushinteger(L, buildPos.x);
				lua_pushinteger(L, buildPos.y);
				lua_pushinteger(L, buildPos.z);
				lua_pushinteger(L, rot);
				PCALL(L, 4, 0, 0);
			}
		
	}

	void onRender(MinecraftUIRenderContext* ctx) {
		std::lock_guard<std::recursive_mutex> lock(thread_mutex);

		for (auto it1 = lua_list.begin(); it1 != lua_list.end(); it1 = it1 + 1) {
			lua_State* L = it1->L;
			lua_getglobal(L, "MC_OnUIRender");
			lua_pushlightuserdata(L, ctx);
			PCALL(L, 1, 0, 0);
		}
	}

	void handleCMD(std::vector<std::string>* cmd) {
		std::lock_guard<std::recursive_mutex> lock(thread_mutex);

		for (auto it1 = lua_list.begin(); it1 != lua_list.end(); it1 = it1 + 1) {
			lua_State* L = it1->L;
			lua_getglobal(L, "MC_OnCmdEvent");
			lua_pushlightuserdata(L, cmd);
			PCALL(L, 1, 0, 0);
		}

	}

	void onAttack(GameMode* object, Actor* target) {
		if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->gamemode != object)return;

		std::lock_guard<std::recursive_mutex> lock(thread_mutex);
		for (auto it1 = lua_list.begin(); it1 != lua_list.end(); it1 = it1 + 1) {
			lua_State* L = it1->L;
			lua_getglobal(L, "MC_LocalPlayerAttackedEvent");
			lua_pushlightuserdata(L, target);
			PCALL(L, 1, 0, 0);
		}
		
	}

	void onSendPacket(Packet* packet) {
		std::lock_guard<std::recursive_mutex> lock(thread_mutex);

		for (auto it1 = lua_list.begin(); it1 != lua_list.end(); it1 = it1 + 1) {
			lua_State* L = it1->L;
			lua_getglobal(L, "MC_SendPacketEvent");
			lua_pushlightuserdata(L, packet);
			PCALL(L, 1, 0, 0);
		}
	}

	void OnNewGame(std::string const& ip) {
		std::lock_guard<std::recursive_mutex> lock(thread_mutex);

		for (auto it1 = lua_list.begin(); it1 != lua_list.end(); it1 = it1 + 1) {
			lua_State* L = it1->L;
			lua_getglobal(L, "MC_OnNewGame");
			lua_pushstring(L, ip.c_str());
			PCALL(L, 1, 0, 0);
		}
	}

	void onRenderText(MinecraftUIRenderContext* ctx,std::string *text) {
		std::lock_guard<std::recursive_mutex> lock(thread_mutex);
		for (auto it1 = lua_list.begin(); it1 != lua_list.end(); it1 = it1 + 1) {
			lua_State* L = it1->L;
			lua_getglobal(L, "MC_OnRenderText");
			lua_pushlightuserdata(L, ctx);
			lua_pushlightuserdata(L, text);
			PCALL(L, 2, 0, 0);
		}
	}
};

Script::Script()
{
	ModuleType = "Other";
	UIType = 1;

	runtime = new ScriptRuntime;
}


Script::~Script()
{
	delete runtime;
}

const char* Script::GetName()
{
	return "Script";
}

void Script::UiInit()
{
	loadDefaultLua();
}



void Script::OnTick(Actor* act){
	runtime->onActorTick(act);
}

void Script::OnCmd(std::vector<std::string>* cmd)
{
	runtime->handleCMD(cmd);

	if ((*cmd)[0] == ".RunPlugin") {
		if (cmd->size() < 2)return;
		moduleManager->executedCMD = true;
		std::string filePath = (*cmd)[1];
		mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("Loading " + filePath, "");

		load(filePath);
	}
	else if ((*cmd)[0] == ".StopLua") {
		if (cmd->size() < 2)return;
		moduleManager->executedCMD = true;
		std::string procID = (*cmd)[1];
		runtime->exit(atoi(procID.c_str()));
	}
	else if ((*cmd)[0] == ".LuaList") {
		moduleManager->executedCMD = true;
		std::lock_guard<std::recursive_mutex> lock(runtime->thread_mutex);
		for (auto it = runtime->lua_list.begin(); it != runtime->lua_list.end(); it = it + 1) {
			std::stringstream os;
			os << "LuaProcID: " << it->procID << " LuaID: " << it->id;
			mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(os.str(), "");
		}
	}
}
uint64_t (*old_MinecraftUIRenderContext_drawText)(MinecraftUIRenderContext* ctx, Font& font, const float* pos, std::string const& text, const float* color, float alpha, const float* empty, const float* idk, int i, TextMeasureData const& textData, const uintptr_t* caretMeasureData);


uint64_t new_MinecraftUIRenderContext_drawText(MinecraftUIRenderContext* ctx, Font& font, const float* pos, std::string const& text, const float* color, float alpha, const float* empty, const float* idk, int i, TextMeasureData const& textData, const uintptr_t* caretMeasureData) {
	std::string newText = text;
	moduleManager->getModule<Script>()->runtime->onRenderText(ctx, &newText);
	return (*old_MinecraftUIRenderContext_drawText)(ctx, font, pos, newText, color, alpha, empty, idk, i, textData, caretMeasureData);
}

float (*old_Player_getSpeed)(Player*);

float now_Player_getSpeed(Player* obj) {
	if (moduleManager->getModule<Script>()->runtime->modSpeed) {
		return moduleManager->getModule<Script>()->runtime->speed;
	}
	else {
		return (*old_Player_getSpeed)(obj);
	}
}

bool (*old_Options_getFancySkies)(void*);

bool now_Options_getFancySkies(void* Options) {
	if (moduleManager->getModule<Script>()->runtime->modFancySkies) {
		return moduleManager->getModule<Script>()->runtime->fancySkies;
	}
	else {
		return (*old_Options_getFancySkies)(Options);
	}
}

void Script::MinecraftInit()
{
	HackSDK::FastHook(mGameData.basePtr.MinecraftUIRenderContext_drawText, (void*)new_MinecraftUIRenderContext_drawText, (void**)&old_MinecraftUIRenderContext_drawText);
	HackSDK::FastHook(mGameData.basePtr.Options_getFancySkies, (void*)now_Options_getFancySkies, (void**)&old_Options_getFancySkies);
	HackSDK::FastHook(mGameData.basePtr.Player_getSpeed, (void*)now_Player_getSpeed, (void**)&old_Player_getSpeed);
}

void Script::loadDefaultLua() {
	runtime->runBase64Lua("QWlySnVtcCA9IGZhbHNlDQpNYXlGbHkgPSBmYWxzZQ0KU2V0Rmx5U3BlZWQgPSBmYWxzZQ0KRmx5U3BlZWQgPSAwLjUNCk5vQ2xpcCA9IGZhbHNlDQpIaWdoSnVtcCA9IGZhbHNlDQpTZXRTcGVlZCA9IGZhbHNlDQpXYWxrU3BlZWQgPSAwLjM1DQpQaGFzZSA9IGZhbHNlDQoNCkZseUV2ZW50ID0gZmFsc2UNCg0KTW9kRW5hYmxlZCA9IGZhbHNlDQoNCg0KZnVuY3Rpb24gb25Mb2NhbFBsYXllclRpY2soKQ0KCWxvY2FsIGxwID0gTUNfR2V0TG9jYWxQbGF5ZXIoKQ0KCWlmKEFpckp1bXAgPT0gdHJ1ZSkNCgl0aGVuDQoJCU1DX0FjdG9yX3NldE9uR3JvdW5kKGxwLHRydWUpDQoJZW5kDQoJaWYoTWF5Rmx5ID09IHRydWUpDQoJdGhlbg0KCQlNQ19TZXRQbGF5ZXJBYmlsaXR5KGxwLDEwLCJib29sIix0cnVlKQ0KCWVsc2UNCgkJTUNfU2V0UGxheWVyQWJpbGl0eShscCwxMCwiYm9vbCIsZmFsc2UpDQoJZW5kDQoJaWYoU2V0Rmx5U3BlZWQgPT0gdHJ1ZSkNCgl0aGVuDQoJCU1DX1NldFBsYXllckFiaWxpdHkobHAsMTMsImZsb2F0IixGbHlTcGVlZCkNCgllbHNlDQoJCU1DX1NldFBsYXllckFiaWxpdHkobHAsMTMsImZsb2F0IiwwLjEpDQoJZW5kDQoJaWYoU2V0U3BlZWQgPT0gdHJ1ZSkNCgl0aGVuDQoJCU1DX01vZFNwZWVkKHRydWUsV2Fsa1NwZWVkKQ0KCWVsc2UNCgkJTUNfTW9kU3BlZWQoZmFsc2UpDQoJZW5kDQoJaWYoRmx5RXZlbnQgPT0gdHJ1ZSkNCgl0aGVuDQoJCUZseUV2ZW50ID0gZmFsc2UNCgkJTUNfSnVtcCgpDQoJCU1DX1NldFBsYXllckFiaWxpdHkobHAsOSwiYm9vbCIsdHJ1ZSkNCgllbmQNCglpZihOb0NsaXAgPT0gdHJ1ZSkNCgl0aGVuDQoJCU1DX1NldFBsYXllckFiaWxpdHkobHAsMTcsImJvb2wiLHRydWUpDQoJZWxzZQ0KCQlNQ19TZXRQbGF5ZXJBYmlsaXR5KGxwLDE3LCJib29sIixmYWxzZSkNCgllbmQNCglpZihIaWdoSnVtcCA9PSB0cnVlKQ0KCXRoZW4NCgkJTUNfQWRkTW9iRWZmZWN0KGxwLDgsNCw4MCxmYWxzZSxmYWxzZSkNCgllbmQNCglpZihQaGFzZSA9PSB0cnVlKQ0KCXRoZW4NCgkJbG9jYWwgeCx5LHosZHgsZHksZHogPSBNQ19HZXRNb2JBQUJCKGxwKQ0KCQlNQ19TZXRNb2JBQUJCKGxwLHgseSx6LGR4LHksZHopDQoJZWxzZQ0KCQlsb2NhbCB4LHkseixkeCxkeSxkeiA9IE1DX0dldE1vYkFBQkIobHApDQoJCWlmKGR5IC0geSA8IDAuMSkNCgkJdGhlbgkNCgkJCU1DX1NldE1vYkFBQkIobHAseCx5LHosZHgseSArIDEuOCxkeikNCgkJZW5kDQoJZW5kDQplbmQNCg0KZnVuY3Rpb24gTUNfQWN0b3JUaWNrRXZlbnQoYWN0b3IpDQoJaWYoYWN0b3IgPT0gTUNfR2V0TG9jYWxQbGF5ZXIoKSkNCgl0aGVuDQoJCWlmKE1vZEVuYWJsZWQgPT0gdHJ1ZSkNCgkJdGhlbg0KCQkJb25Mb2NhbFBsYXllclRpY2soKQ0KCQllbmQNCgllbmQNCmVuZA0KDQpmdW5jdGlvbiBPcGVuTWFpbldpbmRvdygpDQoJTWFpblRleHRWaWV3ID0gbmlsDQoJTWFpbldpbiA9IFVJX0xvYWRXaW5kb3dGcm9tSnNvbkJhc2U2NCgiZXdvZ0lDSkRZVzVOYjNabElqb2dkSEoxWlN3S0lDQWlUM0pwWlc1MFlYUnBiMjRpT2lBeExBb2dJQ0pEWVc1VGFHOTNTMlY1WW05aGNtUWlPaUJtWVd4elpTd0tJQ0FpVm1sbGQzTWlPaUJiQ2lBZ0lDQjdDaUFnSUNBZ0lDSlVlWEJsSWpvZ0lsUmxlSFJXYVdWM0lpd0tJQ0FnSUNBZ0lrOXVRMnhwWTJ0TWFYTjBaVzVsY2lJNklDSkRiR2xqYTBoaGJtUnNaWElpTEFvZ0lDQWdJQ0FpUW1sdVpFZHNiMkpoYkZaaGJIVmxJam9nSWsxaGFXNVVaWGgwVm1sbGR5SXNDaUFnSUNBZ0lDSkVZWFJoSWpvZ2V3b2dJQ0FnSUNBZ0lDSlVaWGgwSWpvZ0lrMWxiblVpTEFvZ0lDQWdJQ0FnSUNKQ1lXTnJaM0p2ZFc1a1EyOXNiM0lpT2lBaUkwWkdNREF3TUNJc0NpQWdJQ0FnSUNBZ0lsUmxlSFJUYVhwbElqb2dNakFLSUNBZ0lDQWdmUW9nSUNBZ2ZRb2dJRjBLZlFvPSIpDQplbmQNCg0KZnVuY3Rpb24gQ2xvc2VNYWluV2luZG93KCkNCglVSV9WaWV3X3JlbGVhc2UoTWFpblRleHRWaWV3KQ0KCVVJX1dpbmRvd19kZWxldGVXaW5kb3coTWFpbldpbikNCglNYWluVGV4dFZpZXcgPSBuaWwNCglNYWluV2luID0gbmlsDQplbmQNCg0KZnVuY3Rpb24gT3Blbk1lbnVXaW5kb3coKQ0KCU1lbnVCYWNrID0gbmlsDQoJdHZfRmx5ID0gbmlsDQoJdHZfQWlySnVtcCA9IG5pbA0KCXR2X01heUZseSA9IG5pbA0KCXR2X1NldEZseVNwZWVkID0gbmlsDQoJdHZfTm9DbGlwID0gbmlsDQoJdHZfSGlnaEp1bXAgPSBuaWwNCgl0dl9TZXRTcGVlZCA9IG5pbA0KCXR2X1BoYXNlID0gbmlsDQoJTWVudVdpbmRvdyA9IFVJX0xvYWRXaW5kb3dGcm9tSnNvbkJhc2U2NCgiZXcwS0lDQWlRMkZ1VFc5MlpTSTZJSFJ5ZFdVc0RRb2dJQ0pQY21sbGJuUmhkR2x2YmlJNklERXNEUW9nSUNKRFlXNVRhRzkzUzJWNVltOWhjbVFpT2lCbVlXeHpaU3dOQ2lBZ0lsWnBaWGR6SWpvZ1d3MEtJQ0FnSUhzTkNpQWdJQ0FnSUNKVWVYQmxJam9nSWxSbGVIUldhV1YzSWl3TkNpQWdJQ0FnSUNKUGJrTnNhV05yVEdsemRHVnVaWElpT2lBaVEyeHBZMnRJWVc1a2JHVnlJaXdOQ2lBZ0lDQWdJQ0pDYVc1a1IyeHZZbUZzVm1Gc2RXVWlPaUFpVFdWdWRVSmhZMnNpTEEwS0lDQWdJQ0FnSWtSaGRHRWlPaUI3RFFvZ0lDQWdJQ0FnSUNKVVpYaDBJam9nSWtKaFkyc2lMQTBLSUNBZ0lDQWdJQ0FpVkdWNGRGTnBlbVVpT2lBeU1Dd05DaUFnSUNBZ0lDQWdJa0poWTJ0bmNtOTFibVJEYjJ4dmNpSTZJQ0lqUmtZd01EQXdJZzBLSUNBZ0lDQWdmUTBLSUNBZ0lIMHNEUW9nSUNBZ2V3MEtJQ0FnSUNBZ0lsUjVjR1VpT2lBaVZHVjRkRlpwWlhjaUxBMEtJQ0FnSUNBZ0lrOXVRMnhwWTJ0TWFYTjBaVzVsY2lJNklDSkRiR2xqYTBoaGJtUnNaWElpTEEwS0lDQWdJQ0FnSWtKcGJtUkhiRzlpWVd4V1lXeDFaU0k2SUNKMGRsOUdiSGtpTEEwS0lDQWdJQ0FnSWtSaGRHRWlPaUI3RFFvZ0lDQWdJQ0FnSUNKVVpYaDBJam9nSWtac2VTSXNEUW9nSUNBZ0lDQWdJQ0pVWlhoMFUybDZaU0k2SURJd0xBMEtJQ0FnSUNBZ0lDQWlRbUZqYTJkeWIzVnVaRU52Ykc5eUlqb2dJaU5HUmpBd01EQWlEUW9nSUNBZ0lDQjlEUW9nSUNBZ2ZTd05DaUFnSUNCN0RRb2dJQ0FnSUNBaVZIbHdaU0k2SUNKVVpYaDBWbWxsZHlJc0RRb2dJQ0FnSUNBaVQyNURiR2xqYTB4cGMzUmxibVZ5SWpvZ0lrTnNhV05yU0dGdVpHeGxjaUlzRFFvZ0lDQWdJQ0FpUW1sdVpFZHNiMkpoYkZaaGJIVmxJam9nSW5SMlgwNXZRMnhwY0NJc0RRb2dJQ0FnSUNBaVJHRjBZU0k2SUhzTkNpQWdJQ0FnSUNBZ0lsUmxlSFFpT2lBaVRtOURiR2x3SWl3TkNpQWdJQ0FnSUNBZ0lsUmxlSFJUYVhwbElqb2dNakFzRFFvZ0lDQWdJQ0FnSUNKQ1lXTnJaM0p2ZFc1a1EyOXNiM0lpT2lBaUkwWkdNREF3TUNJTkNpQWdJQ0FnSUgwTkNpQWdJQ0I5TEEwS0lDQWdJSHNOQ2lBZ0lDQWdJQ0pVZVhCbElqb2dJbFJsZUhSV2FXVjNJaXdOQ2lBZ0lDQWdJQ0pQYmtOc2FXTnJUR2x6ZEdWdVpYSWlPaUFpUTJ4cFkydElZVzVrYkdWeUlpd05DaUFnSUNBZ0lDSkNhVzVrUjJ4dlltRnNWbUZzZFdVaU9pQWlkSFpmUVdseVNuVnRjQ0lzRFFvZ0lDQWdJQ0FpUkdGMFlTSTZJSHNOQ2lBZ0lDQWdJQ0FnSWxSbGVIUWlPaUFpUVdseVNuVnRjQ0lzRFFvZ0lDQWdJQ0FnSUNKVVpYaDBVMmw2WlNJNklESXdMQTBLSUNBZ0lDQWdJQ0FpUW1GamEyZHliM1Z1WkVOdmJHOXlJam9nSWlOR1JqQXdNREFpRFFvZ0lDQWdJQ0I5RFFvZ0lDQWdmU3dOQ2lBZ0lDQjdEUW9nSUNBZ0lDQWlWSGx3WlNJNklDSlVaWGgwVm1sbGR5SXNEUW9nSUNBZ0lDQWlUMjVEYkdsamEweHBjM1JsYm1WeUlqb2dJa05zYVdOclNHRnVaR3hsY2lJc0RRb2dJQ0FnSUNBaVFtbHVaRWRzYjJKaGJGWmhiSFZsSWpvZ0luUjJYMU5sZEVac2VWTndaV1ZrSWl3TkNpQWdJQ0FnSUNKRVlYUmhJam9nZXcwS0lDQWdJQ0FnSUNBaVZHVjRkQ0k2SUNKVFpYUkdiSGxUY0dWbFpDSXNEUW9nSUNBZ0lDQWdJQ0pVWlhoMFUybDZaU0k2SURJd0xBMEtJQ0FnSUNBZ0lDQWlRbUZqYTJkeWIzVnVaRU52Ykc5eUlqb2dJaU5HUmpBd01EQWlEUW9nSUNBZ0lDQjlEUW9nSUNBZ2ZTd05DaUFnSUNCN0RRb2dJQ0FnSUNBaVZIbHdaU0k2SUNKVVpYaDBWbWxsZHlJc0RRb2dJQ0FnSUNBaVQyNURiR2xqYTB4cGMzUmxibVZ5SWpvZ0lrTnNhV05yU0dGdVpHeGxjaUlzRFFvZ0lDQWdJQ0FpUW1sdVpFZHNiMkpoYkZaaGJIVmxJam9nSW5SMlgwMWhlVVpzZVNJc0RRb2dJQ0FnSUNBaVJHRjBZU0k2SUhzTkNpQWdJQ0FnSUNBZ0lsUmxlSFFpT2lBaVRXRjVSbXg1SWl3TkNpQWdJQ0FnSUNBZ0lsUmxlSFJUYVhwbElqb2dNakFzRFFvZ0lDQWdJQ0FnSUNKQ1lXTnJaM0p2ZFc1a1EyOXNiM0lpT2lBaUkwWkdNREF3TUNJTkNpQWdJQ0FnSUgwTkNpQWdJQ0I5TEEwS0lDQWdJSHNOQ2lBZ0lDQWdJQ0pVZVhCbElqb2dJbFJsZUhSV2FXVjNJaXdOQ2lBZ0lDQWdJQ0pQYmtOc2FXTnJUR2x6ZEdWdVpYSWlPaUFpUTJ4cFkydElZVzVrYkdWeUlpd05DaUFnSUNBZ0lDSkNhVzVrUjJ4dlltRnNWbUZzZFdVaU9pQWlkSFpmU0dsbmFFcDFiWEFpTEEwS0lDQWdJQ0FnSWtSaGRHRWlPaUI3RFFvZ0lDQWdJQ0FnSUNKVVpYaDBJam9nSWtocFoyaEtkVzF3SWl3TkNpQWdJQ0FnSUNBZ0lsUmxlSFJUYVhwbElqb2dNakFzRFFvZ0lDQWdJQ0FnSUNKQ1lXTnJaM0p2ZFc1a1EyOXNiM0lpT2lBaUkwWkdNREF3TUNJTkNpQWdJQ0FnSUgwTkNpQWdJQ0I5TEEwS0lDQWdJSHNOQ2lBZ0lDQWdJQ0pVZVhCbElqb2dJbFJsZUhSV2FXVjNJaXdOQ2lBZ0lDQWdJQ0pQYmtOc2FXTnJUR2x6ZEdWdVpYSWlPaUFpUTJ4cFkydElZVzVrYkdWeUlpd05DaUFnSUNBZ0lDSkNhVzVrUjJ4dlltRnNWbUZzZFdVaU9pQWlkSFpmVUdoaGMyVWlMQTBLSUNBZ0lDQWdJa1JoZEdFaU9pQjdEUW9nSUNBZ0lDQWdJQ0pVWlhoMElqb2dJbEJvWVhObElpd05DaUFnSUNBZ0lDQWdJbFJsZUhSVGFYcGxJam9nTWpBc0RRb2dJQ0FnSUNBZ0lDSkNZV05yWjNKdmRXNWtRMjlzYjNJaU9pQWlJMFpHTURBd01DSU5DaUFnSUNBZ0lIME5DaUFnSUNCOUxBMEtJQ0FnSUhzTkNpQWdJQ0FnSUNKVWVYQmxJam9nSWxSbGVIUldhV1YzSWl3TkNpQWdJQ0FnSUNKUGJrTnNhV05yVEdsemRHVnVaWElpT2lBaVEyeHBZMnRJWVc1a2JHVnlJaXdOQ2lBZ0lDQWdJQ0pDYVc1a1IyeHZZbUZzVm1Gc2RXVWlPaUFpZEhaZlUyVjBVM0JsWldRaUxBMEtJQ0FnSUNBZ0lrUmhkR0VpT2lCN0RRb2dJQ0FnSUNBZ0lDSlVaWGgwSWpvZ0lsTmxkRk53WldWa0lpd05DaUFnSUNBZ0lDQWdJbFJsZUhSVGFYcGxJam9nTWpBc0RRb2dJQ0FnSUNBZ0lDSkNZV05yWjNKdmRXNWtRMjlzYjNJaU9pQWlJMFpHTURBd01DSU5DaUFnSUNBZ0lIME5DaUFnSUNCOURRb2dJRjBOQ24wPSIpDQplbmQNCg0KZnVuY3Rpb24gQ2xvc2VNZW51V2luZG93KCkNCglVSV9WaWV3X3JlbGVhc2UodHZfQWlySnVtcCkNCglVSV9WaWV3X3JlbGVhc2UodHZfRmx5KQ0KCVVJX1ZpZXdfcmVsZWFzZSh0dl9NYXlGbHkpDQoJVUlfVmlld19yZWxlYXNlKE1lbnVCYWNrKQ0KCVVJX1ZpZXdfcmVsZWFzZSh0dl9TZXRGbHlTcGVlZCkNCglVSV9WaWV3X3JlbGVhc2UodHZfSGlnaEp1bXApDQoJVUlfVmlld19yZWxlYXNlKHR2X1NldFNwZWVkKQ0KCVVJX1ZpZXdfcmVsZWFzZSh0dl9QaGFzZSkNCglVSV9XaW5kb3dfZGVsZXRlV2luZG93KE1lbnVXaW5kb3cpDQoJTWVudUJhY2sgPSBuaWwNCgl0dl9GbHkgPSBuaWwNCgl0dl9BaXJKdW1wID0gbmlsDQoJdHZfTWF5Rmx5ID0gbmlsDQoJdHZfU2V0Rmx5U3BlZWQgPSBuaWwNCgl0dl9Ob0NsaXAgPSBuaWwNCgl0dl9IaWdoSnVtcCA9IG5pbA0KCXR2X1NldFNwZWVkID0gbmlsDQoJdHZfUGhhc2UgPSBuaWwNCglNZW51V2luZG93ID0gbmlsDQplbmQNCg0KZnVuY3Rpb24gQ2xpY2tIYW5kbGVyKHZpZXcpDQoJaWYodmlldyA9PSBNYWluVGV4dFZpZXcpDQoJdGhlbg0KCQlDbG9zZU1haW5XaW5kb3coKQ0KCQlPcGVuTWVudVdpbmRvdygpDQoJCXJldHVybg0KCWVuZA0KCQ0KCWlmKHZpZXcgPT0gTWVudUJhY2spDQoJdGhlbg0KCQlPcGVuTWFpbldpbmRvdygpDQoJCUNsb3NlTWVudVdpbmRvdygpDQoJCXJldHVybg0KCWVuZA0KCU1vZEVuYWJsZWQgPSB0cnVlDQoJaWYodmlldyA9PSB0dl9GbHkpDQoJdGhlbg0KCQlVSV9Ub2FzdCgi5by65Yi26aOe6KGMIikNCgkJRmx5RXZlbnQgPSB0cnVlDQoJCXJldHVybg0KCWVuZA0KCWlmKHZpZXcgPT0gdHZfQWlySnVtcCkNCgl0aGVuDQoJCVVJX1RvYXN0KCLouI/nqboiKQ0KCQlpZihBaXJKdW1wID09IGZhbHNlKQ0KCQl0aGVuDQoJCQlBaXJKdW1wID0gdHJ1ZQ0KCQllbHNlDQoJCQlBaXJKdW1wID0gZmFsc2UNCgkJZW5kDQoJCXJldHVybg0KCWVuZA0KCWlmKHZpZXcgPT0gdHZfTWF5Rmx5KQ0KCXRoZW4NCgkJVUlfVG9hc3QoIumjnuihjOadg+mZkCIpDQoJCWlmKE1heUZseSA9PSBmYWxzZSkNCgkJdGhlbg0KCQkJTWF5Rmx5ID0gdHJ1ZQ0KCQllbHNlDQoJCQlNYXlGbHkgPSBmYWxzZQ0KCQllbmQNCgkJcmV0dXJuDQoJZW5kDQoJaWYodmlldyA9PSB0dl9TZXRTcGVlZCkNCgl0aGVuDQoJCVVJX1RvYXN0KCLkurrnianliqDpgJ8iKQ0KCQlpZihTZXRTcGVlZCA9PSBmYWxzZSkNCgkJdGhlbg0KCQkJU2V0U3BlZWQgPSB0cnVlDQoJCWVsc2UNCgkJCVNldFNwZWVkID0gZmFsc2UNCgkJZW5kDQoJCXJldHVybg0KCWVuZA0KCWlmKHZpZXcgPT0gdHZfU2V0Rmx5U3BlZWQpDQoJdGhlbg0KCQlVSV9Ub2FzdCgi6aOe6KGM5Yqg6YCfIikNCgkJaWYoU2V0Rmx5U3BlZWQgPT0gZmFsc2UpDQoJCXRoZW4NCgkJCVNldEZseVNwZWVkID0gdHJ1ZQ0KCQllbHNlDQoJCQlTZXRGbHlTcGVlZCA9IGZhbHNlDQoJCWVuZA0KCQlyZXR1cm4NCgllbmQNCglpZih2aWV3ID09IHR2X05vQ2xpcCkNCgl0aGVuDQoJCVVJX1RvYXN0KCLnqb/lopkiKQ0KCQlpZihOb0NsaXAgPT0gZmFsc2UpDQoJCXRoZW4NCgkJCU5vQ2xpcCA9IHRydWUNCgkJZWxzZQ0KCQkJTm9DbGlwID0gZmFsc2UNCgkJZW5kDQoJCXJldHVybg0KCWVuZA0KCQ0KCWlmKHZpZXcgPT0gdHZfSGlnaEp1bXApDQoJdGhlbg0KCQlVSV9Ub2FzdCgi6auY6LezIikNCgkJaWYoSGlnaEp1bXAgPT0gZmFsc2UpDQoJCXRoZW4NCgkJCUhpZ2hKdW1wID0gdHJ1ZQ0KCQllbHNlDQoJCQlIaWdoSnVtcCA9IGZhbHNlDQoJCWVuZA0KCQlyZXR1cm4NCgllbmQNCg0KCQ0KCWlmKHZpZXcgPT0gdHZfUGhhc2UpDQoJdGhlbg0KCQlVSV9Ub2FzdCgiUGhhc2UiKQ0KCQlpZihQaGFzZSA9PSBmYWxzZSkNCgkJdGhlbg0KCQkJUGhhc2UgPSB0cnVlDQoJCWVsc2UNCgkJCVBoYXNlID0gZmFsc2UNCgkJZW5kDQoJCXJldHVybg0KCWVuZA0KZW5kDQoNCk9wZW5NYWluV2luZG93KCk=");
	runtime->runBase64Lua("Ym94ID0gZmFsc2UNCmxpbmUgPSBmYWxzZQ0KDQpkcmF3TW9iID0gZmFsc2UNCg0KY3VycmVudEN0eCA9IG5pbA0KDQpmdW5jdGlvbiBEcmF3TGluZShlbnQpDQoJbG9jYWwgZXgsZXksZXosZWR4LGVkeSxlZHogPSBNQ19HZXRNb2JBQUJCKGVudCkNCglsb2NhbCB3aWR0aCxoZWlnaHQgPSBNQ19HZXRTY3JlZW5EYXRhKCkNCglsb2NhbCBzY3JlZW5YLHNjcmVlblkgPSBNQ19Xb3JsZFRvU2NyZWVuKChleCArIGVkeCkvMiwoZXkgKyBlZHkpLzIsKGV6ICsgZWR6KS8yKQ0KCU1DX0RyYXdMaW5lKGN1cnJlbnRDdHgsd2lkdGggLyAyLDAsc2NyZWVuWCxzY3JlZW5ZLDAuNSwwLjksMCwwKQ0KZW5kDQoNCmZ1bmN0aW9uIERyYXdCb3goZW50KQ0KCWxvY2FsIGV4LGV5LGV6LGVkeCxlZHksZWR6ID0gTUNfR2V0TW9iQUFCQihlbnQpDQoJTUNfRHJhd0JveDNEKGN1cnJlbnRDdHgsZXggLCBleSwgZXosIGVkeCxlZHksZWR6ICwgMC40LCAwLCAwLjksIDApDQplbmQNCg0KZnVuY3Rpb24gRHJhd0VudGl0eShlbnQpDQoJaWYoZW50ID09IE1DX0dldExvY2FsUGxheWVyKCkpDQoJdGhlbg0KCQlyZXR1cm4NCgllbmQNCglpZihib3ggPT0gdHJ1ZSkNCgl0aGVuDQoJCURyYXdCb3goZW50KQ0KCWVuZA0KDQoJaWYobGluZSA9PSB0cnVlKQ0KCXRoZW4NCgkJRHJhd0xpbmUoZW50KQ0KCWVuZA0KZW5kDQoNCmZ1bmN0aW9uIEVudGl0eUhhbmRsZXIoZW50KQ0KCWlmKGRyYXdNb2IgPT0gdHJ1ZSkNCgl0aGVuDQoJCURyYXdFbnRpdHkoZW50KQ0KCWVsc2UNCgkJaWYoTUNfQWN0b3JDbGFzc1RyZWVfaXNJbnN0YW5jZU9mKGVudCw2MykgPT0gdHJ1ZSkNCgkJdGhlbg0KCQkJRHJhd0VudGl0eShlbnQpDQoJCWVuZA0KCWVuZA0KZW5kDQoNCmZ1bmN0aW9uIE1DX09uVUlSZW5kZXIoY3R4KQ0KCWN1cnJlbnRDdHggPSBjdHgNCglpZihsaW5lID09IHRydWUpDQoJdGhlbg0KCQlNQ19EcmF3VGV4dChjdXJyZW50Q3R4LDEwLjAsMTAuMCwiwqdswqc0IExpbmUiLDEuNSkNCgllbmQNCglpZihib3ggPT0gdHJ1ZSkNCgl0aGVuDQoJCWlmKGxpbmUgPT0gdHJ1ZSkNCgkJdGhlbg0KCQkJTUNfRHJhd1RleHQoY3VycmVudEN0eCwxMC4wLDIwLjAsIsKnbMKnYSBCb3giLDEuNSkNCgkJZWxzZQ0KCQkJTUNfRHJhd1RleHQoY3VycmVudEN0eCwxMC4wLDEwLjAsIsKnbMKnYSBCb3giLDEuNSkNCgkJZW5kDQoJZW5kDQoJDQoJaWYoTUNfR2V0TG9jYWxQbGF5ZXIoKSA9PSBuaWwpDQoJdGhlbg0KCQlyZXR1cm4NCgllbmQNCglNQ19Gb3JFYWNoTW9iKCJFbnRpdHlIYW5kbGVyIikNCmVuZA0KDQpmdW5jdGlvbiBPcGVuTWFpbldpbmRvdygpDQoJTWFpblRleHRWaWV3ID0gbmlsDQoJTWFpbldpbiA9IFVJX0xvYWRXaW5kb3dGcm9tSnNvbkJhc2U2NCgiZXcwS0lDQWlRMkZ1VFc5MlpTSTZJSFJ5ZFdVc0RRb2dJQ0pQY21sbGJuUmhkR2x2YmlJNklERXNEUW9nSUNKRFlXNVRhRzkzUzJWNVltOWhjbVFpT2lCbVlXeHpaU3dOQ2lBZ0lsWnBaWGR6SWpvZ1d3MEtJQ0FnSUhzTkNpQWdJQ0FnSUNKVWVYQmxJam9nSWxSbGVIUldhV1YzSWl3TkNpQWdJQ0FnSUNKUGJrTnNhV05yVEdsemRHVnVaWElpT2lBaVEyeHBZMnRJWVc1a2JHVnlJaXdOQ2lBZ0lDQWdJQ0pDYVc1a1IyeHZZbUZzVm1Gc2RXVWlPaUFpVFdGcGJsUmxlSFJXYVdWM0lpd05DaUFnSUNBZ0lDSkVZWFJoSWpvZ2V3MEtJQ0FnSUNBZ0lDQWlWR1Y0ZENJNklDSkZVMUFpTEEwS0lDQWdJQ0FnSUNBaVFtRmphMmR5YjNWdVpFTnZiRzl5SWpvZ0lpTXdNREF3UmtZaUxBMEtJQ0FnSUNBZ0lDQWlWR1Y0ZEZOcGVtVWlPaUF5TUEwS0lDQWdJQ0FnZlEwS0lDQWdJSDBOQ2lBZ1hRMEtmUT09IikNCmVuZA0KDQpmdW5jdGlvbiBDbG9zZU1haW5XaW5kb3coKQ0KCVVJX1ZpZXdfcmVsZWFzZShNYWluVGV4dFZpZXcpDQoJVUlfV2luZG93X2RlbGV0ZVdpbmRvdyhNYWluV2luKQ0KCU1haW5UZXh0VmlldyA9IG5pbA0KZW5kDQoNCmZ1bmN0aW9uIE9wZW5NZW51V2luZG93KCkNCglNZW51QmFjayA9IG5pbA0KCXR2X0xpbmUgPSBuaWwNCgl0dl9Cb3ggPSBuaWwNCglNZW51V2luZG93ID0gVUlfTG9hZFdpbmRvd0Zyb21Kc29uQmFzZTY0KCJldzBLSUNBaVEyRnVUVzkyWlNJNklIUnlkV1VzRFFvZ0lDSlBjbWxsYm5SaGRHbHZiaUk2SURFc0RRb2dJQ0pEWVc1VGFHOTNTMlY1WW05aGNtUWlPaUJtWVd4elpTd05DaUFnSWxacFpYZHpJam9nV3cwS0lDQWdJSHNOQ2lBZ0lDQWdJQ0pVZVhCbElqb2dJbFJsZUhSV2FXVjNJaXdOQ2lBZ0lDQWdJQ0pQYmtOc2FXTnJUR2x6ZEdWdVpYSWlPaUFpUTJ4cFkydElZVzVrYkdWeUlpd05DaUFnSUNBZ0lDSkNhVzVrUjJ4dlltRnNWbUZzZFdVaU9pQWlUV1Z1ZFVKaFkyc2lMQTBLSUNBZ0lDQWdJa1JoZEdFaU9pQjdEUW9nSUNBZ0lDQWdJQ0pVWlhoMElqb2dJa0poWTJzaUxBMEtJQ0FnSUNBZ0lDQWlWR1Y0ZEZOcGVtVWlPaUF5TUN3TkNpQWdJQ0FnSUNBZ0lrSmhZMnRuY205MWJtUkRiMnh2Y2lJNklDSWpSa1l3TURBd0lnMEtJQ0FnSUNBZ2ZRMEtJQ0FnSUgwc0RRb2dJQ0FnZXcwS0lDQWdJQ0FnSWxSNWNHVWlPaUFpVkdWNGRGWnBaWGNpTEEwS0lDQWdJQ0FnSWs5dVEyeHBZMnRNYVhOMFpXNWxjaUk2SUNKRGJHbGphMGhoYm1Sc1pYSWlMQTBLSUNBZ0lDQWdJa0pwYm1SSGJHOWlZV3hXWVd4MVpTSTZJQ0owZGw5TWFXNWxJaXdOQ2lBZ0lDQWdJQ0pFWVhSaElqb2dldzBLSUNBZ0lDQWdJQ0FpVkdWNGRDSTZJQ0pFY21GM1RHbHVaU0lzRFFvZ0lDQWdJQ0FnSUNKVVpYaDBVMmw2WlNJNklESXdMQTBLSUNBZ0lDQWdJQ0FpUW1GamEyZHliM1Z1WkVOdmJHOXlJam9nSWlOR1JqQXdNREFpRFFvZ0lDQWdJQ0I5RFFvZ0lDQWdmU3dOQ2lBZ0lDQjdEUW9nSUNBZ0lDQWlWSGx3WlNJNklDSlVaWGgwVm1sbGR5SXNEUW9nSUNBZ0lDQWlUMjVEYkdsamEweHBjM1JsYm1WeUlqb2dJa05zYVdOclNHRnVaR3hsY2lJc0RRb2dJQ0FnSUNBaVFtbHVaRWRzYjJKaGJGWmhiSFZsSWpvZ0luUjJYMEp2ZUNJc0RRb2dJQ0FnSUNBaVJHRjBZU0k2SUhzTkNpQWdJQ0FnSUNBZ0lsUmxlSFFpT2lBaVJISmhkMEp2ZUNJc0RRb2dJQ0FnSUNBZ0lDSlVaWGgwVTJsNlpTSTZJREl3TEEwS0lDQWdJQ0FnSUNBaVFtRmphMmR5YjNWdVpFTnZiRzl5SWpvZ0lpTkdSakF3TURBaURRb2dJQ0FnSUNCOURRb2dJQ0FnZlEwS0lDQmREUXA5IikNCmVuZA0KDQpmdW5jdGlvbiBDbG9zZU1lbnVXaW5kb3coKQ0KCVVJX1ZpZXdfcmVsZWFzZSh0dl9MaW5lKQ0KCVVJX1ZpZXdfcmVsZWFzZSh0dl9Cb3gpDQoJVUlfVmlld19yZWxlYXNlKE1lbnVCYWNrKQ0KCVVJX1dpbmRvd19kZWxldGVXaW5kb3coTWVudVdpbmRvdykNCglNZW51QmFjayA9IG5pbA0KCXR2X0xpbmUgPSBuaWwNCgl0dl9Cb3ggPSBuaWwNCmVuZA0KDQpmdW5jdGlvbiBDbGlja0hhbmRsZXIodmlldykNCglpZih2aWV3ID09IE1haW5UZXh0VmlldykNCgl0aGVuDQoJCUNsb3NlTWFpbldpbmRvdygpDQoJCU9wZW5NZW51V2luZG93KCkNCgkJcmV0dXJuDQoJZW5kDQoJaWYodmlldyA9PSBNZW51QmFjaykNCgl0aGVuDQoJCU9wZW5NYWluV2luZG93KCkNCgkJQ2xvc2VNZW51V2luZG93KCkNCgkJcmV0dXJuDQoJZW5kDQoJaWYodmlldyA9PSB0dl9MaW5lKQ0KCXRoZW4NCgkJVUlfVG9hc3QoIuWwhOe6vyIpDQoJCWlmKGxpbmUgPT0gdHJ1ZSkNCgkJdGhlbg0KCQkJbGluZSA9IGZhbHNlDQoJCWVsc2UNCgkJCWxpbmUgPSB0cnVlDQoJCWVuZA0KCQlyZXR1cm4NCgllbmQNCglpZih2aWV3ID09IHR2X0JveCkNCgl0aGVuDQoJCVVJX1RvYXN0KCLnorDmkp7nrrHmlrnmoYYiKQ0KCQlpZihib3ggPT0gdHJ1ZSkNCgkJdGhlbg0KCQkJYm94ID0gZmFsc2UNCgkJZWxzZQ0KCQkJYm94ID0gdHJ1ZQ0KCQllbmQNCgllbmQNCmVuZA0KDQpmdW5jdGlvbiBNQ19PbkNtZEV2ZW50KGNtZCkNCg0KCWxvY2FsIHAxID0gTUNfQ29tbWFuZFBhcmFtc19nZXRTdHJpbmcoY21kLDApDQoJaWYocDEgPT0gIi5FU1AiKQ0KCXRoZW4NCgkJbG9jYWwgbnVtID0gTUNfQ29tbWFuZFBhcmFtc19zaXplKGNtZCkNCgkJaWYobnVtID09IDIpDQoJCXRoZW4NCgkJCU1DX0NvbW1hbmRfZXhlY3V0ZWQoKQ0KCQkJaWYoTUNfQ29tbWFuZFBhcmFtc19nZXRTdHJpbmcoY21kLDEpID09ICJ0cnVlIikNCgkJCXRoZW4NCgkJCQlkcmF3TW9iID0gdHJ1ZQ0KCQkJZWxzZWlmKE1DX0NvbW1hbmRQYXJhbXNfZ2V0U3RyaW5nKGNtZCwxKSA9PSAiZmFsc2UiKQ0KCQkJdGhlbg0KCQkJCWRyYXdNb2IgPSBmYWxzZQ0KCQkJZW5kDQoJCWVuZA0KCWVuZA0KZW5kDQoNCk9wZW5NYWluV2luZG93KCk=");
}

void Script::initViews() {
	Android::EditText* Script_input = mAndroid->newEditText();
	UIUtils::updateEditTextData(Script_input, "PluginPath or LuaProcID");

	Android::TextView* Script_run = mAndroid->newTextView();
	UIUtils::updateTextViewData(Script_run, "LoadPlugin", "#FF0000", 19);
	Script_run->setOnClickListener([=](Android::View*) {
		std::string filePath = Script_input->text;
		mAndroid->Toast("Loading " + filePath);
		load(filePath);
	});


	Android::TextView* Script_exit = mAndroid->newTextView();
	UIUtils::updateTextViewData(Script_exit, "Exit", "#FF0000", 19);
	Script_exit->setOnClickListener([=](Android::View*) {
		std::string id = Script_input->text;
		mAndroid->Toast("Closing " + id);
		runtime->exit(atoi(id.c_str()));
	});

	Android::TextView* Script_encrypt = mAndroid->newTextView();
	UIUtils::updateTextViewData(Script_encrypt, "Encrypt", "#FF0000", 19);
	Script_encrypt->setOnClickListener([=](Android::View*) {
		ScriptRuntime::encrypt(Script_input->text);
		mAndroid->Toast("Encrypt");
	});

	SecondWindowList.push_back(Script_input);
	SecondWindowList.push_back(Script_run);
	SecondWindowList.push_back(Script_exit);
	SecondWindowList.push_back(Script_encrypt);
}

void Script::OnBuild(GameMode* object, BlockPos const& pos, int rot)
{
	runtime->onBuild(object, pos, rot);
}

void Script::OnRender(MinecraftUIRenderContext* ctx)
{
	runtime->onRender(ctx);
}

void Script::OnAttack(GameMode* object, Actor* act)
{
	runtime->onAttack(object, act);
}

void Script::OnSendPacket(Packet* packet)
{
	runtime->onSendPacket(packet);
}

void Script::OnNewGame(std::string const& ip)
{
	runtime->OnNewGame(ip);
}

void Script::load(std::string const& filePath)
{
	auto sv = Utils::split(filePath, "/");
	auto sv1 = Utils::split(sv[sv.size() - 1], ".");
	if (sv1[1] == "py") {
		if (moduleManager->getModule<PyManager>()) {
			moduleManager->getModule<PyManager>()->loadMod(filePath.c_str());
		}
	}
	else if (sv1[1] == "mcp") {
		if (moduleManager->getModule<PyManager>()) {
			moduleManager->getModule<PyManager>()->loadMCP(filePath.c_str());
		}
	}
	else if (sv1[1] == "pypkg") {
		if (moduleManager->getModule<PyManager>()) {
			moduleManager->getModule<PyManager>()->loadPackage(filePath.c_str());
		}
	}
	else if (sv1[1] == "lua") {
		runtime->runFile(filePath);
	}
	else if (sv1[1] == "so") {
		mAndroid->loadLibrary(filePath);
	}
	else {
		std::string log = "Unknown file type ";
		log.append(sv1[1]);
		logMessage(log);
	}
}

void Script::runString(std::string Code, std::string luaID)
{
	runtime->runString(Code, luaID);
}

void Script::exit(int id)
{
	runtime->exit(id);
}
