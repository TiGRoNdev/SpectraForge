#!/bin/bash
#
# Wrapper script to run SpectraForge applications with system Vulkan loader
# instead of vcpkg version
#
# Usage: ./run_with_system_vulkan.sh <executable_name> [args...]
#

# Ensure system Vulkan libraries are prioritized
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"

# Optional: Disable validation layers for better performance (remove this line if debugging)
# export VK_INSTANCE_LAYERS=""

# Run the application
if [ $# -eq 0 ]; then
    echo "Usage: $0 <executable_name> [args...]"
    echo ""
    echo "Example:"
    echo "  $0 ./build/SpectraForge_Example_Demo"
    exit 1
fi

echo "🚀 Запуск с системным Vulkan loader..."
echo "   LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
echo ""

exec "$@"

