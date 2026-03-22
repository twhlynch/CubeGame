#!/usr/bin/env bash

# read version
read -r BUILD <./VERSION
read -r VERSION < <(tail -n +2 ./VERSION)

# previous version
last_version=$(git log --grep="^chore: bump version" --pretty=format:"%H" | tail -n +2 | head -n1)

# commits since
commits=$(git log "$last_version"..HEAD~1 --pretty=format:"- %s")

echo "**v$VERSION** ($BUILD)"
echo "$commits"
