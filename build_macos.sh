xcodebuild \
	-project Builds/macos_independent/ARTest.xcodeproj \
	-scheme ARTest \
	-configuration Debug \
	build | xcpretty

open ./Builds/macos_independent/build/ARTest/Debug/ARTest.app

read
