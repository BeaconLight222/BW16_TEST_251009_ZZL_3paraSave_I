#!/bin/bash

# Configuration
OTA_BASE_DIR="C:/Users/calla/work/beacon/OTA_TESTING"
SOURCE_BIN="C:/Users/calla/AppData/Local/Arduino15/packages/realtek/tools/ameba_d_tools/1.1.3/km0_km4_image2.bin"

# 1. Create a new folder with current time YYYY-MM-DD-HH-MM-SS
TIMESTAMP_DIR=$(date +"%Y-%m-%d_%H-%M-%S")
NEW_DIR="${OTA_BASE_DIR}/${TIMESTAMP_DIR}"

echo "Creating directory: ${NEW_DIR}"
mkdir -p "${NEW_DIR}"

# 2. Copy and rename to beacon-firmware_YYYYMMDD.bin
TIMESTAMP_FILE=$(date +"%Y%m%d")
GIT_TAG=$(git describe --tags --abbrev=0)
NEW_BIN_NAME="Beacon_Aviton_${GIT_TAG}_${TIMESTAMP_FILE}.bin"
NEW_BIN_PATH="${NEW_DIR}/${NEW_BIN_NAME}"

echo "Copying binary to ${NEW_BIN_PATH}..."
cp "${SOURCE_BIN}" "${NEW_BIN_PATH}"

# Navigate to the new directory for subsequent steps
# Note: This assumes ecdsasigner.key and signer_pubkey.pem are in the parent directory (OTA_TESTING)
cd "${NEW_DIR}" || { echo "Failed to enter ${NEW_DIR}"; exit 1; }

# 3. Sign the binary
SIG_FILE="${NEW_BIN_NAME}.sig"
echo "Signing firmware..."
openssl dgst -sha256 -sign ../ecdsasigner.key -out "${SIG_FILE}" "${NEW_BIN_NAME}"

# 4. Verify the signature
echo "Verifying signature..."
VERIFY_RESULT=$(openssl dgst -sha256 -verify ../signer_pubkey.pem -signature "${SIG_FILE}" "${NEW_BIN_NAME}" 2>&1)

if [[ "${VERIFY_RESULT}" == "Verified OK" ]]; then
    echo "Verification successful: ${VERIFY_RESULT}"
else
    echo "ERROR: Signature verification failed!"
    echo "${VERIFY_RESULT}"
    exit 1
fi

# 5. Base64 encode the signature
B64_FILE="${SIG_FILE}.b64"
echo "Encoding signature to base64..."
openssl base64 -A -in "${SIG_FILE}" -out "${B64_FILE}"

echo "------------------------------------------------"
echo "OTA preparation complete!"
echo "Folder: ${NEW_DIR}"
echo "Binary: ${NEW_BIN_NAME}"
echo "Base64 Sig: ${B64_FILE}"
