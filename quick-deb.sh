#!/bin/bash

# Quick Debian Package Builder for Garmin FIT Utilities
# Minimal version for development and testing

set -e

# Cleanup function
cleanup() {
    [ -d "${TEMP_DIR:-}" ] && rm -rf "${TEMP_DIR}" || true
}
trap cleanup EXIT

# Configuration
PACKAGE_NAME="garmin-fit-utilities"
PACKAGE_VERSION="1.0.0"
PACKAGE_ARCHITECTURE="amd64"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}[INFO]${NC} Building Garmin FIT Utilities .deb package..."

# Get project root
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
TEMP_DIR="${PROJECT_ROOT}/temp-deb-quick"
DEB_ROOT="${TEMP_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}_${PACKAGE_ARCHITECTURE}"

# Clean and build
echo -e "${BLUE}[INFO]${NC} Cleaning and building project..."
rm -rf "${BUILD_DIR}" "${TEMP_DIR}"
mkdir -p "${BUILD_DIR}"

cd "${BUILD_DIR}"
cmake .. -DCMAKE_BUILD_TYPE=Release -DOPT_BUILD_GUI=ON
make -j$(nproc)

echo -e "${GREEN}[SUCCESS]${NC} Build completed"

# Create package structure
echo -e "${BLUE}[INFO]${NC} Creating package structure..."
mkdir -p "${DEB_ROOT}/DEBIAN"
mkdir -p "${DEB_ROOT}/usr/bin"
mkdir -p "${DEB_ROOT}/usr/lib"
mkdir -p "${DEB_ROOT}/usr/share/applications"
mkdir -p "${DEB_ROOT}/usr/share/pixmaps"

# Copy files
cp "${BUILD_DIR}/garmin-"* "${DEB_ROOT}/usr/bin/" 2>/dev/null || true
cp "${BUILD_DIR}/libgarmin-sdk-cpp.so" "${DEB_ROOT}/usr/lib/"
rm -f "${DEB_ROOT}/usr/bin/libgarmin-sdk-cpp.so" 2>/dev/null || true

# Desktop integration
sed 's|Exec=.*|Exec=/usr/bin/garmin-disconnect|g; s|Icon=.*|Icon=/usr/share/pixmaps/garmin-disconnect.png|g' \
    "${PROJECT_ROOT}/garmin-disconnect.desktop" > "${DEB_ROOT}/usr/share/applications/garmin-disconnect.desktop"

cp "${PROJECT_ROOT}/src/gui/icon/garmin-disconnect.png" "${DEB_ROOT}/usr/share/pixmaps/"

# Copy data files
if [ -d "${PROJECT_ROOT}/data" ]; then
    mkdir -p "${DEB_ROOT}/usr/share/garmin-disconnect/data"
    cp -r "${PROJECT_ROOT}/data/"* "${DEB_ROOT}/usr/share/garmin-disconnect/data/"
fi

# Set permissions
chmod 755 "${DEB_ROOT}/usr/bin/"*
chmod 644 "${DEB_ROOT}/usr/lib/"*

# Calculate size
INSTALLED_SIZE=$(du -sk "${DEB_ROOT}" | cut -f1)

# Create control file
cat > "${DEB_ROOT}/DEBIAN/control" << EOF
Package: ${PACKAGE_NAME}
Version: ${PACKAGE_VERSION}
Architecture: ${PACKAGE_ARCHITECTURE}
Maintainer: Claude Code <noreply@anthropic.com>
Installed-Size: ${INSTALLED_SIZE}
Depends: libc6, libstdc++6, libpugixml1v5, libwxbase3.2-1, libwxgtk3.2-1
Section: utils
Priority: optional
Description: Garmin FIT file utilities suite
 Command-line tools and GUI application for managing Garmin FIT files.
 Includes file editing, analysis, coordinate tracking, and batch operations.
EOF

# Build package
echo -e "${BLUE}[INFO]${NC} Building .deb package..."
cd "${TEMP_DIR}"
dpkg-deb --root-owner-group --build "${PACKAGE_NAME}_${PACKAGE_VERSION}_${PACKAGE_ARCHITECTURE}"

# Move to project root
DEB_FILE="${PACKAGE_NAME}_${PACKAGE_VERSION}_${PACKAGE_ARCHITECTURE}.deb"
mv "${DEB_FILE}" "${PROJECT_ROOT}/"

# Cleanup handled by trap

echo -e "${GREEN}[SUCCESS]${NC} Package created: ${PROJECT_ROOT}/${DEB_FILE}"
echo -e "${BLUE}[INFO]${NC} Size: $(du -h "${PROJECT_ROOT}/${DEB_FILE}" | cut -f1)"
echo
echo "To install: sudo dpkg -i ${DEB_FILE}"
echo "To remove:  sudo apt remove ${PACKAGE_NAME}"