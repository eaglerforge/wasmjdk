cd docs

WASMOPT=" -O4 --converge --const-hoisting --strip-debug --enable-threads --enable-simd --enable-fp16 --disable-gc --disable-memory64 --enable-bulk-memory --enable-nontrapping-float-to-int"
WASMOPT=$WASMOPT" --debug"
WASMOPT=$WASMOPT=" -s 0 --optimize-level 9 --vacuum --type-ssa"
WASMOPT=$WASMOPT=" --type-refining --type-merging --tuple-optimization --trap-mode-clamp"
WASMOPT=$WASMOPT=" --ssa --simplify-locals --simplify-globals-optimizing --signature-pruning"
WASMOPT=$WASMOPT=" --signature-refining --rse --roundtrip --flatten --rereloop --reorder-locals"
WASMOPT=$WASMOPT=" --reorder-globals --reorder-functions --remove-unused-types --remove-unused-module-elements"
WASMOPT=$WASMOPT=" --remove-unused-names --remove-unused-brs --precompute-propagate --post-emscripten"
WASMOPT=$WASMOPT=" --optimize-instructions --optimize-casts --low-memory-unused --once-reduction"
WASMOPT=$WASMOPT=" --merge-locals --merge-similar-functions --merge-j2cl-itables --merge-blocks"
WASMOPT=$WASMOPT=" --local-subtyping --licm --inlining-optimizing --heap-store-optimization"
WASMOPT=$WASMOPT=" --gufa --gufa-optimizing --gto --gsi --global-refining --directize --dce"
WASMOPT=$WASMOPT=" --dae-optimizing --const-hoisting --code-folding --code-pushing"
WASMOPT=$WASMOPT=" --cfp --abstract-type-refining --monomorphize --pass-arg=monomorphize-min-benefit@30"
WASMOPT=$WASMOPT=" --log-execution --simplify-locals --optimize-instructions --merge-locals --simplify-locals --coalesce-locals --merge-locals --vacuum"

wasm-opt jvm.wasm -o jvm_hypr.wasm $WASMOPT && rm jvm.wasm
mv jvm_hypr.wasm jvm.wasm
echo "+hyprpass" >> rt/rt.info