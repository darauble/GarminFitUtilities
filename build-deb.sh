#!/bin/bash

# Garmin FIT Utilities - Debian Package Builder
# This script builds the project and creates a .deb package

set -e  # Exit on any error

# Cleanup function
cleanup() {
    if [ -d "${TEMP_DIR}" ]; then
        log_info "Cleaning up temporary files..."
        rm -rf "${TEMP_DIR}"
    fi
}

# Set trap to cleanup on exit
trap cleanup EXIT

# Configuration
PACKAGE_NAME="garmin-fit-utilities"
PACKAGE_VERSION="1.0.0"
PACKAGE_REVISION="1"
PACKAGE_ARCHITECTURE="amd64"
PACKAGE_MAINTAINER="Darau, Blė <darau.ble@gmail.com>"
PACKAGE_DESCRIPTION="Comprehensive suite for managing and analyzing Garmin FIT files"
PACKAGE_LONG_DESCRIPTION="Garmin FIT Utilities provides command-line tools and a GUI application for managing, analyzing, and modifying Garmin FIT files. Includes tools for file editing, activity analysis, coordinate tracking, and batch operations."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Get project root directory
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
TEMP_DIR="${PROJECT_ROOT}/temp-deb"
DEB_ROOT="${TEMP_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_REVISION}_${PACKAGE_ARCHITECTURE}"

log_info "Starting Garmin FIT Utilities .deb package build"
log_info "Project root: ${PROJECT_ROOT}"

# Check dependencies
log_info "Checking build dependencies..."

check_dependency() {
    if ! command -v "$1" &> /dev/null; then
        log_error "$1 is required but not installed"
        exit 1
    fi
}

check_dependency "cmake"
check_dependency "make"
check_dependency "g++"
check_dependency "dpkg-deb"

# Check for required libraries
if ! pkg-config --exists pugixml; then
    log_error "pugixml development package is required (libpugixml-dev)"
    exit 1
fi

if ! pkg-config --exists gtk+-3.0; then
    log_warning "GTK+3 development package not found. GUI might not build properly."
fi

# Check for mapnik (required for GUI map rendering)
if ! command -v mapnik-config &> /dev/null; then
    log_error "mapnik-config not found. Please install libmapnik-dev"
    exit 1
fi

# Check for ogr2ogr (recommended for PBF to Spatialite conversion)
if ! command -v ogr2ogr &> /dev/null; then
    log_warning "ogr2ogr not found. Install gdal-bin for PBF to Spatialite conversion support"
fi

log_success "All required dependencies found"

# Clean previous builds
log_info "Cleaning previous builds..."
rm -rf "${BUILD_DIR}"
rm -rf "${TEMP_DIR}"

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

log_info "Configuring CMake build..."

# Configure CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DOPT_BUILD_GUI=ON \
    -DOPT_BUILD_EDITOR=ON \
    -DOPT_BUILD_RENAME_FILES=ON \
    -DOPT_BUILD_POINTS_VISITED=ON

log_info "Building project..."

# Build the project
make -j$(nproc)

log_success "Build completed successfully"

# Verify built executables
log_info "Verifying built executables..."

EXECUTABLES=(
    "garmin-edit"
    "garmin-rename-files" 
    "garmin-points-visited"
    "garmin-disconnect"
    "libgarmin-sdk-cpp.so"
)

for exe in "${EXECUTABLES[@]}"; do
    if [ ! -f "${BUILD_DIR}/${exe}" ]; then
        log_error "Expected executable ${exe} not found!"
        exit 1
    fi
    log_success "Found: ${exe}"
done

# Create temporary debian package structure
log_info "Creating Debian package structure..."

mkdir -p "${DEB_ROOT}/DEBIAN"
mkdir -p "${DEB_ROOT}/usr/bin"
mkdir -p "${DEB_ROOT}/usr/lib"
mkdir -p "${DEB_ROOT}/usr/share/applications"
mkdir -p "${DEB_ROOT}/usr/share/pixmaps"
mkdir -p "${DEB_ROOT}/usr/share/doc/${PACKAGE_NAME}"
mkdir -p "${DEB_ROOT}/usr/share/man/man1"

# Copy executables
log_info "Installing executables..."
cp "${BUILD_DIR}/garmin-edit" "${DEB_ROOT}/usr/bin/"
cp "${BUILD_DIR}/garmin-rename-files" "${DEB_ROOT}/usr/bin/"
cp "${BUILD_DIR}/garmin-points-visited" "${DEB_ROOT}/usr/bin/"
cp "${BUILD_DIR}/garmin-disconnect" "${DEB_ROOT}/usr/bin/"

# Copy shared library
cp "${BUILD_DIR}/libgarmin-sdk-cpp.so" "${DEB_ROOT}/usr/lib/"

# Copy desktop file and icon
log_info "Installing desktop integration..."
sed "s|Exec=.*|Exec=/usr/bin/garmin-disconnect|g; s|Icon=.*|Icon=/usr/share/pixmaps/garmin-disconnect.png|g" \
    "${PROJECT_ROOT}/garmin-disconnect.desktop" > "${DEB_ROOT}/usr/share/applications/garmin-disconnect.desktop"

