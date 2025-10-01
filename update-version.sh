#!/bin/bash

# Version Update Script for Garmin FIT Utilities
# Updates version numbers in source files and documentation

set -e

if [ $# -ne 1 ]; then
    echo "Usage: $0 <version>"
    echo "Example: $0 1.0.1"
    exit 1
fi

VERSION="$1"
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

# Validate version format (simple semantic version check)
if ! echo "$VERSION" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+$'; then
    echo "Error: Version must be in format MAJOR.MINOR.PATCH (e.g., 1.0.1)"
    exit 1
fi

# Extract version components
IFS='.' read -r MAJOR MINOR PATCH <<< "$VERSION"

echo "Updating version to $VERSION..."

# Update version.hpp
echo "Updating src/gui/version.hpp..."
cat > "${PROJECT_ROOT}/src/gui/version.hpp" << EOF
#pragma once

// Garmin FIT Utilities Version Information
// This file is automatically updated by the build system

#define GARMIN_FIT_UTILITIES_VERSION_MAJOR $MAJOR
#define GARMIN_FIT_UTILITIES_VERSION_MINOR $MINOR
#define GARMIN_FIT_UTILITIES_VERSION_PATCH $PATCH

#define GARMIN_FIT_UTILITIES_VERSION_STRING "$VERSION"
#define GARMIN_FIT_UTILITIES_BUILD_DATE __DATE__
#define GARMIN_FIT_UTILITIES_BUILD_TIME __TIME__

// Convenient version checking macros
#define GARMIN_FIT_UTILITIES_VERSION_CHECK(major, minor, patch) \\
    ((GARMIN_FIT_UTILITIES_VERSION_MAJOR > (major)) || \\
     (GARMIN_FIT_UTILITIES_VERSION_MAJOR == (major) && GARMIN_FIT_UTILITIES_VERSION_MINOR > (minor)) || \\
     (GARMIN_FIT_UTILITIES_VERSION_MAJOR == (major) && GARMIN_FIT_UTILITIES_VERSION_MINOR == (minor) && GARMIN_FIT_UTILITIES_VERSION_PATCH >= (patch)))

// Application metadata
#define GARMIN_FIT_UTILITIES_APP_NAME "Garmin Disconnect"
#define GARMIN_FIT_UTILITIES_DESCRIPTION "Comprehensive GUI application for managing Garmin FIT files"
#define GARMIN_FIT_UTILITIES_COPYRIGHT "© 2025 Darau, Blė"
#define GARMIN_FIT_UTILITIES_WEBSITE "https://github.com/darauble/garmin-fit-utilities"
EOF

# Update build script version
echo "Updating build-deb.sh..."
sed -i "s/^PACKAGE_VERSION=\".*\"/PACKAGE_VERSION=\"$VERSION\"/" "${PROJECT_ROOT}/build-deb.sh"

echo "Updating quick-deb.sh..."
sed -i "s/^PACKAGE_VERSION=\".*\"/PACKAGE_VERSION=\"$VERSION\"/" "${PROJECT_ROOT}/quick-deb.sh"

echo "Version updated successfully to $VERSION"
echo ""
echo "Next steps:"
echo "1. Update CHANGELOG.md with new version information"
echo "2. Build and test the application"
echo "3. Commit changes to version control"
echo "4. Create a git tag: git tag v$VERSION"