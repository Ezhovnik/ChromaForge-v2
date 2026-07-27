#!/bin/bash

# This script fixes dylib paths for macOS .app bundle

EXECUTABLE_NAME="${1:-ChromaForge}"
SCHEME="${2:-Release}"
BUILD_DIR="${3:-build}"

OUTPUT_DIR="$BUILD_DIR/$SCHEME"
mkdir -p "$OUTPUT_DIR/libs"

echo "Fixing dylibs for $EXECUTABLE_NAME in $OUTPUT_DIR"

# Find all non-system dylibs the executable depends on
dylibs=$(otool -L "$OUTPUT_DIR/$EXECUTABLE_NAME" | grep -v "/usr/lib" | grep -v "/System" | grep -v "$EXECUTABLE_NAME" | awk '{print $1}')

for dylib in $dylibs; do
    if [ -f "$dylib" ]; then
        cp -n "$dylib" "$OUTPUT_DIR/libs/"
        libname=$(basename "$dylib")
        install_name_tool -change "$dylib" "@executable_path/libs/$libname" "$OUTPUT_DIR/$EXECUTABLE_NAME"
        echo "  Fixed: $dylib -> @executable_path/libs/$libname"
    fi
done
