cd docs
WASMOPT=" --debug --enable-threads --enable-simd --enable-fp16 --disable-gc --disable-memory64"
WASMOPT=$WASMOPT" -O4 --simplify-locals --coalesce-locals --dce --vacuum --precompute --converge --enable-bulk-memory --enable-nontrapping-float-to-int"
wasm-opt jvm.wasm -o jvm_hypr.wasm $WASMOPT && rm jvm.wasm
mv jvm_hypr.wasm jvm.wasm
echo "+hyprpass" >> rt/rt.info