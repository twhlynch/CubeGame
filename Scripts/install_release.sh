#!/usr/bin/env bash

adb uninstall com.index.cubegame
adb install Builds/android_oculus/Releases/app-release.apk

adb shell am start -n com.index.cubegame/android.app.NativeActivity
