#!/bin/sh

VER="3.4.1"
mkdir -p lwjgl
cd lwjgl

mkdir -p classpath
cd classpath
unzip -o *.zip
rm *.zip
rm *-natives-linux.jar
cd ..

rm -rf src
wget https://github.com/LWJGL/lwjgl3/archive/refs/tags/$VER.zip
unzip $VER.zip
rm $VER.zip
mv lwjgl* src


export EMTOOLCHAIN=$(dirname $(which emcc))/../share/emscripten
export EMCC=$(which emcc)
export EMCPP=$(which em++)

cd src
rm -r modules/lwjgl/core/src/main/c/libffi/*
rm -r object_files
mkdir object_files

ant -Dtarget=linux -Dlocal.arch=x86_64 \
    -Dtoolchain.prefix=$EMTOOLCHAIN \
    -Dtoolchain.sysroot=$EMTOOLCHAIN \
    -Dkotlin.parallel.backend=false \
    -Dbin.gcc=$EMCC -Dbin.g++=$EMCPP -Dbin.linker=$EMCC -Ddebug=true \
    compile-templates release

export CFLAGS="-DLWJGL_LINUX=1 -DLWJGL_WASM=1 -Wno-unused-command-line-argument -isystem ../../patch_include/system_override -fvisibility=default -sEXPORT_ALL -fPIC -sRELOCATABLE"

TARGETS="glfw opengles core opengl egl"
#INC=$(find . -name "*.h" -exec dirname {} + | sort -u | sed 's/^/-I/')
INC="-I./modules/lwjgl/core/src/main/c -I./modules/lwjgl/core/src/main/c/linux -I../../libffi/wasm_build/include"
echo $INC
echo ""
for targ in $TARGETS
do
  SOURCE_FILES=$(find ./modules/lwjgl/$targ -name *.c)
  LOCAL_INC=$(find ./modules/lwjgl/$targ -name "*.h" -exec dirname {} + | sort -u | sed 's/^/-I/')
  for file in $SOURCE_FILES
  do
    echo "Direct compiling file $file in $targ"
    sed -i 's/"ffi.h"/<ffi.h>/g' $file
    emcc -c $file -o object_files/$(basename $file .c).o $LOCAL_INC $INC -I../../wasmjdk_build/include -I../../wasmjdk_build/include/linux -I../../libffi/wasm_build/include -L../../libffi/wasm_build/lib -lffi $CFLAGS -sFULL_ES2 -sFULL_ES3 -sJSPI
  done
done
echo "it is done"