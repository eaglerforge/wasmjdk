#include <iostream>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>

#include <jni.h>
#include <iostream>
#include <pthread.h>
#include <emscripten.h>
#include <ffi.h>
#include <dlfcn.h>
#include <cstdint>
#include <limits.h>

extern "C" EMSCRIPTEN_KEEPALIVE void sample_function(JNIEnv* env, jobject unsafe, jobject obj, jlong offset, jobject e_h, jobject x_h) {
    std::cout << "Write Offset: " << offset << std::endl;
}

EMSCRIPTEN_KEEPALIVE
int ffi_testbed() {
    std::cout << "unlong: " << sizeof(int) << std::endl;
    std::cout << "long: " << sizeof(long int) << std::endl;
    std::cout << "longer long: " << sizeof(long long int) << std::endl;
    // Step 1: Use dlsym to get the address of the function
    void* handle = dlopen(nullptr, RTLD_NOW); // Load the current executable
    if (!handle) {
        std::cerr << "Cannot open library: " << dlerror() << std::endl;
        return 1;
    }

    // Step 2: Load the symbols of the function
    // Reset errors
    dlerror();
    typedef void (*sample_func_t)(JNIEnv*, jobject, jobject, jlong, jobject, jobject);
    sample_func_t sample_function_ptr = (sample_func_t)dlsym(handle, "sample_function");
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol 'sample_function': " << dlsym_error << std::endl;
        dlclose(handle);
        return 1;
    }

    ffi_cif cif;
    ffi_type *args[6];
    void *values[6];

    args[0] = &ffi_type_uint32;
    args[1] = &ffi_type_uint32;
    args[2] = &ffi_type_uint32;
    args[3] = &ffi_type_uint64;
    args[4] = &ffi_type_uint32;
    args[5] = &ffi_type_uint32;

    values[0] = new uint32_t(62);
    values[1] = new uint32_t(52);
    values[2] = new uint32_t(42);
    values[3] = new uint64_t(69420); //new long int(69420);
    values[4] = new uint32_t(32);
    values[5] = new uint32_t(22);

    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 6, &ffi_type_void, args) != FFI_OK) {
        std::cerr << "ffi_prep_cif failed!" << std::endl;
        dlclose(handle);
        return 1;
    }

    ffi_call(&cif, (void (*)()) sample_function_ptr, nullptr, values);

    delete static_cast<uint32_t*>(values[0]);
    delete static_cast<uint32_t*>(values[1]);
    delete static_cast<uint32_t*>(values[2]);
    delete static_cast<uint64_t*>(values[3]);
    delete static_cast<uint32_t*>(values[4]);
    delete static_cast<uint32_t*>(values[5]);
    
    dlclose(handle);
    
    return 0;
}


EMSCRIPTEN_KEEPALIVE
void* p_run_classfile(void* arg) {
    JavaVM *jvm;
    JNIEnv *env;
    JavaVMInitArgs vm_args;
    
    std::cout << "\n[Pthread] Trying to init JVM in a side thread..." << std::endl;

    //add java home, upload modules file, fix class path to a root-based path
    JavaVMOption options[4];
    options[0].optionString = (char*)"-Djava.class.path=/home/web_user/classes";
    options[1].optionString = (char*)"--module-path=/home/web_user/wasmjdk/lib/modules";
    options[2].optionString = (char*)"--enable-native-access=ALL-UNNAMED";
    options[3].optionString = (char*)"-Xss16M";
    vm_args.nOptions = 4;

    vm_args.version = JNI_VERSION_21;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = false;

    jint res = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);
    if (res != JNI_OK) {
        std::cerr << "\n[Pthread] JNI Failed to create a java vm :(" << std::endl;
        return nullptr;
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
    std::cout << "[PThread] JVM Exited safely!!" << std::endl;
    return nullptr;
}

EMSCRIPTEN_KEEPALIVE
void run_classfile_proxy() {
    pthread_t thread_id;
    int rc = pthread_create(&thread_id, NULL, p_run_classfile, NULL);
    
    if (rc) {
        std::cerr << "Error: Unable to create thread," << rc << std::endl;
        return;
    }
    
    pthread_detach(thread_id);
    std::cout << "[Main] JVM thread dispatched. Main thread is now free!" << std::endl;
}