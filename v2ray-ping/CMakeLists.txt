cmake_minimum_required(VERSION 3.22.2)
project("v2ray-ping")

message(STATUS "cmake binary directory: ${CMAKE_BINARY_DIR}")

# Compile commands are output to "compile_commands.json", so that tools such as "ccls" can provide assistance.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set gcc compile options.
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g3 -ggdb")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -Wall -g3 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")
file(GLOB_RECURSE SRC_FILES src/*)
set(UNIT_TEST ${PROJECT_NAME}_test)

add_executable(
    ${PROJECT_NAME}
    ${SRC_FILES}
)

find_package(CURL REQUIRED)
if (CURL_FOUND)
    message(STATUS "libcurl version: ${CURL_VERSION_STRING}")
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
endif(CURL_FOUND)

install(TARGETS ${PROJECT_NAME})
