# Block

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${CMAKE_PROJECT_NAME_LOWER} ${SOURCE_FILES})
target_compile_definitions(${CMAKE_PROJECT_NAME_LOWER} PUBLIC ${QMG2SC_TARGET_DEF})

get_target_property(SystemC_INCLUDE_DIRS SystemC::systemc INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(${CMAKE_PROJECT_NAME_LOWER} PUBLIC ${SystemC_INCLUDE_DIRS})
target_link_libraries(${CMAKE_PROJECT_NAME_LOWER}
    qemu-system-aarch64
    qmg
    communication
    module
    configuration
    common
    cciapi
    SystemC::systemc)

# Install
file(GLOB ${CMAKE_PROJECT_NAME_LOWER}_PUBLIC_HEADERS
    qmg2sc.h
    qmg2sc.hpp)
set_target_properties(${CMAKE_PROJECT_NAME_LOWER} PROPERTIES
		PUBLIC_HEADER "${${CMAKE_PROJECT_NAME_LOWER}_PUBLIC_HEADERS}")
install(TARGETS ${CMAKE_PROJECT_NAME_LOWER}
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		PUBLIC_HEADER DESTINATION include/${CMAKE_PROJECT_NAME_LOWER})