cp "${PROJECT_ROOT}/src/gui/icon/garmin-disconnect.png" "${DEB_ROOT}/usr/share/pixmaps/"

# Copy data files
log_info "Installing data files..."
if [ -d "${PROJECT_ROOT}/data" ]; then
    mkdir -p "${DEB_ROOT}/usr/share/garmin-disconnect/data"
    cp -r "${PROJECT_ROOT}/data/"* "${DEB_ROOT}/usr/share/garmin-disconnect/data/"
    log_success "Data files installed"
else
    log_warning "Data directory not found at ${PROJECT_ROOT}/data"
fi

# Create documentation
log_info "Installing documentation..."
cp "${PROJECT_ROOT}/README.md" "${DEB_ROOT}/usr/share/doc/${PACKAGE_NAME}/"
cp "${PROJECT_ROOT}/CHANGELOG.md" "${DEB_ROOT}/usr/share/doc/${PACKAGE_NAME}/"

# Create copyright file
cat > "${DEB_ROOT}/usr/share/doc/${PACKAGE_NAME}/copyright" << EOF
Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: ${PACKAGE_NAME}
Source: https://github.com/anthropics/garmin-fit-utilities

Files: *
Copyright: 2025 Darau, Blė
License: MIT
EOF

# Create Debian changelog from CHANGELOG.md
log_info "Creating Debian changelog from CHANGELOG.md..."
if [ -f "${PROJECT_ROOT}/CHANGELOG.md" ]; then
    # Create a Debian-format changelog from the markdown changelog
    cat > "${DEB_ROOT}/usr/share/doc/${PACKAGE_NAME}/changelog.Debian" << EOF
${PACKAGE_NAME} (${PACKAGE_VERSION}-${PACKAGE_REVISION}) unstable; urgency=low

  * Release v${PACKAGE_VERSION}
  * See CHANGELOG.md for detailed changes

 -- ${PACKAGE_MAINTAINER}  $(date -R)
EOF
    gzip "${DEB_ROOT}/usr/share/doc/${PACKAGE_NAME}/changelog.Debian"
else
    log_warning "CHANGELOG.md not found, creating basic changelog"
    cat > "${DEB_ROOT}/usr/share/doc/${PACKAGE_NAME}/changelog.Debian" << EOF
${PACKAGE_NAME} (${PACKAGE_VERSION}-${PACKAGE_REVISION}) unstable; urgency=low

  * Package build v${PACKAGE_VERSION}

 -- ${PACKAGE_MAINTAINER}  $(date -R)
EOF
    gzip "${DEB_ROOT}/usr/share/doc/${PACKAGE_NAME}/changelog.Debian"
fi

# Create man pages (basic ones)
log_info "Creating man pages..."

create_man_page() {
    local cmd="$1"
    local desc="$2"
    cat > "${DEB_ROOT}/usr/share/man/man1/${cmd}.1" << EOF
.TH ${cmd^^} 1 "$(date +%B\ %Y)" "${PACKAGE_VERSION}" "User Commands"
.SH NAME
${cmd} - ${desc}
.SH SYNOPSIS
.B ${cmd}
[OPTIONS]
.SH DESCRIPTION
${desc} part of the Garmin FIT Utilities suite.
.SH OPTIONS
Run ${cmd} with no arguments to see available options.
.SH SEE ALSO
Full documentation available in /usr/share/doc/${PACKAGE_NAME}/
.SH AUTHOR
${PACKAGE_MAINTAINER}
EOF
    gzip "${DEB_ROOT}/usr/share/man/man1/${cmd}.1"
}

create_man_page "garmin-edit" "Garmin FIT file analysis and editing utility"
create_man_page "garmin-rename-files" "Batch rename FIT files using timestamps"  
create_man_page "garmin-points-visited" "Check if activities visited specific coordinates"
create_man_page "garmin-disconnect" "GUI application for comprehensive FIT file management"

# Set proper permissions
log_info "Setting file permissions..."
chmod 755 "${DEB_ROOT}/usr/bin/"*
chmod 644 "${DEB_ROOT}/usr/lib/"*
chmod 644 "${DEB_ROOT}/usr/share/applications/"*
chmod 644 "${DEB_ROOT}/usr/share/pixmaps/"*

# Fix documentation permissions properly
find "${DEB_ROOT}/usr/share/doc" -type d -exec chmod 755 {} \;
find "${DEB_ROOT}/usr/share/doc" -type f -exec chmod 644 {} \;

# Fix man page permissions properly  
find "${DEB_ROOT}/usr/share/man" -type d -exec chmod 755 {} \;
find "${DEB_ROOT}/usr/share/man" -type f -exec chmod 644 {} \;

# Create control file
log_info "Creating package control file..."

# Calculate installed size
INSTALLED_SIZE=$(du -sk "${DEB_ROOT}" | cut -f1)

