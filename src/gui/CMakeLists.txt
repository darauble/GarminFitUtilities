if(OPT_BUILD_GUI AND wxWidgets_FOUND)
    # Find Mapnik using mapnik-config
    find_program(MAPNIK_CONFIG mapnik-config)
    if(NOT MAPNIK_CONFIG)
        message(FATAL_ERROR "mapnik-config not found. Please install libmapnik-dev")
    endif()

    # Check for GDAL/OGR (for ogr2ogr runtime dependency)
    find_program(OGR2OGR_EXECUTABLE ogr2ogr)
    if(OGR2OGR_EXECUTABLE)
        message(STATUS "ogr2ogr found at: ${OGR2OGR_EXECUTABLE}")
        message(STATUS "PBF to Spatialite conversion will be available")
    else()
        message(WARNING "ogr2ogr not found. Install gdal-bin for PBF to Spatialite conversion support")
    endif()

    # Get Mapnik configuration
    execute_process(COMMAND ${MAPNIK_CONFIG} --version OUTPUT_VARIABLE MAPNIK_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${MAPNIK_CONFIG} --cflags OUTPUT_VARIABLE MAPNIK_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${MAPNIK_CONFIG} --libs OUTPUT_VARIABLE MAPNIK_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${MAPNIK_CONFIG} --ldflags OUTPUT_VARIABLE MAPNIK_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${MAPNIK_CONFIG} --includes OUTPUT_VARIABLE MAPNIK_INCLUDE_DIRS OUTPUT_STRIP_TRAILING_WHITESPACE)

    message(STATUS "Mapnik found - version ${MAPNIK_VERSION}")
    message(STATUS "Mapnik includes: ${MAPNIK_INCLUDE_DIRS}")
    message(STATUS "Mapnik libs: ${MAPNIK_LIBS}")

    # Parse include dirs (remove -I prefix)
    string(REPLACE "-I" "" MAPNIK_INCLUDE_DIRS "${MAPNIK_INCLUDE_DIRS}")
    separate_arguments(MAPNIK_INCLUDE_DIRS)

    # GUI application executable
    set(GUI_SOURCES
        main.cpp
        MainFrame.cpp
        panels/ActivitiesPanel.cpp
        panels/ActivityDetailsPanel.cpp
        panels/FileTreePanel.cpp
        panels/ProductEditorPanel.cpp
        panels/TimestampEditorPanel.cpp
        panels/GpxEditorPanel.cpp
        panels/RawEditorPanel.cpp
        panels/PointsPanel.cpp
        panels/RenamePanel.cpp
        panels/MapPanel.cpp
        panels/MapRenderer.cpp
        dialogs/ActivityDetailDialog.cpp
        dialogs/SettingsDialog.cpp
        models/ActivityDataModel.cpp
        models/FileSystemModel.cpp
        models/CoordinateModel.cpp
        utils/GuiUtils.cpp
        utils/SettingsManager.cpp
        utils/MetadataCache.cpp
        utils/DataDirectoryResolver.cpp
        interfaces/IFileOperations.cpp
    )

    add_executable(garmin-disconnect ${GUI_SOURCES})

    # Link with existing libraries
    target_link_libraries(garmin-disconnect
        ${wxWidgets_LIBRARIES}
        editor
        parsers
        metadata
        coordinates
        directory-scanner
        command-args
        rename-files
        points-visited
        garmin-sdk-cpp
        pugixml
    )

    # Include directories
    target_include_directories(garmin-disconnect PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${wxWidgets_INCLUDE_DIRS}
        ${MAPNIK_INCLUDE_DIRS}
    )

    # Add compile definition for Mapnik
    target_compile_definitions(garmin-disconnect PRIVATE HAVE_MAPNIK)

    # Add Mapnik compile flags
    target_compile_options(garmin-disconnect PRIVATE ${MAPNIK_CFLAGS})

    # Link Mapnik libraries
    separate_arguments(MAPNIK_LIBS)
    separate_arguments(MAPNIK_LDFLAGS)
    target_link_libraries(garmin-disconnect ${MAPNIK_LIBS})
    target_link_options(garmin-disconnect PRIVATE ${MAPNIK_LDFLAGS})

    # Windows-specific settings
    if(WIN32)
        set_target_properties(garmin-disconnect PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    endif()

    # Set C++ standard
    set_target_properties(garmin-disconnect PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )

    # Install data files to share/garmin-disconnect/data
    set(DATA_DIR "${CMAKE_SOURCE_DIR}/data")
    if(EXISTS ${DATA_DIR})
        # Install all data files
        install(DIRECTORY ${DATA_DIR}/
                DESTINATION share/garmin-disconnect/data
                FILES_MATCHING
                PATTERN "*.xml"
                PATTERN "*.ini"
                PATTERN "*.txt"
                PATTERN "*.md")
        message(STATUS "Data files will be installed from ${DATA_DIR}")
    else()
        message(WARNING "Data directory not found at ${DATA_DIR}")
    endif()

    message(STATUS "GUI application (garmin-disconnect) will be built")
else()
    message(STATUS "GUI application will not be built (OPT_BUILD_GUI=${OPT_BUILD_GUI}, wxWidgets_FOUND=${wxWidgets_FOUND})")
endif()