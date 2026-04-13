#!/bin/sh

mkdir -p lwjgl
cd lwjgl

rm -rf gl4es
git clone --depth=1 https://github.com/ptitseb/gl4es.git
cd gl4es
sed -i 's/-fvisibility=hidden/-fvisibility=default/g' CMakeLists.txt #WE love ducttape and cableties
mkdir build && cd build
emcmake cmake .. -DNOX11=ON -DNOEGL=ON -DDEFAULT_ES=3 -DPLATFORM=EMSCRIPTEN -DSTATICLIB=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-fPIC -sRELOCATABLE=1 -sSHARED_MEMORY=1 -matomics -mbulk-memory -pthread -fvisibility=default "
emmake make -j$(nproc)
cd ..
mv lib/libGL.a lib/libGL4ES.a
cd ..

if [ "$1" = "gl" ]; then
  exit
fi

TRG=../scripts/lwjgl3.sh
chmod +x $TRG
$TRG

mkdir -p classpath
cd classpath
unzip -o *.zip
rm *.zip
rm *-natives-linux.jar
mv lwjgl-*/jar/*.jar .
find . -mindepth 1 -type d -exec rm -rf {} +
cd ..