cat > "${DEB_ROOT}/DEBIAN/control" << EOF
Package: ${PACKAGE_NAME}
Version: ${PACKAGE_VERSION}-${PACKAGE_REVISION}
Architecture: ${PACKAGE_ARCHITECTURE}
Maintainer: ${PACKAGE_MAINTAINER}
Installed-Size: ${INSTALLED_SIZE}
Depends: libc6 (>= 2.34), libgcc-s1 (>= 3.0), libstdc++6 (>= 11), libpugixml1v5, libwxbase3.2-1, libwxgtk3.2-1, libmapnik3.1 | libmapnik (>= 3.0)
Recommends: gdal-bin, nautilus | dolphin | nemo | pcmanfm
Suggests: osmium-tool
Section: utils
Priority: optional
Homepage: https://github.com/darauble/garmin-fit-utilities
Description: ${PACKAGE_DESCRIPTION}
 ${PACKAGE_LONG_DESCRIPTION}
 .
 Included utilities:
  - garmin-edit: File analysis and editing
  - garmin-rename-files: Batch file renaming
  - garmin-points-visited: Coordinate tracking
  - garmin-disconnect: GUI application with all features
 .
 Features:
  - Cross-platform file manager integration
  - Drag & drop functionality
  - Context menus for file operations
  - Product ID editing
  - Activity analysis and visualization
  - Points visited tracking with GPS coordinates
EOF

# Create postinst script for desktop database update
cat > "${DEB_ROOT}/DEBIAN/postinst" << 'EOF'
#!/bin/bash
set -e

if [ "$1" = "configure" ]; then
    # Update desktop database
    if command -v update-desktop-database >/dev/null 2>&1; then
        update-desktop-database -q /usr/share/applications || true
    fi
    
    # Update MIME database
    if command -v update-mime-database >/dev/null 2>&1; then
        update-mime-database /usr/share/mime || true
    fi
    
    # Refresh icon cache
    if command -v gtk-update-icon-cache >/dev/null 2>&1; then
        gtk-update-icon-cache -q /usr/share/pixmaps || true
    fi
    
    # Update shared library cache
    ldconfig || true
fi

#DEBHELPER#
EOF

# Create postrm script for cleanup
cat > "${DEB_ROOT}/DEBIAN/postrm" << 'EOF'
#!/bin/bash
set -e

if [ "$1" = "remove" ] || [ "$1" = "purge" ]; then
    # Update desktop database
    if command -v update-desktop-database >/dev/null 2>&1; then
        update-desktop-database -q /usr/share/applications || true
    fi
    
    # Update MIME database  
    if command -v update-mime-database >/dev/null 2>&1; then
        update-mime-database /usr/share/mime || true
    fi
    
    # Update shared library cache
    ldconfig || true
fi

#DEBHELPER#
EOF

chmod 755 "${DEB_ROOT}/DEBIAN/postinst"
chmod 755 "${DEB_ROOT}/DEBIAN/postrm"

# Build the .deb package
log_info "Building .deb package..."

cd "${TEMP_DIR}"
dpkg-deb --root-owner-group --build "${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_REVISION}_${PACKAGE_ARCHITECTURE}"

# Move the package to project root
DEB_FILE="${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_REVISION}_${PACKAGE_ARCHITECTURE}.deb"
mv "${DEB_FILE}" "${PROJECT_ROOT}/"

# Verify the package
log_info "Verifying .deb package..."
dpkg-deb --info "${PROJECT_ROOT}/${DEB_FILE}"
dpkg-deb --contents "${PROJECT_ROOT}/${DEB_FILE}" | head -20

# Check package with lintian if available
if command -v lintian &> /dev/null; then
    log_info "Running lintian checks..."
    lintian "${PROJECT_ROOT}/${DEB_FILE}" || log_warning "Lintian found some issues (non-fatal)"
fi

# Temporary files will be cleaned up by trap

# Final success message
log_success "Package created successfully: ${PROJECT_ROOT}/${DEB_FILE}"
log_info "Package size: $(du -h "${PROJECT_ROOT}/${DEB_FILE}" | cut -f1)"

echo
echo -e "${GREEN}=== INSTALLATION INSTRUCTIONS ===${NC}"
echo "To install the package:"
echo "  sudo dpkg -i ${DEB_FILE}"
echo
echo "To install dependencies if needed:"
echo "  sudo apt-get install -f"
echo  
echo "To remove the package:"
echo "  sudo apt-get remove ${PACKAGE_NAME}"
echo
echo -e "${GREEN}=== PACKAGE CONTENTS ===${NC}"
echo "Executables:"
echo "  - /usr/bin/garmin-edit"
echo "  - /usr/bin/garmin-rename-files"
echo "  - /usr/bin/garmin-points-visited" 
echo "  - /usr/bin/garmin-disconnect (GUI)"
echo
echo "Desktop integration:"
echo "  - /usr/share/applications/garmin-disconnect.desktop"
echo "  - /usr/share/pixmaps/garmin-disconnect.png"
echo
echo -e "${BLUE}Package build completed successfully!${NC}"