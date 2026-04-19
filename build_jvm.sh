export PROJECT_ROOT_DIRECTORY=$(pwd)
mkdir -p lwjgl/classpath
if [ "$RELEASE" = "" ]; then
  RELEASE=$(cat RELEASE)
fi;

OPT_FLAGS=" "
if [ "$RELEASE" = "1" ]; then
  OPT_FLAGS="-D__RELEASE_FLAGS_PLACEHOLDER__ "
else
  OPT_FLAGS=$(cat debug.flags)" "
fi

echo "Release Mode (0/1): "$RELEASE
echo "Mode Flags: "$OPT_FLAGS
sleep 0.115
EMSTACKDEBUG=0
EMMETHODLOGS=0
EMSTATICCALLLOGS=0

SHIMLD=false #use a modded linker that fixes some linking issues
export SOURCE_DATE_EPOCH=315532802
EMBIN=$(dirname $(which emcc))
export SHIM_INCLUDES=$(pwd)"/patch_include/";
export EMTOOLCHAIN=$(dirname $(which emcc))/../share/emscripten
# make sure youve built libffi

export EXPOSE=$(cat export.flags)

export LIBFFI_BUILD=$(pwd)/libffi/wasm_build
export CXX=$EMTOOLCHAIN"/em++"
export CC=$EMTOOLCHAIN"/emcc"
export LD=$( [ "$SHIMLD" = "true" ] && echo -n $(pwd)"/ldshim.sh" || echo -n $CC )
export AR=$EMTOOLCHAIN"/emar"
export STRIP=true
export NM=$EMTOOLCHAIN"/emnm"
export INCL="-I"$SHIM_INCLUDES" -I"$LIBFFI_BUILD"/include ";
export CFLAGS=" -sJSPI -sWASM_BIGINT=1 -DVM_LITTLE_ENDIAN -DEMSTATICCALLLOGS=$EMSTATICCALLLOGS -DEMSTACKDEBUG=$EMSTACKDEBUG -DEMMETHODLOGS=$EMMETHODLOGS -sMAIN_MODULE=1 -sRELOCATABLE=1 -sUSE_PTHREADS=1 -sSHARED_MEMORY=1 -pthread -fPIC -fno-direct-access-external-data -fvisibility=default -Wno-macro-redefined -Wno-undef -Wno-format -Wno-format-security -Wno-unused -Wno-unused-private-field -Wno-missing-braces -Wno-unused-function -Wno-bitwise-instead-of-logical -Wno-deprecated-declarations -Wno-unused-command-line-argument -sMAIN_MODULE=1 -sRELOCATABLE=1 -DSTATIC_BUILD=1 -matomics -mbulk-memory "$INCL" "$OPT_FLAGS
export CXXFLAGS=$CFLAGS" "
export LDFLAGS="-sRELOCATABLE=1 -Wno-unused-command-line-argument -sMAIN_MODULE=1 -fPIC -fvisibility=default -sERROR_ON_UNDEFINED_SYMBOLS=0 "$EXPOSE" --no-entry "
export PRECOMPILED_HEADERS_AVAILABLE=false
export BUILD_JDK=$(readlink -f $(dirname $(which java))"/../../..")
export EXTEXE="js"
export OBJCOPY=true
export STRIP_SYMBOLS="false"
AC_BYPASS=$(cat ac_bypass.flags)

#which native jdk dependencies to add. space-separated
JDK_TARGETS="java.base"

#which software jdk dependencies to add. comma-separated
JMOD_TARGETS="java.base,java.logging,java.xml,jdk.unsupported,java.scripting,java.management,java.security.sasl,java.naming,jdk.charsets,jdk.crypto.ec,java.instrument,jdk.zipfs,java.datatransfer,java.prefs,java.desktop,java.transaction.xa,java.sql"

