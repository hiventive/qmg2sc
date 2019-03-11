# Tests

file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach(QMG2SC_TARGET ${QMG2SC_TARGET_LIST})
    set(QMG2SC_TEST_EXEC_NAME ${CMAKE_PROJECT_NAME_LOWER}-${QMG2SC_TARGET}-tests)
    add_executable(${QMG2SC_TEST_EXEC_NAME} ${TEST_FILES})
    target_include_directories(${QMG2SC_TEST_EXEC_NAME} PUBLIC ${GTEST_INCLUDE_DIRS})
    target_compile_definitions(${QMG2SC_TEST_EXEC_NAME} PUBLIC ${QMG2SC_TARGET_DEF})

    set(${CMAKE_PROJECT_NAME_LOWER}-LIBRARIES
            qmg2sc-${QMG2SC_TARGET}
            qmg-${QMG2SC_TARGET}
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

    target_link_libraries(${QMG2SC_TEST_EXEC_NAME} ${${CMAKE_PROJECT_NAME_LOWER}-LIBRARIES})

    add_test(NAME ${CMAKE_PROJECT_NAME}${QMG2SC_TARGET}Tests COMMAND ${QMG2SC_TEST_EXEC_NAME})
endforeach()
