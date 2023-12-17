cmake_minimum_required(VERSION 3.15)

project(rule2mosdns)

set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(FetchContent)

find_package(curlcpp)
if(NOT curlcpp_FOUND)
    FetchContent_Declare(
        curlcpp
        GIT_REPOSITORY https://github.com/JosephP91/curlcpp.git
        GIT_TAG 3.1
    )
    FetchContent_Populate(curlcpp)
    add_subdirectory(${curlcpp_SOURCE_DIR} ${curlcpp_BINARY_DIR})
endif()

find_package(nlohmann_json)
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_Populate(nlohmann_json)
    add_subdirectory(${nlohmann_json_SOURCE_DIR} ${nlohmann_json_BINARY_DIR})
endif()

find_package(cxxopts)
if(NOT cxxopts_FOUND)
    FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG v3.1.1
    )
    FetchContent_Populate(cxxopts)
    add_subdirectory(${cxxopts_SOURCE_DIR} ${cxxopts_BINARY_DIR})
endif()

set(ABSL_BUILD_TESTING OFF)
set(ABSL_USE_GOOGLETEST_HEAD OFF)
#set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(ABSL_ENABLE_INSTALL ON)

find_package(absl)
if(NOT absl_FOUND)
    FetchContent_Declare(
     absl
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG 20230802.1
    )
    FetchContent_Populate(absl)
    add_subdirectory(${absl_SOURCE_DIR} ${absl_BINARY_DIR})
endif()

find_package(re2)
if(NOT re2_FOUND)
    FetchContent_Declare(
        re2
        GIT_REPOSITORY https://github.com/google/re2.git
        GIT_TAG 2023-11-01
    )
    FetchContent_Populate(re2)
    add_subdirectory(${re2_SOURCE_DIR} ${re2_BINARY_DIR})
endif()

find_package(fmt)
if(NOT fmt_FOUND)
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.1.1
    )
    FetchContent_Populate(fmt)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
endif()


add_library(downloader downloader.hpp downloader.cpp)
target_link_libraries(downloader curlcpp::curlcpp)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json downloader cxxopts::cxxopts fmt::fmt re2::re2)

#add_executable(hosts2dset one_per_line.cpp)
#
#target_link_libraries(hosts2dset downloader)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/links.txt
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/keys_to_remove.txt
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})