JMOD_NATIVE_DEPS="java.instrument java.management java.datatransfer java.prefs" #intentionally exclude java.desktop

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
    --with-jvm-features=serialgc,services,management,cds,jvmti \
    --disable-jvm-feature-shenandoahgc \
    --disable-jvm-feature-g1gc \
    --disable-jvm-feature-epsilongc \
    --disable-jvm-feature-parallelgc \
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

  echo ""  >> build/emscripten/spec.gmk
  echo "# EMPATCH"  >> build/emscripten/spec.gmk
  echo "EXTRA_MODULES := "$(echo $JMOD_TARGETS | sed 's/,/ /g') >> build/emscripten/spec.gmk
  echo "Patching release flags to bypass autoconf (i hate autoconf)"
  echo "Release Flags: "$(cat ../release.flags)
  sed -i "s|-D__RELEASE_FLAGS_PLACEHOLDER__|$(cat ../release.flags)|g" build/*/spec.gmk
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

  # ./build_jvm.sh skip will only do JMOD logic!
  if [ "$1" != "skip" ]; then
    if [ "$SHIMLD" = "true" ]; then
      echo "Shimming LD"
      emmake make -k images emscripten LOG=info LD=$LD LDCXX=$LD
    else
      echo "Not shimming LD"
      emmake make -k images emscripten LOG=info
    fi;
  fi
  
  echo "------"
  echo ">> Ignore the error, it is caused by a validation step in the makefile that cannot handle wasm binaries"
  echo "------"

  rm -rf ../wasmjdk_build/*
  mkdir -p ../wasmjdk_build/lib
  mkdir -p ../wasmjdk_build/include
  mkdir -p ../wasmjdk_build/jmod
  cp build/emscripten/jdk/lib/zero/libjvm.so ../wasmjdk_build/lib/libjvm.a #plain, solo JVM

  cp -r build/emscripten/jdk/include/* ../wasmjdk_build/include/

  mkdir -p monolith
  mkdir -p monolith/include
  mkdir -p monolith/jmod_objects
  mkdir -p monolith/jmod_soups
  echo "" > monolith/objs.txt;
  cp -r build/emscripten/jdk/include/* monolith/include/

  for targ in $JDK_TARGETS; do
    find build/emscripten/support/native/$targ -name "*.o" | grep -v "jsig.o" >> monolith/objs.txt
    echo "Adding to monolithic build: $item"
    cp -r build/emscripten/support/modules_include/$targ/* monolith/include/
  done

  #Extensive Include Directory, use if getting include errors and do lazy to do it properly
  cp -r monolith/include monolith/include_extensive
  cp monolith/include/linux/* monolith/include_extensive/
  #java.base headers
  cp -r ./src/java.base/share/native/*/*.h monolith/include_extensive/
  cp -r ./src/java.base/unix/native/*/*.h monolith/include_extensive/
  #Hotspot Headers
  cp -r ./src/hotspot/share/include/*.h monolith/include_extensive/
  cp -r ./src/hotspot/os/posix/include/*.h monolith/include_extensive/

  echo "(starting jmod natives compile)"
  echo ""
  for targ in $JMOD_NATIVE_DEPS; do
    echo "|-- Building: $targ"
    INCLUDE_FLAGS_EXPLICIT=$(find src/$targ/ -name include -type d | grep -vE "windows|macosx|aix|_headless" | sed 's/^/-I/')
    INCLUDE_FLAGS_IMPLICIT=$(find src/$targ/ -name lib* -type d | grep -vE "windows|macosx|aix|_headless" | sed 's/^/-I/')
    INCLUDE_FLAGS_COMMON=$(find src/$targ/ -path "*/native/common" -type d -exec find {} -maxdepth 1 -mindepth 1 -type d \; | grep -vE "windows|macosx|aix" | sed 's/^/-I/')
    #echo $INCLUDE_FLAGS_EXPLICIT" "$INCLUDE_FLAGS_IMPLICIT" "$INCLUDE_FLAGS_COMMON
    SUBLIBS=$(find src/$targ -type d \( -name "lib*" -o -name "common" \) | grep -vE "windows|macosx|aix|_headless")
    for lib in $SUBLIBS; do
      LIB_BASENAME=$(echo $(basename $lib) | sed 's/lib//')
      if [ ! -f ../wierd_jmod_flags/$LIB_BASENAME.flags ]; then
        mkdir -p ../weird_jmod_flags/
        touch ../weird_jmod_flags/$LIB_BASENAME.flags
      fi
      WEIRDFLAGS=$(cat ../weird_jmod_flags/$LIB_BASENAME.flags)
      LIB_BASENAME_MOREBASIC=$(echo $LIB_BASENAME | sed -E "s/_.+//")
      INCLUDE_FLAGS_LOCAL=$(find $(find ./src/$targ/ -path "*native*" -name "*$LIB_BASENAME_MOREBASIC*" -type d | grep -vE "windows|macosx|aix|_headless") -type d | sed "s/^/-I/")
      #echo $INCLUDE_FLAGS_LOCAL
      echo "    |-- Building $LIB_BASENAME for $targ"
      CFILES=$(find $lib -name "*.c")
      for cfile in $CFILES; do
        INC="-I./"$(echo $(dirname $cfile) | sed "s/$(basename $lib)/common/")
        OBJ_NAME=$targ"_$(basename $cfile .c).o"
        echo "        |-- Compiling: "$OBJ_NAME
        emcc -c $cfile -o monolith/jmod_objects/$OBJ_NAME -fPIC -I$SHIM_INCLUDES -I$LIBFFI_BUILD/include -I./monolith/include_extensive -fvisibility=default $INCLUDE_FLAGS_EXPLICIT $INCLUDE_FLAGS_IMPLICIT $INCLUDE_FLAGS_COMMON $INCLUDE_FLAGS_LOCAL $WEIRDFLAGS $INC -I./build/emscripten/support/headers/$targ/ -Wno-parentheses -matomics -mbulk-memory -sSHARED_MEMORY=1 -DX_PLATFORM=2 -DFT2_BUILD_LIBRARY=1 -D__linux__=1 -DNO_XRANDR=1 -ferror-limit=999 -DHEADLESS=1
        # if [ $? != 0 ]; then
        #   echo ""
        #   echo "Errors occured, terminating build process."
        #   exit
        # fi
      done
    done
  done

  echo "Adding libjvm to monolithic build:"
  find build/emscripten/hotspot/variant-zero/libjvm/objs -name "*.o" | grep -v "jsig.o" >> monolith/objs.txt

  echo "Adding external jmod symbols to monolithic build: "
  find monolith/jmod_objects/ -name "*.o" >> monolith/objs.txt
  echo "Adding lwjgl symbols to monolithic build: "
  find ../lwjgl/src/object_files/ -name "*.o" >> monolith/objs.txt
  
  #sort -u monolith/objs.txt -o monolith/objs.txt #dedupe opbject files

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

  chmod +x ../scripts/sub_jmods.sh
  ../scripts/sub_jmods.sh "$JMOD_TARGETS"

  echo "Finalising monolith..."
  cp -r monolith/ ../wasmjdk_build/
  rm -r monolith/
  echo "Done! check the wasmjdk_build folder."

  echo "Dumping Compiler Error Logs: ";
  echo $(cat build/emscripten/build.log | grep -A 3 -E "\.[ch](pp|xx|cc)?:[0-9]+:[0-9]+: error:")
fi;