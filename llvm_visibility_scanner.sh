#!/bin/sh

TARGET_WASM=$1

echo "Scanning $TARGET_WASM for visibility mismatches..."
echo "------------------------------------------------"

llvm-nm "$TARGET_WASM" | grep "GOT.func.internal" | sed 's/.*internal.//' | while read -r symbol; do
    match=$(llvm-nm "$TARGET_WASM" | grep -w "$symbol" | grep " t ")
    
    if [ ! -z "$match" ]; then
        echo "[!] MISMATCH FOUND: $symbol"
        echo "    Referenced externally but marked as local (t) at: $match"
        echo ""
    fi
done

echo "Scan complete."