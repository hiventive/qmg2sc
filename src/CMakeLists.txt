# Block

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB QMG2SC_PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

foreach(QMG2SC_TARGET ${QMG2SC_TARGET_LIST})
    string(CONCAT QMG2SC_LIB_NAME ${CMAKE_PROJECT_NAME_LOWER} "-${QMG2SC_TARGET}")
    add_library(${QMG2SC_LIB_NAME} ${SOURCE_FILES})

    get_target_property(SystemC_INCLUDE_DIRS SystemC::systemc INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${QMG2SC_LIB_NAME} PUBLIC ${SystemC_INCLUDE_DIRS})
    target_link_libraries(${QMG2SC_LIB_NAME}
        qmg-${QMG2SC_TARGET}
        communication
        module
        configuration
        common
        cciapi
        SystemC::systemc)

    # Install
    # set_target_properties(${QMG2SC_LIB_NAME} PROPERTIES
    #         PUBLIC_HEADER "${${CMAKE_PROJECT_NAME_LOWER}_PUBLIC_HEADERS}")
    install(TARGETS ${QMG2SC_LIB_NAME}
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib)
endforeach()

list(GET QMG2SC_TARGET_LIST 0 QMG2SC_TARGET_LIST_FIRST)
install(FILES ${QMG2SC_PUBLIC_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME_LOWER})
