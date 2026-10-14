#!/bin/bash

set -e

total_time=0

time_command() {
    local name="$1"
    shift

    echo "=== $name ==="

    local start=$(date +%s.%N)

    "$@"

    local status=$?

    local end=$(date +%s.%N)
    local elapsed=$(echo "$end - $start" | bc)

    echo "$name took ${elapsed}s"
    echo

    total_time=$(echo "$total_time + $elapsed" | bc)

    return $status
}

time_command "Compiling shaders" \
    $VULKAN_SDK/bin/slangc shaders/shader.slang \
    -target spirv \
    -profile spirv_1_4 \
    -emit-spirv-directly \
    -fvk-use-entrypoint-name \
    -entry vertMain \
    -entry fragMain \
    -o shaders/slang.spv

time_command "Configuring CMake" \
    cmake -B build

time_command "Building C++" \
    cmake --build build

echo "=============================="
echo "Total build time: ${total_time}s"
echo "=============================="

./build/bin/BeLightVulkan
