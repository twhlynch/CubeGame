#!/usr/bin/env bash

adb uninstall com.index.cubegame
adb install Builds/android_oculus/app/release/app-release.apk

adb shell am start -n com.index.cubegame/android.app.NativeActivity
