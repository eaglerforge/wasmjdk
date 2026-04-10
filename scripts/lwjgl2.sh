LWJGL="lwjgl"; VER="lwjgl2.9.3"; TARGET="jars"; #legacy version

rm -rf src
wget https://github.com/LWJGL/$LWJGL/archive/refs/tags/$VER.zip
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

JAVA_HOME=$JAVA8_HOME ant -Dtarget=linux -Dlocal.arch=x86_64 \
    -Dtoolchain.prefix=$EMTOOLCHAIN \
    -Dtoolchain.sysroot=$EMTOOLCHAIN \
    -Dkotlin.parallel.backend=false \
    -Dbin.gcc=$EMCC -Dbin.g++=$EMCPP -Dbin.linker=$EMCC -Ddebug=true -Dsource=1.8 -Dtarget=1.8 -Dcompiler=$EMCC -Dlinker=$EMCC \
    $TARGET release

echo ""
echo "Calling compile_native to generate headers";
JAVA_HOME=$JAVA8_HOME ant -Dtarget=linux -Dlocal.arch=x86_64 \
    -Dtoolchain.prefix=$EMTOOLCHAIN \
    -Dtoolchain.sysroot=$EMTOOLCHAIN \
    -Dkotlin.parallel.backend=false \
    -Dbin.gcc=$EMCC -Dbin.g++=$EMCPP -Dbin.linker=$EMCC -Ddebug=true -Dsource=1.8 -Dtarget=1.8 -Dcompiler=$EMCC -Dlinker=$EMCC \
    $TARGET compile_native > /dev/null 2>&1
echo "Generated headers!";
sleep 1

export CFLAGS="-DLWJGL_LINUX=1 -DLWJGL_WASM=1 -Wno-unused-command-line-argument -isystem ../../patch_include/system_override -fvisibility=default -sEXPORT_ALL -fPIC -sRELOCATABLE"

TARGET="opengl" #you can't have both opengl and opengles
#INC=$(find . -name "*.h" -exec dirname {} + | sort -u | sed 's/^/-I/')
INC="-I./src/native/common/$TARGET -I./src/native/linux/$TARGET -I./src/native/common/ -I./src/native/linux/ -I./src/native/generated/ ./src/native/common/opengl/ ./src/native/common/opengles/ -I../gl4es/include"
TOOLS_FILES=$(find ./src/native/linux/ ./src/native/common ./src/native/generated -maxdepth 1 -name *.c)
echo $INC
echo ""

for file in $TOOLS_FILES
do
  echo "[TOOLS] Direct compiling file $file"
  sed -i 's/"ffi.h"/<ffi.h>/g' $file
  emcc -c $file -o object_files/$(basename $file .c).o $INC -I../../wasmjdk_build/include -I../../wasmjdk_build/include/linux -I../../libffi/wasm_build/include -L../../libffi/wasm_build/lib -lffi $CFLAGS -sFULL_ES2 -sFULL_ES3 -sJSPI -D__EMSCRIPTEN__=1
done
targ=$TARGET
SOURCE_FILES=$(find ./src/native/linux/$targ ./src/native/generated/$targ ./src/native/common/$targ -name *.c)
LOCAL_INC="-I./src/native/common/$targ "$(find ./src/native/$targ -name "*.h" -exec dirname {} + | sort -u | sed 's/^/-I/')

for file in $SOURCE_FILES
do
  #echo $LOCAL_INC
  #echo $file
  echo "Direct compiling file $file in $targ"
  sed -i 's/"ffi.h"/<ffi.h>/g' $file
  emcc -c $file -o object_files/$(basename $file .c).o $LOCAL_INC $INC -I../../wasmjdk_build/include -I../../wasmjdk_build/include/linux -I../../libffi/wasm_build/include -L../../libffi/wasm_build/lib -lffi $CFLAGS -sFULL_ES2 -sFULL_ES3 -sJSPI -D__EMSCRIPTEN__=1
done

rm -r ../classpath
mkdir ../classpath
cp dist/lwjgl-*.*.*.zip ../classpath/


echo "it is done"