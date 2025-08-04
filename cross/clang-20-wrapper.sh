#!/usr/bin/env bash

set -eu

#!/bin/bash

CLANG="clang-20"
WASM_LD="wasm-ld-20"

# Capture all args
ARGS=("$@")
NEW_ARGS=()
USE_WASM_LD="false"
OUTPUT_FILE=""
IS_LINKING="false"
NEXT_IS_OUTPUT="false"

for arg in "${ARGS[@]}"; do
    case "$arg" in
    -fuse-ld=wasm-ld*)
        USE_WASM_LD="true"
        ;;
    -c)
        IS_LINKING="false"
        NEW_ARGS+=("$arg")
        ;;
    -o)
        NEW_ARGS+=("$arg")
        NEXT_IS_OUTPUT="true"
        ;;
    *.c | *.cpp | *.o)
        IS_LINKING="true"
        NEW_ARGS+=("$arg")
        ;;
    *)
        if [[ "$NEXT_IS_OUTPUT" == true ]]; then
            OUTPUT_FILE="$arg"
            NEXT_IS_OUTPUT=false
        fi
        NEW_ARGS+=("$arg")
        ;;
    esac
done

if [[ "$USE_WASM_LD" == true && "$IS_LINKING" == true ]]; then
    # Extract object files and flags meant for the linker
    LINKER_ARGS=()
    for arg in "${NEW_ARGS[@]}"; do
        # Skip compiler-only flags
        case "$arg" in
        -target=* | -I* | -D* | -c | -f* | -Wall | -Wextra | -std=*)
            continue
            ;;
        *.c | *.cpp)
            continue
            ;;
        -Wl,--as-needed | -Wl,--no-undefined | -Wl,--start-group | -Wl,--end-group | -nostdlib)
            continue
            ;;
        -Wl,*)
            IFS=',' read -ra parts <<<"${arg#-Wl,}"
            for part in "${parts[@]}"; do
                LINKER_ARGS+=("$part")
            done
            ;;
        *)
            LINKER_ARGS+=("$arg")
            ;;
        esac
    done
    exec "$WASM_LD" "${LINKER_ARGS[@]}"
else
    exec "$CLANG" "${NEW_ARGS[@]}"
fi
