project(PackageTest CXX)
enable_language(C) # Fix CMake 3.1.0 (at least) thread issue
cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_CXX_STANDARD 11)

# # Thread
# set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
# set(THREADS_PREFER_PTHREAD_FLAG TRUE)

# find_package(Threads REQUIRED)
# find_package(SystemCLanguage 2.3.2 REQUIRED CONFIG)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

if(NOT QMG2SC_TARGET_LIST)
    set(QMG2SC_TARGET_LIST "arm")
endif()
list(GET QMG2SC_TARGET_LIST 0 QMG2SC_TARGET_CONAN_TEST)

add_executable(example example.cpp)
message(${CONAN_LIBS})
target_link_libraries(example
    ${CONAN_LIBS})

enable_testing()
add_test(NAME example
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
		COMMAND example)
