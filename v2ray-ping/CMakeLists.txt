set(MODULE_NAME "v2ray-ping")
message(STATUS "module: ${MODULE_NAME}")

add_executable(
    ${MODULE_NAME}
    main.cpp
    v2ray-ping.cpp
    resource.cpp
    error-handler.cpp
    Socks5.cpp
    HttpClient.cpp
    V2rayProcess.cpp
)

find_package(CURL REQUIRED)
if (CURL_FOUND)
    message(STATUS "libcurl version: ${CURL_VERSION_STRING}")
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} ${CURL_LIBRARIES})
endif(CURL_FOUND)
