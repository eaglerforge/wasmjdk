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
    JavaVMOption options[2];

    int x = 4;

    options[0].optionString = (char*)"-Djava.class.path=.";

    options[1].optionString = (char*)"--enable-native-access=ALL-UNNAMED";

    vm_args.version = JNI_VERSION_21;
    vm_args.nOptions = 2;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = false;

    x = 7;

    x = 3;

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