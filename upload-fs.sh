#!/bin/bash

echo "========================================"
echo "  Building and Uploading Filesystem"
echo "========================================"
echo ""

echo "[1/2] Building filesystem image..."
pio run --target buildfs
if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Build filesystem failed!"
    exit 1
fi

echo ""
echo "[2/2] Uploading filesystem to ESP32..."
pio run --target uploadfs
if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Upload filesystem failed!"
    exit 1
fi

echo ""
echo "========================================"
echo "  SUCCESS! Filesystem uploaded!"
echo "========================================"

