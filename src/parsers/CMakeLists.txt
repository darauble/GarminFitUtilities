file(GLOB PARSERS "*.cpp")
add_library(parsers STATIC ${PARSERS})
target_link_libraries(parsers pugixml)
