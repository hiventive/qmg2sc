# Tests

file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_compile_definitions(${CMAKE_PROJECT_NAME_LOWER} PUBLIC ${QMG2SC_TARGET_DEF})

add_executable(${CMAKE_PROJECT_NAME_LOWER}-tests ${TEST_FILES})
target_include_directories(${CMAKE_PROJECT_NAME_LOWER}-tests PUBLIC ${GTEST_INCLUDE_DIRS})

set(${CMAKE_PROJECT_NAME_LOWER}-LIBRARIES
        ${CMAKE_PROJECT_NAME_LOWER}
		qemu-system-aarch64
		qmg
		communication
        module
        configuration
        common
        cciapi
        SystemC::systemc
		${GTEST_LIBRARIES})

# Disable SPDLog with VS 2012
if(NOT (MSVC AND MSVC_VERSION EQUAL 1700))
	list(APPEND ${CMAKE_PROJECT_NAME_LOWER}-LIBRARIES spdlog::spdlog)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME_LOWER}-tests ${${CMAKE_PROJECT_NAME_LOWER}-LIBRARIES})

add_test(NAME ${CMAKE_PROJECT_NAME}Tests COMMAND ${CMAKE_PROJECT_NAME_LOWER}-tests)
