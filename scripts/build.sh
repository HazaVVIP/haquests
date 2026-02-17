#!/bin/bash
set -e

echo "Building HAQuests..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

echo "Build complete!"
echo "Binaries are in build/examples/"
