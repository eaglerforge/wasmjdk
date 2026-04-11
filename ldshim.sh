#!/bin/sh

REAL_EMCC=$(which emcc)
LOG_FILE=$PROJECT_ROOT_DIRECTORY/linker.log
JVM_LIB=$PROJECT_ROOT_DIRECTORY"/jdk/build/emscripten/support/modules_libs/java.base/zero/libjvm.so"
JAVA_LIB=$PROJECT_ROOT_DIRECTORY"/jdk/build/emscripten/support/modules_libs/java.base/libjava.so"
NET_LIB=$PROJECT_ROOT_DIRECTORY"/jdk/build/emscripten/support/modules_libs/java.base/libnet.so"
VERIFY_LIB=$PROJECT_ROOT_DIRECTORY"/jdk/build/emscripten/support/modules_libs/java.base/libverify.so"
JIMAGE_LIB=$PROJECT_ROOT_DIRECTORY"/jdk/build/emscripten/support/modules_libs/java.base/libjimage.so"
NIO_LIB=$PROJECT_ROOT_DIRECTORY"/jdk/build/emscripten/support/modules_libs/java.base/libnio.so"
JSIG_LIB=$PROJECT_ROOT_DIRECTORY"/jdk/build/emscripten/support/modules_libs/java.base/libjsig.so"
JLI_LIB=$PROJECT_ROOT_DIRECTORY"/jdk/build/emscripten/support/modules_libs/java.base/libjli.so"
ZIP_LIB=$PROJECT_ROOT_DIRECTORY"/jdk/build/emscripten/support/modules_libs/java.base/libzip.so"
FALLBACKLINKER_LIB=$PROJECT_ROOT_DIRECTORY"/jdk/build/emscripten/support/modules_libs/java.base/libfallbackLinker.so"

echo "--- SHIM START ---" >> "$LOG_FILE"
echo "IN: $@" >> "$LOG_FILE"
echo $PROJECT_ROOT_DIRECTORY >> "$LOG_FILE"
NEW_ARGS=()

for arg in "$@"; do
    case "$arg" in
        -Wl,-z,origin)
            echo "STRIPPED: $arg" >> "$LOG_FILE"
            ;;
        -Wl,-rpath,*)
            echo "STRIPPED: $arg" >> "$LOG_FILE"
            ;;
        -ljvm)
            NEW_ARGS+=("$JVM_LIB")
            echo "REPLACED -ljvm WITH: $JVM_LIB" >> "$LOG_FILE"
            ;;
        -ljava)
            NEW_ARGS+=("$JAVA_LIB")
            echo "REPLACED -ljava WITH: $JAVA_LIB" >> "$LOG_FILE"
            ;;
        -lnet)
            NEW_ARGS+=("$NET_LIB")
            echo "REPLACED -lnet WITH: $NET_LIB" >> "$LOG_FILE"
            ;;
        -lverify)
            NEW_ARGS+=("$VERIFY_LIB")
            echo "REPLACED -lverify WITH: $VERIFY_LIB" >> "$LOG_FILE"
            ;;
        -ljimage)
            NEW_ARGS+=("$JIMAGE_LIB")
            echo "REPLACED -ljimage WITH: $JIMAGE_LIB" >> "$LOG_FILE"
            ;;
        -lnio)
            NEW_ARGS+=("$NIO_LIB")
            echo "REPLACED -lnio WITH: $NIO_LIB" >> "$LOG_FILE"
            ;;
        -ljsig)
            NEW_ARGS+=("$JSIG_LIB")
            echo "REPLACED -ljsig WITH: $JSIG_LIB" >> "$LOG_FILE"
            ;;
        -ljli)
            NEW_ARGS+=("$JLI_LIB")
            echo "REPLACED -ljli WITH: $JLI_LIB" >> "$LOG_FILE"
            ;;
        -lzip)
            NEW_ARGS+=("$ZIP_LIB")
            echo "REPLACED -lzip WITH: $ZIP_LIB" >> "$LOG_FILE"
            ;;
        -lfallbackLinker)
            NEW_ARGS+=("$FALLBACKLINKER_LIB")
            echo "REPLACED -lfallbackLinker WITH: $FALLBACKLINKER_LIB" >> "$LOG_FILE"
            ;;
        *)
            NEW_ARGS+=("$arg")
            ;;
    esac
done

echo "OUT: ${NEW_ARGS[*]}" >> "$LOG_FILE"

exec "$REAL_EMCC" "${NEW_ARGS[@]}"