#!/bin/bash
echo "=== Running Blue Cube Demo Test ==="
echo "Starting at $(date)"
echo ""

# Run the demo for 3 seconds and capture output
timeout 3s ./build_linux/BlueCube_Demo 2>&1 | grep -E "(BlueCube|triangle|artifact|error|warning|Exception)" | tail -30

echo ""
echo "Test completed at $(date)"
