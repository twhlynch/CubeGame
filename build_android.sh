cd Builds/android_oculus
./gradlew assembleDebug installDebug

read
adb shell am start -n com.index.artest/.ARTNativeActivity

read
adb logcat --pid=$(adb shell pidof -s com.index.artest) | grep "$1"
