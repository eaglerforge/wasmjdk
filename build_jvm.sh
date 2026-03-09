SHIMLD=false #use a fake linker that never returns errors
export SOURCE_DATE_EPOCH=315532802
EMBIN=$(dirname $(which emcc))
export SHIM_INCLUDES=$(pwd)"/patch_include/";
export EMTOOLCHAIN=$(dirname $(which emcc))/../share/emscripten
# make sure youve built libffi

export EXPOSE=$(cat export_flags)

export LIBFFI_BUILD=$(pwd)/libffi/wasm_build
export CXX=$EMTOOLCHAIN"/em++"
export CC=$EMTOOLCHAIN"/emcc"
export LD=$( [ "$SHIMLD" = "true" ] && echo -n $(pwd)"/ldshim.sh" || echo -n $CC )
export AR=$EMTOOLCHAIN"/emar"
export STRIP=true
export NM=$EMTOOLCHAIN"/emnm"
export INCL="-I"$SHIM_INCLUDES" -I"$LIBFFI_BUILD"/include";
export CFLAGS="-sUSE_PTHREADS=1 -sSHARED_MEMORY=1 -pthread -O0 -g -gseparate-dwarf -gsource-map -fPIC -fvisibility=default -Wno-macro-redefined -Wno-undef -Wno-format -Wno-format-security -Wno-unused -Wno-unused-private-field -Wno-missing-braces -Wno-unused-function -Wno-bitwise-instead-of-logical -Wno-deprecated-declarations -Wno-unused-command-line-argument -sMAIN_MODULE=1 -sRELOCATABLE=1 "$INCL
export CXXFLAGS=$CFLAGS
export LDFLAGS="-sRELOCATABLE=1 -Wno-unused-command-line-argument -sMAIN_MODULE=1 -fPIC -fvisibility=default -sERROR_ON_UNDEFINED_SYMBOLS=0 "$EXPOSE" --no-entry "
export PRECOMPILED_HEADERS_AVAILABLE=false
export BUILD_JDK=$(readlink -f $(dirname $(which java))"/../../..")
export EXTEXE="yes"
export OBJCOPY=true
export STRIP_SYMBOLS="false"
# if still broken, remove libffi to configure, and bypass the error message
# and instead just manually link to it using LDFLAGS and CFLAGS
if [ "$1" = "config" ]; then
  cp libffi/wasm_build/lib/libffi.a libffi/wasm_build/lib/libffi.so.0
  cd jdk
  rm -rf build/*

  emconfigure bash configure \
    --with-toolchain-path=$EMTOOLCHAIN \
    --with-conf-name=emscripten \
    --with-tools-dir=$EMTOOLCHAIN \
    --openjdk-target=i686-unknown-linux-gnu \
    --with-jvm-variants=zero \
    --with-libffi-include="$LIBFFI_BUILD"/include \
    --with-libffi-lib="$LIBFFI_BUILD"/lib \
    --disable-libffi-bundling \
    --enable-headless-only \
    --without-cups --with-freetype=bundled --without-fontconfig \
    --with-alsa=bundled --with-libpng=bundled \
    --with-native-debug-symbols=internal \
    --enable-precompiled-headers=no --disable-warnings-as-errors \
    --with-extra-cflags="$CFLAGS" --with-extra-cxxflags="$CFLAGS" --with-extra-ldflags="$LDFLAGS" \
    --with-build-jdk="$BUILD_JDK" --with-boot-jdk="$BUILD_JDK" \
    AR=$AR STRIP=$STRIP CXX=$CXX CC=$CC NM=$NM ar=$AR strip=$STRIP cxx=$CXX cc=$CC nm=$NM LD=$LD LDCXX=$LD STRIP_SYMBOLS=$STRIP_SYMBOLS
else
  cp libffi/wasm_build/lib/libffi.a libffi/wasm_build/lib/libffi.so.0
  cd jdk

  find build/emscripten/support -type f -exec touch -t 202601010000.05 {} + #fix zip throwing timestamp errors
  
  #or LOG=info or LOG=trace. use SHELL="bash -x" for too many logs
  echo ""
  echo "# NOTICE #############################################"
  echo "# Due to how OpenJDK structures it's module system,  #"
  echo "# there may be compilation errors due to unsupported #"
  echo "# modules such as java.desktop being force compiled  #"
  echo "######################################################"
  echo ""
  sleep 1.5

  if [ "$SHIMLD" = "true" ]; then
    emmake make -k images emscripten LOG=info LD=$LD LDCXX=$LD
  else
    emmake make -k images emscripten LOG=info
  fi
  
  echo "."
  echo "Ignore the error, it is caused by a validation step in the makefile that cannot handle wasm binaries"
  echo "."

  rm -rf ../wasmjdk_build/*
  mkdir -p ../wasmjdk_build/lib
  mkdir -p ../wasmjdk_build/include
  mkdir -p ../wasmjdk_build/jmod
  cp build/emscripten/jdk/lib/zero/libjvm.so ../wasmjdk_build/lib/libjvm.a #plain, solo JVM


  cp -r build/emscripten/jdk/include/* ../wasmjdk_build/include/


  mkdir -p monolith
  mkdir -p monolith/include
  echo "" > monolith/objs.txt;
  cp -r build/emscripten/jdk/include/* monolith/include/

  
  JDK_TARGETS="java.base" #which native jdk dependencies to add. space-separated
  JMOD_TARGETS="java.base,java.logging,java.xml,jdk.unsupported" #which software jdk dependencies to add. comma-separated

  for targ in $JDK_TARGETS; do
    find build/emscripten/support/native/$targ -name "*.o" >> monolith/objs.txt
    echo "Adding to monolithic build: $item"
    cp -r build/emscripten/support/modules_include/$targ/* monolith/include/
  done
  echo "Adding libjvm to monolithic build:"
  find build/emscripten/hotspot/variant-zero/libjvm/objs -name "*.o" >> monolith/objs.txt
  
  emar rcs monolith/libjvm.a @monolith/objs.txt #all the modules rolled into one

  echo "Copying .jmod files..."
  cp build/emscripten/images/jmods/*.jmod ../wasmjdk_build/jmod/

  echo "Culling unused .jmod files..."
  for file in ../wasmjdk_build/jmod/*.jmod; do
      filename="${file##*/}"
      module_name="${filename%.jmod}"

      case ",$JMOD_TARGETS," in
          *",$module_name,"*)
              echo "  [KEEP] $file"
              ;;
          *)
              echo "  [DEL]  $file"
              rm "$file"
              ;;
      esac
  done

  chmod +x ../sub_jmods.sh
  ../sub_jmods.sh "$JMOD_TARGETS"

  echo "Finalising monolith..."
  cp -r monolith/ ../wasmjdk_build/
  rm -r monolith/
  echo "Done! check the wasmjdk_build folder."
fi
