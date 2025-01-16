cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)

project(decode-base64)

include_directories(${CMAKE_JS_INC})
include_directories(${CMAKE_SOURCE_DIR}/addons)
include_directories(${CMAKE_SOURCE_DIR}/node_modules/node-addon-api)

file(GLOB ADDONS_SRC "addons/*.cpp")
add_library(${PROJECT_NAME} SHARED ${ADDONS_SRC} ${CMAKE_JS_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

if(MSVC AND CMAKE_JS_NODEJSLIB_DEF AND CMAKE_JS_NODEJSLIB_TARGET)
    execute_process(COMMAND  ${CMAKE_AR} def:/${CMAKE_JS_NODEJSLIB_DEF} out:/${CMAKE_JS_NODEJSLIB_TARGET} ${CMAKE_STATIC_LINKER_GLAGS})

endif()

message(STATUS "Node.js base64 addon ready to build")