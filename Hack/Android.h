#pragma once

#include <vector>
#include <string>
#include <stdio.h>
#include "jni.h"
#include "json/json.h"
#include <functional>
#include "ByOpen/byopen.h"

class JNIEnvManager {
public:
    JavaVM* vm0;
    bool need_detach = false;
    JNIEnvManager(JavaVM* vm);
    JNIEnv* getEnv();
    ~JNIEnvManager();

};

class Interface;

class Android{
private:
	Interface* interface;

public:
	
    JavaVM* vm;
    Android(JavaVM* currentVM);
    void loadLibrary(std::string const& libname);
	static std::string getHWId();
	
	void setMediaVolume(int v);
	void playMusic(std::string const& path);
	void Toast(std::string s);


	void handleJson(Json::Value const& val);

	class View {
	public:
		Android* android;
		~View();
		void setOnClickListener(std::function<void(View*)>);
		std::function<void(View*)> onClickListener = nullptr;
		void setOnLongClickListener(std::function<void(View*)>);
		std::function<void(View*)> onLongClickListener = nullptr;
		void release(void);
		void setFocusable(bool v);
	};


	class Button :public View {
	public:
		Button(Android* a);
		void updateData(Json::Value val);
		void updateData(std::string s);
	};

	class TextView :public View {
	public:
		TextView(Android* a);
		void updateData(Json::Value val);
		void updateData(std::string s);
	};

	class EditText :public View {
	public:
		EditText(Android* a);
		void updateData(Json::Value val);
		void updateData(std::string s);
		void setTextChangedListener(std::function<void(EditText*, std::string)>);
		std::function<void(EditText*, std::string)> textChangedListener = nullptr;
		void handle(std::string s);
		std::string text = "";
	};

	class ImageView :public View {
	public:
		ImageView(Android* a,std::string const& path);
	};

	class Switch :public View {
	public:
		Switch(Android* a);
		void updateData(Json::Value val);
		void updateData(std::string s);
		void setShowText(bool v);
		void setOnCheckedChangeListener(std::function<void(Switch*, bool)>);
		std::function<void(Switch*, bool)> onCheckedChangeListener = nullptr;
	};

	class LinearLayout :public View {
	public:
		LinearLayout(Android* a);
		void addView(View const& view);
		void removeView(View const& view);
		void setBackground(std::string const& path);
		void setOrientation(int v);
	};
	class Window {
	public:
		Android* android;
		Window(Android* a);
		~Window();
		int id;
		void noLimit(bool v);
		void setCanMove(bool v);
		void addView(View const& view);
		void removeView(View const& view);
		void deleteWindow(void);
		void setCanShowKeyboard(bool v);
		void setOrientation(int v);
		void setTouchable(bool v);
		void moveTo(int x, int y);
	};
	Window* newWindow(void);
	Switch* newSwitch(void);
	EditText* newEditText(void);
	TextView* newTextView(void);
	Button* newButton(void);
	LinearLayout* newLinearLayout(void);
	ImageView* newImageView(std::string ImagePath);
};

extern Android* mAndroid;


namespace UIUtils {

	static void updateTextViewData(Android::TextView* tv, std::string Text, std::string TextColor, std::string BackgroundColor, int TextSize) __attribute__((always_inline));

	static void updateTextViewData(Android::TextView* tv, std::string Text, std::string TextColor, std::string BackgroundColor, int TextSize) {
		Json::Value v1;
		v1["Text"] = Text;
		v1["TextColor"] = TextColor;
		v1["BackgroundColor"] = BackgroundColor;
		v1["TextSize"] = TextSize;
		tv->updateData(v1);
	}

	static void updateTextViewData(Android::TextView* tv, std::string Text, std::string TextColor, int TextSize)  __attribute__((always_inline));

	static void updateTextViewData(Android::TextView* tv, std::string Text, std::string TextColor, int TextSize) {
		Json::Value v1;
		v1["Text"] = Text;
		v1["TextColor"] = TextColor;
		v1["TextSize"] = TextSize;
		tv->updateData(v1);
	}

	static void updateEditTextData(Android::EditText* et, std::string hint)  __attribute__((always_inline));

	static void updateEditTextData(Android::EditText* et, std::string hint) {
		Json::Value v1;
		v1["Hint"] = hint;
		et->updateData(v1);
	}

	static float et_getFloat(Android::EditText* et) __attribute__((always_inline));

	static float et_getFloat(Android::EditText* et) {
		return atof(et->text.c_str());
	}

	static int et_getInt(Android::EditText* et) __attribute__((always_inline));

	static int et_getInt(Android::EditText* et) {
		return atoi(et->text.c_str());
	}

	static bool et_getBool(Android::EditText* et) __attribute__((always_inline));

	static bool et_getBool(Android::EditText* et) {
		if (et->text == "true") {
			return true;
		}
		return false;
	}
	static  Android::TextView* createSimpleTextViewWithBool(bool* b, std::string const& name) __attribute__((always_inline));
	static Android::TextView* createSimpleTextViewWithBool(bool* b, std::string const& name) {
		Android::TextView* tv = mAndroid->newTextView();
		UIUtils::updateTextViewData(tv, name, "#FF0000", 19);
		tv->setOnClickListener([=](Android::View*) {
			if (*b) {
				mAndroid->Toast(name + " disabled!");
				*b = false;
			}
			else {
				mAndroid->Toast(name + " enabled!");
				*b = true;
			}
			});
		return tv;
	}
	static  Android::EditText* createSimpleEditTextWithString(std::string* t, std::string const& name) __attribute__((always_inline));
	static Android::EditText* createSimpleEditTextWithString(std::string* t, std::string const& name) {
		Android::EditText* et = mAndroid->newEditText();
		UIUtils::updateEditTextData(et, name);
		et->setTextChangedListener([=](Android::EditText*, std::string text) {
			*t = text;
			});
		return et;
	}

	static  Android::EditText* createSimpleEditTextWithInt(int* t, std::string const& name) __attribute__((always_inline));
	static Android::EditText* createSimpleEditTextWithInt(int* t, std::string const& name) {
		Android::EditText* et = mAndroid->newEditText();
		UIUtils::updateEditTextData(et, name);
		et->setTextChangedListener([=](Android::EditText*, std::string text) {
			if (et->text != "") {
				*t = et_getInt(et);
			}
			});
		return et;
	}
}