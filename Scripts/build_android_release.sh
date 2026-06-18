#!/usr/bin/env bash

BUILD_DIR="Builds/android_oculus"
SECRETS_FILE="Other/build-secrets.json"
KEYSTORE_FILE="keystore"

STORE_PASSWORD=$(jq -r '.["keystore-password"]' "$SECRETS_FILE")
KEY_PASSWORD=$(jq -r '.["keystore-key-password"]' "$SECRETS_FILE")

function err() {
	echo "$1" >&2
	exit 1
}

[ -f "$KEYSTORE_FILE" ] || err "keystore not found"
[ -z "$KEY_PASSWORD" ] && err "keystore-key-password not set in $SECRETS_FILE"
[ -z "$STORE_PASSWORD" ] && err "keystore-password not set in $SECRETS_FILE"

# build
cd "$BUILD_DIR"
./gradlew assembleRelease
cd ../../

OUTPUT="$BUILD_DIR/app/release/app-release.apk"
mkdir -p "$BUILD_DIR/app/release/"

# sign
apksigner sign \
	--ks "$KEYSTORE_FILE" \
	--ks-key-alias key0 \
	--ks-pass "pass:$STORE_PASSWORD" \
	--key-pass "pass:$KEY_PASSWORD" \
	--out "$OUTPUT" \
	"$BUILD_DIR/app/build/outputs/apk/release/app-release-unsigned.apk"

apksigner verify "$OUTPUT"

echo "Built to $OUTPUT"
