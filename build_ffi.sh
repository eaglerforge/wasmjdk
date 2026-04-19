cd libffi
chmod +x autogen.sh

rm Makefile

if [ ! -f configure ]; then
    ./autogen.sh
fi

emconfigure ./configure \
    --host=wasm32-unknown-emscripten \
    --prefix=$(pwd)/wasm_build \
    --enable-static \
    --disable-shared \
    --disable-multi-os-directory \
    CFLAGS="-O3 -sJSPI -fPIC -fno-jump-tables -fno-direct-access-external-data -fvisibility=default -matomics -mbulk-memory -sMAIN_MODULE=1 -sRELOCATABLE=1 -sWASM_BIGINT=1" \
    CONFIG_SHELL=$(which bash) SHELL=$(which bash)

emmake make
emmake make install