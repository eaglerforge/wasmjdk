#!/bin/sh
JMOD_DIR="../wasmjdk_build/jmod"
TEMP_DIR="./temp_jmod_fix"
TARGET_PLATFORM="linux-x86"
JAVA_VER="25-internal"

# START - recreate any missing software-only jmods (no clue why they are missing but who cares frfr)
IFS=',' read -ra ADDR <<< "$1"
for i in "${ADDR[@]}"; do
    echo "Checking jmod: $i"
    if [ $i != "java.base" ]; then
        if [ -d "src/"$i"/share" ]; then
            if [ -d "src/"$i"/unix" ] || [ -d "src/"$i"/share/native" ] || [ -d "src/"$i"/linux" ]; then
                echo "WARNING: jmod $i is not entirely cross platform! Please ensure required symbols are included"
            fi
            if [ ! -f "../wasmjdk_build/jmod/$i.jmod" ]; then
                echo "Recreating..: "$i
                jmod create \
                    --class-path build/emscripten/jdk/modules/$i \
                    --target-platform "$TARGET_PLATFORM" \
                    --module-version "$JAVA_VER" \
                    ../wasmjdk_build/jmod/$i.jmod
            fi
        fi
    fi
done

# END


rm ../wasmjdk_build/jmod/java.base.jmod

jmod create \
    --class-path "build/emscripten/jdk/modules/java.base" \
    --target-platform "$TARGET_PLATFORM" \
    --module-version "$JAVA_VER" \
    "../wasmjdk_build/jmod/java.base.jmod"

rm -rf "$TEMP_DIR"
mkdir -p "$TEMP_DIR"

for jmod_file in "$JMOD_DIR"/*.jmod; do
    filename=$(basename "$jmod_file")
    echo "Re-stamping $filename to $TARGET_PLATFORM..."

    jmod extract --dir "$TEMP_DIR/extract" "$jmod_file"

    jmod create --class-path "$TEMP_DIR/extract/classes" \
                --target-platform "$TARGET_PLATFORM" \
                "$TEMP_DIR/$filename"

    rm -rf "$TEMP_DIR/extract"
done

mv "$TEMP_DIR"/*.jmod "$JMOD_DIR/"
rm -rf "$TEMP_DIR"

echo "All modules unified to $TARGET_PLATFORM."

echo "Updating hashes..."
jmod hash --module-path ../wasmjdk_build/jmod/ --hash-modules ".*" ../wasmjdk_build/jmod/java.base.jmod

echo "Compiling .jmods into one final image..."

jlink --module-path ../wasmjdk_build/jmod/ \
--add-modules "$1" \
--output ../wasmjdk_build/runtime/ \
--endian little \
--disable-plugin system-modules