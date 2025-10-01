# Changelog

All notable changes to Garmin FIT Utilities will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-09-26

### Added
- **Command-line utilities suite**:
  - `garmin-edit`: File analysis and editing utility with hierarchical commands
  - `garmin-rename-files`: Batch rename FIT files using File ID timestamps
  - `garmin-points-visited`: Check if activity tracks visited specific coordinates
  
- **GUI Application (`garmin-disconnect`)**:
  - Comprehensive sports activity manager with tabbed interface
  - Activities table with real-time FIT file parsing and sorting
  - Activity details panel with comprehensive file information display
  - File tree panel with dual view modes (Folder/Time hierarchy)
  - Product Editor tab for changing device product IDs
  - Cross-platform file manager integration
  - Context menus for "Open Folder" and "Open Containing Folder"
  - Drag & drop functionality (COPY by default, MOVE with SHIFT key)
  - Settings persistence with cross-platform config storage
  - Window size/position memory and directory restoration
  
- **Technical Features**:
  - Custom binary FIT parser (10x+ faster than Garmin SDK)
  - Cross-platform compatibility (Windows, Linux, macOS)
  - wxWidgets GUI framework for native look and feel
  - Interface-based architecture with `IFileOperations` for code reuse
  - Comprehensive error handling and user feedback
  - Automatic file manager detection (Nautilus, Dolphin, Nemo, Explorer, Finder)
  
- **Build & Packaging**:
  - CMake build system with configurable components
  - Debian package creation scripts (full and quick versions)
  - Desktop integration with .desktop file and application icon
  - Man pages and comprehensive documentation
  - Dependency management and verification

### Technical Details
- **Languages**: C++20 with modern standard library usage
- **Dependencies**: pugixml, wxWidgets, Garmin FIT SDK
- **Architecture**: Modular design with binary-mapper/scanner framework
- **Performance**: Direct byte manipulation for fast file processing
- **UI Framework**: wxWidgets for cross-platform native GUI

### Known Limitations
- Requires Garmin FIT SDK to be placed at `../FitSDKRelease_21.171.00/`
- Some advanced FIT file features may not be fully supported
- Testing primarily done on Linux systems

---

## Version Format
- **MAJOR.MINOR.PATCH** following Semantic Versioning
- **MAJOR**: Incompatible API changes or major feature overhauls
- **MINOR**: New functionality in a backwards compatible manner  
- **PATCH**: Backwards compatible bug fixes and minor improvements