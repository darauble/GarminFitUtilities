set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories("command-args")
include_directories("containers")
include_directories("coordinates")
include_directories("directory-scanner")
include_directories("editor")
include_directories("exceptions")
include_directories("metadata")
include_directories("parsers")
include_directories("points-visited")
include_directories("rename-files")

add_subdirectory("directory-scanner")

if (OPT_BUILD_POINTS_VISITED OR OPT_BUILD_EDITOR)
    add_subdirectory("parsers")
endif (OPT_BUILD_POINTS_VISITED OR OPT_BUILD_EDITOR)

if (OPT_BUILD_POINTS_VISITED OR OPT_BUILD_EDITOR)
    add_subdirectory("coordinates")
endif (OPT_BUILD_POINTS_VISITED OR OPT_BUILD_EDITOR)

if (OPT_BUILD_POINTS_VISITED)
    add_subdirectory("command-args")
    add_subdirectory("points-visited")
    add_executable(garmin-points-visited garmin-points-visited.cpp)
    target_link_libraries(garmin-points-visited command-args coordinates directory-scanner points-visited parsers garmin-sdk-cpp)
endif(OPT_BUILD_POINTS_VISITED)

if (OPT_BUILD_RENAME_FILES)
    add_subdirectory("rename-files")
    add_executable(garmin-rename-files garmin-rename-files.cpp)
    target_link_libraries(garmin-rename-files directory-scanner rename-files garmin-sdk-cpp)
endif (OPT_BUILD_RENAME_FILES)

if (OPT_BUILD_EDITOR)
    add_subdirectory("editor")
    add_subdirectory("containers")
    add_subdirectory("metadata")
    add_executable(garmin-edit garmin-edit.cpp)
    target_link_libraries(garmin-edit editor parsers metadata coordinates directory-scanner garmin-sdk-cpp pugixml)
endif(OPT_BUILD_EDITOR)