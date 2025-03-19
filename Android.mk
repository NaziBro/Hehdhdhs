

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := hack


SRC_FILS := $(wildcard $(LOCAL_PATH)/*/*.s $(LOCAL_PATH)/*/*/*.s $(LOCAL_PATH)/*/*/*/*.s $(LOCAL_PATH)/*/*/*/*/*.s $(LOCAL_PATH)/*/*.c $(LOCAL_PATH)/*/*/*.c $(LOCAL_PATH)/*/*/*/*.c $(LOCAL_PATH)/*/*/*/*/*.c $(LOCAL_PATH)/*/*.cpp $(LOCAL_PATH)/*.cpp $(LOCAL_PATH)/*/*/*.cpp $(LOCAL_PATH)/*/*/*/*.cpp $(LOCAL_PATH)/*/*/*/*/*.cpp)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/python/Include \
$(LOCAL_PATH)/python/Objects \
$(LOCAL_PATH)/python/Parser \
$(LOCAL_PATH)/python/Python \
$(LOCAL_PATH)/Hack/NBT/include \
$(LOCAL_PATH)/Hack/CBrotli/include 


LOCAL_SRC_FILES :=  \ $(SRC_FILS:$(LOCAL_PATH)/%=%)
				
LOCAL_CPPFLAGS := -fexceptions	-fdeclspec -std=c++17  -frtti -DHAVE_NEON=1

LOCAL_LDLIBS += -lz -llog -landroid -lEGL -lGLESv3 -lOpenSLES

LOCAL_CPPFLAGS += -fvisibility=hidden

LOCAL_CFLAGS += -fvisibility=hidden

include $(BUILD_SHARED_LIBRARY)