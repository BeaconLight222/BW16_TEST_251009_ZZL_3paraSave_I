#!/bin/bash

# Configuration
SOURCE_BIN="C:/Users/calla/AppData/Local/Arduino15/packages/realtek/tools/ameba_d_tools/1.1.3/km0_km4_image2.bin"
DEST_DIR="C:/Users/calla/work/beacon/Flasher_v1.0.0 Beacon_Aviton_v2_4_1"
DEST_FILE="Beacon_Aviton_v2_4_1.bin"

echo "Copying binary to Flasher directory..."
cp "${SOURCE_BIN}" "${DEST_DIR}/${DEST_FILE}"

if [ $? -eq 0 ]; then
    echo "Successfully copied and renamed to: ${DEST_DIR}/${DEST_FILE}"
else
    echo "ERROR: Failed to copy file."
    exit 1
fi
