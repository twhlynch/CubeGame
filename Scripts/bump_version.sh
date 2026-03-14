#!/usr/bin/env bash

if [ $# -ne 1 ]; then
	echo "Usage: $0 major|minor|patch"
	exit 1
fi

TYPE=$1

# read version
read -r BUILD <./VERSION
read -r VERSION < <(tail -n +2 ./VERSION)

IFS='.' read -r MAJOR MINOR PATCH <<<"$VERSION"

# increment build number
BUILD=$((BUILD + 1))

# increment version
if [ "$TYPE" = "major" ]; then
	MAJOR=$((MAJOR + 1))
	MINOR=0
	PATCH=0

elif [ "$TYPE" = "minor" ]; then
	MINOR=$((MINOR + 1))
	PATCH=0

elif [ "$TYPE" = "patch" ]; then
	PATCH=$((PATCH + 1))

else
	echo "Usage: $0 major|minor|patch"
	exit 1
fi

# write new version
echo "$BUILD" >./VERSION
echo "$MAJOR.$MINOR.$PATCH" >>./VERSION

MESSAGE="$MAJOR.$MINOR.$PATCH ($BUILD)"

# commit
git add ./VERSION
git commit -m "chore: bump version to $MESSAGE"

echo "$MESSAGE"
