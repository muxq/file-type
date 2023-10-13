cmake_minimum_required(VERSION 3.11)
project(
  ft
  VERSION 1.0.0
  DESCRIPTION "file type program")

include(FetchContent)
# jsoncpp
FetchContent_Declare(
  jsoncpp
  GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
  GIT_TAG master)
FetchContent_GetProperties(jsoncpp)
if(NOT jsoncpp_POPULATED)
  FetchContent_Populate(jsoncpp)
endif()

set(JSON_SRC ${jsoncpp_SOURCE_DIR}/src/lib_json)

set(OPTION_DEPS_JSON_SOURCE)
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${JSON_SRC}/json_reader.cpp
    ${JSON_SRC}/json_value.cpp ${JSON_SRC}/json_writer.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${jsoncpp_SOURCE_DIR}/include
                                                   ${JSON_SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE shlwapi)
