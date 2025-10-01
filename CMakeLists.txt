cmake_minimum_required(VERSION 3.18)
project(garmin-fit-utilities CXX)

set(GARMIN_SDK_DIR_NAME "FitSDKRelease_21.171.00")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add_compile_options(-fsycl)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# set(CMAKE_SKIP_RPATH TRUE)

option(OPT_BUILD_EDITOR "Build the file analysis and editing utility" ON)
option(OPT_BUILD_RENAME_FILES "Build the file renaming utility" ON)
option(OPT_BUILD_POINTS_VISITED "Check if the point was visited by activities tracks" ON)
option(OPT_BUILD_GUI "Build the GUI sports manager application" ON)

find_package(pugixml REQUIRED)

if(OPT_BUILD_GUI)
    find_package(wxWidgets REQUIRED COMPONENTS core base)
    if(wxWidgets_FOUND)
        include(${wxWidgets_USE_FILE})
    else()
        message(WARNING "wxWidgets not found. GUI application will not be built.")
        set(OPT_BUILD_GUI OFF)
    endif()
endif()

get_filename_component(GARMIN_SDK_CPP "../${GARMIN_SDK_DIR_NAME}/cpp" ABSOLUTE "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB GARMIN_FIT_SOURCES "${GARMIN_SDK_CPP}/*.cpp")
add_library(garmin-sdk-cpp SHARED ${GARMIN_FIT_SOURCES})
set_target_properties(garmin-sdk-cpp PROPERTIES POSITION_INDEPENDENT_CODE ON)

include_directories(${GARMIN_SDK_CPP})

add_subdirectory("src")
