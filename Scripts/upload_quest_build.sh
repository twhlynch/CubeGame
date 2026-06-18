#!/usr/bin/env bash

APK_FILE="Builds/android_oculus/app/release/app-release.apk"
SECRETS_FILE="Other/build-secrets.json"

function err() {
	echo "$1" >&2
	exit 1
}

[ -f "$APK_FILE" ] || err "$APK_FILE not found"
[ -f "$SECRETS_FILE" ] || err "$SECRETS_FILE not found"

APP_ID=$(jq -r '.["oculus"].["quest"].["app-id"]' "$SECRETS_FILE")
APP_SECRET=$(jq -r '.["oculus"].["quest"].["secret"]' "$SECRETS_FILE")

[ -z "$APP_ID" ] && err "oculus.quest.app-id not set in $SECRETS_FILE"
[ -z "$APP_SECRET" ] && err "oculus.quest.secret not set in $SECRETS_FILE"

ovr-platform-util upload-quest-build \
	--app-id "$APP_ID" \
	--app-secret "$APP_SECRET" \
	--apk "$APK_FILE" \
	--channel LIVE
