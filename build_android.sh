cd Builds/android_oculus
./gradlew assembleDebug installDebug

adb shell am start -n com.index.artest/android.app.NativeActivity

read
adb logcat --pid=$(adb shell pidof -s com.index.artest) | grep "$1"
