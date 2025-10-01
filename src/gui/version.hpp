#pragma once

// Garmin FIT Utilities Version Information
// This file is automatically updated by the build system

#define GARMIN_FIT_UTILITIES_VERSION_MAJOR 1
#define GARMIN_FIT_UTILITIES_VERSION_MINOR 0
#define GARMIN_FIT_UTILITIES_VERSION_PATCH 0

#define GARMIN_FIT_UTILITIES_VERSION_STRING "1.0.0"
#define GARMIN_FIT_UTILITIES_BUILD_DATE __DATE__
#define GARMIN_FIT_UTILITIES_BUILD_TIME __TIME__

// Convenient version checking macros
#define GARMIN_FIT_UTILITIES_VERSION_CHECK(major, minor, patch) \
    ((GARMIN_FIT_UTILITIES_VERSION_MAJOR > (major)) || \
     (GARMIN_FIT_UTILITIES_VERSION_MAJOR == (major) && GARMIN_FIT_UTILITIES_VERSION_MINOR > (minor)) || \
     (GARMIN_FIT_UTILITIES_VERSION_MAJOR == (major) && GARMIN_FIT_UTILITIES_VERSION_MINOR == (minor) && GARMIN_FIT_UTILITIES_VERSION_PATCH >= (patch)))

// Application metadata
#define GARMIN_FIT_UTILITIES_APP_NAME "Garmin Disconnect"
#define GARMIN_FIT_UTILITIES_DESCRIPTION "Comprehensive GUI application for managing Garmin FIT files"
#define GARMIN_FIT_UTILITIES_COPYRIGHT "© 2025 Darau, Blė"
#define GARMIN_FIT_UTILITIES_WEBSITE "https://github.com/darauble/garmin-fit-utilities"