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
#include <string>
#include <dirent.h>

#include <unistd.h>
#include <shim/fakestdin.h>
#include <gl4esinit.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

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


std::string build_classpath(const std::string& classesDir, const std::string& condimentsDir) {
    std::string classpath = classesDir;
    
    DIR* dir = opendir(condimentsDir.c_str());
    if (dir == nullptr) {
        perror("Could not open condiments directory");
        return classpath;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        
        if (filename.length() > 4 && 
            filename.compare(filename.length() - 4, 4, ".jar") == 0) {
            
            classpath += ":";
            classpath += condimentsDir;
            if (condimentsDir.back() != '/') {
                classpath += "/";
            }
            classpath += filename;
        }
    }

    closedir(dir);
    return classpath;
}


extern "C" {
    void* __real_dlsym(void* handle, const char* symbol);
    void* __real_dlopen(const char* filename, int flags);

    void* __wrap_dlopen(const char* filename, int flags) {
        return __real_dlopen(NULL, flags);
    }

    void* __wrap_dlsym(void* handle, const char* symbol) {
        if (symbol == nullptr) return nullptr;

        std::string symName(symbol);

        if (symName.find("gl") == 0 && symName.find("gl4es_") != 0) {
            std::string redirectedName = "gl4es_" + symName;
            //std::cout << "GL4ES|Redirected: " << symbol << " -> " << redirectedName << std::endl;
            
            void* addr = __real_dlsym(handle, redirectedName.c_str());
            
            if (addr) {
                //std::cout << "GL4ES|Redirected: " << symbol << " -> " << redirectedName << " [" << addr << "]" << std::endl;
                return addr;
            }
        }

        return __real_dlsym(handle, symbol);
    }
}

EMSCRIPTEN_KEEPALIVE
void* p_run_classfile(void* arg) {
    initialize_gl4es();
    JavaVM *jvm;
    JNIEnv *env;
    JavaVMInitArgs vm_args;
    
    std::cout << "\n[Pthread] Trying to init JVM in a side thread..." << std::endl;

    //add java home, upload modules file, fix class path to a root-based path
    JavaVMOption options[9];
    vm_args.nOptions = 9;
    std::string cp = build_classpath("/home/web_user/classes", "/home/web_user/condiments");
    std::cout << "Classpath: " << cp << std::endl;
    std::string classpathOpt = "-Djava.class.path=" + cp;
    options[0].optionString = const_cast<char*>(classpathOpt.c_str());
    //options[0].optionString = (char*)"-Djava.class.path=/home/web_user/condiments/Main.jar";
    options[1].optionString = (char*)"-Djava.home=/";
    options[2].optionString = (char*)"--enable-native-access=ALL-UNNAMED";
    options[3].optionString = (char*)"-Xss16M";
    options[4].optionString = (char*)"-Dsun.boot.library.path=/lib";
    options[5].optionString = (char*)"--module-path=/home/web_user/wasmjdk/lib/modules";
    options[6].optionString = (char*)"-Dorg.lwjgl.util.Debug=true";
    options[7].optionString = (char*)"-Dorg.lwjgl.util.DebugLoader=true";
    //options[8].optionString = (char*)"-Xcheck:jni";
    //options[8].optionString = (char*)"-Dsun.misc.URLClassPath.debug=true";
    //options[9].optionString = (char*)"-Xlog:class+load=info";
    options[8].optionString = (char*)"-Dos.name=Linux"; //just lie

    vm_args.version = JNI_VERSION_21;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = false;

    jint res = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);
    if (res != JNI_OK) {
        std::cerr << "\n[Pthread] JNI Failed to create a java vm :(. err: " << res << std::endl;
        return nullptr;
    };

    char* name = (char*)arg;
    std::cout << "Finding class: " << name << " (" << strlen(name) << ")" << std::endl;
    jclass cls = env->FindClass(name);
    if (env->ExceptionCheck()) {
        std::cerr << "[JNI ERROR] Could not find class: " << name << std::endl;
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    if (cls != nullptr) {
        jmethodID mid = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
        if (mid != nullptr) {
            if (env->ExceptionCheck() || mid == nullptr) {
                std::cerr << "[JNI ERROR] Could not find 'main' method in class 'Main'" << std::endl;
                env->ExceptionDescribe();
                env->ExceptionClear();
                return nullptr;
            }
            jobjectArray args = env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
            std::cin.clear();
            env->CallStaticVoidMethod(cls, mid, args);
            if (env->ExceptionCheck()) {
                std::cerr << "[Java Runtime Error] An exception occurred in Main.main:" << std::endl;
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
    }

    jvm->DestroyJavaVM();
    std::cout << "[PThread] JVM Exited safely!!" << std::endl;
    return nullptr;
}

EMSCRIPTEN_KEEPALIVE
void run_classfile_proxy(char* cname) {
    pthread_t thread_id;
    int rc = pthread_create(&thread_id, NULL, p_run_classfile, (void*)cname);
    
    if (rc) {
        std::cerr << "Error: Unable to create thread," << rc << std::endl;
        return;
    }
    
    pthread_detach(thread_id);
    std::cout << "[Main] JVM thread dispatched. Main thread is now free!" << std::endl;
}




GLFWwindow* window;
void drawtest() {
    static float timer = 0;
    timer += 0.02f;
    float bgR = (sin(timer) * 0.5f) + 0.5f;
    //std::cout << "frame" << std::endl;
    glClearColor(bgR, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDisable(GL_ALPHA_TEST);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    static float r = 0;
    glRotatef(r += 2.0f, 0, 0, 1);
    glBegin(GL_TRIANGLES);
        glColor3f(1, 0, 0); glVertex3f(-0.5, -0.5, 0);
        glColor3f(0, 1, 0); glVertex3f(0.5, -0.5, 0);
        glColor3f(0, 0, 1); glVertex3f(0, 0.5, 0);
    glEnd();
    glFlush();

    glfwSwapBuffers(window);
    glfwPollEvents();
}

int frametest() {
    if (!glfwInit()) return -1;

    setenv("GL4ES_NOTHREADS", "1", 1);
    setenv("GL4ES_NO_GLSL_CACHE", "1", 1);
    initialize_gl4es();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "gl4es Test", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glViewport(0, 0, 640, 480);

    // while (!glfwWindowShouldClose(window)) {
    //     drawtest(window);
    // }
    emscripten_set_main_loop(drawtest, 0, 1);

    //glfwTerminate();
    return 0;
}