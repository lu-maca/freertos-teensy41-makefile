#!/usr/bin/env bash

set -e

TARGET_DIR="src"

find "$TARGET_DIR" \
    \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
    -type f \
    -exec clang-format -i {} \;

echo "Formatting completed."