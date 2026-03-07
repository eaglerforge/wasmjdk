#!/bin/sh

while [ $# -gt 0 ]; do
    case "$1" in
        -o)
            shift
            if [ -n "$1" ]; then
                touch "$1"
                echo "Shimming linker to (A-path): "$1
                exit 0
            fi
            ;;
        -o*)
            out_file="${1#-o}"
            if [ -n "$out_file" ]; then
                touch "$out_file"
                echo "Shimming linker to (B-path): "$1
                exit 0
            fi
            ;;
    esac
    shift
done

exit 0