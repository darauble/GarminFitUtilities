cmake_minimum_required(VERSION 3.18)
project(garmin-fit-utilities CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add_compile_options(-fsycl)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# set(CMAKE_SKIP_RPATH TRUE)

option(OPT_BUILD_RENAME_FILES "Build the file renaming utility" ON)
option(OPT_BUILD_CHANGE_PRODUCT "Change the Garmin product in the FIT file" OFF)
option(OPT_BUILD_POINTS_VISITED "Check if the point was visited by activities tracks" ON)

get_filename_component(GARMIN_SDK_CPP "../FitSDKRelease_21.158.00/cpp" ABSOLUTE "${CMAKE_CURRENT_SOURCE_DIR}")

set(GARMIN_FIT_SOURCES
    ${GARMIN_SDK_CPP}/fit_accumulated_field.cpp
    ${GARMIN_SDK_CPP}/fit_buffered_mesg_broadcaster.cpp
    ${GARMIN_SDK_CPP}/fit_buffered_record_mesg_broadcaster.cpp
    ${GARMIN_SDK_CPP}/fit_crc.cpp
    ${GARMIN_SDK_CPP}/fit_date_time.cpp
    ${GARMIN_SDK_CPP}/fit_decode.cpp
    ${GARMIN_SDK_CPP}/fit_developer_field.cpp
    ${GARMIN_SDK_CPP}/fit_developer_field_definition.cpp
    ${GARMIN_SDK_CPP}/fit_developer_field_description.cpp
    ${GARMIN_SDK_CPP}/fit.cpp
    ${GARMIN_SDK_CPP}/fit_accumulator.cpp
    ${GARMIN_SDK_CPP}/fit_field.cpp
    ${GARMIN_SDK_CPP}/fit_field_base.cpp
    ${GARMIN_SDK_CPP}/fit_field_definition.cpp
    ${GARMIN_SDK_CPP}/fit_mesg.cpp
    ${GARMIN_SDK_CPP}/fit_mesg_broadcaster.cpp
    ${GARMIN_SDK_CPP}/fit_mesg_definition.cpp
    ${GARMIN_SDK_CPP}/fit_mesg_with_event_broadcaster.cpp
    ${GARMIN_SDK_CPP}/fit_profile.cpp
    ${GARMIN_SDK_CPP}/fit_unicode.cpp
)

add_library(garmin-sdk-cpp SHARED ${GARMIN_FIT_SOURCES})

include_directories(${GARMIN_SDK_CPP})

add_subdirectory("src")
