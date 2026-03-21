cd libffi
chmod +x autogen.sh

if [ ! -f configure ]; then
    ./autogen.sh
fi

emconfigure ./configure \
    --host=wasm32-unknown-emscripten \
    --prefix=$(pwd)/wasm_build \
    --enable-static \
    --disable-shared \
    --disable-multi-os-directory \
    CFLAGS="-O3 -fPIC -fno-jump-tables -fno-direct-access-external-data -fvisibility=default -sMAIN_MODULE=1 -sRELOCATABLE=1 -sWASM_BIGINT=1"

emmake make
emmake make install