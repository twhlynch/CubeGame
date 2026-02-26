xcodebuild \
	-project Builds/macos_independent/AR-Test.xcodeproj \
	-scheme AR-Test \
	-configuration Debug \
	build | xcpretty

open ./Builds/macos_independent/build/AR-Test/Debug/AR-Test.app

read
