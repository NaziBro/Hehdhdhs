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
#include "Hack/ModuleManager.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm,void *reserved);

jint JNICALL JNI_OnLoad(JavaVM *vm,void *reserved){
    moduleManager->jniOnLoad(vm);
    return JNI_VERSION_1_6;
}