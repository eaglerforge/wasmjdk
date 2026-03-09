#include <iostream>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>

#include <jni.h>
#include <iostream>

int main() {
    JavaVM *jvm;
    JNIEnv *env;
    JavaVMInitArgs vm_args;
    JavaVMOption options[3];

    //add java home, upload modules file, fix class path to a root-based path
    options[0].optionString = (char*)"-Djava.class.path=/home/web_user/classes";
    options[1].optionString = (char*)"--module-path=/home/web_user/wasmjdk/lib/modules";
    options[2].optionString = (char*)"--enable-native-access=ALL-UNNAMED";

    vm_args.version = JNI_VERSION_21;
    vm_args.nOptions = 3;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = false;

    jint res = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);
    if (res != JNI_OK) {
        std::cerr << "JNI Failed to create a java vm :(" << std::endl;
        return 1;
    };

    jclass cls = env->FindClass("Main");
    if (cls != nullptr) {
        jmethodID mid = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
        if (mid != nullptr) {
            jobjectArray args = env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
            env->CallStaticVoidMethod(cls, mid, args);
        }
    }

    jvm->DestroyJavaVM();
    return 0;
}