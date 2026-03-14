xcodebuild \
	-project Builds/macos_independent/CubeGame.xcodeproj \
	-scheme CubeGame \
	-configuration Debug \
	build | xcpretty

open ./Builds/macos_independent/build/CubeGame/Debug/CubeGame.app

read
