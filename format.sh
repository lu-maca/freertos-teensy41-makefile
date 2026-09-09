#!/usr/bin/env bash

set -e

TARGET_DIR="src"

find "$TARGET_DIR" \
    \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
    -type f \
    -print0 |
while IFS= read -r -d '' file; do
    if ! clang-format --dry-run --Werror "$file" >/dev/null 2>&1; then
        echo "Formatting: $file"
        clang-format -i "$file"
    fi
done

echo "Formatting completed."