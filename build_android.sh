cd Builds/android_oculus
./gradlew assembleDebug installDebug

read
adb shell am start -n com.index.ar-test/.ARTNativeActivity

read
adb logcat --pid=$(adb shell pidof -s com.index.ar-test) | grep "$1"
