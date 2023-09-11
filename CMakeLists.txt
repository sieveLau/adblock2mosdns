cmake_minimum_required(VERSION 3.15)

project(adblock2mosdns)

set(CMAKE_CXX_STANDARD 20)

ADD_SUBDIRECTORY(ext/curlcpp)
INCLUDE_DIRECTORIES(ext/curlcpp/include)
ADD_SUBDIRECTORY(ext/json)
INCLUDE_DIRECTORIES(ext/json/include)
ADD_SUBDIRECTORY(ext/cxxopts)
INCLUDE_DIRECTORIES(ext/cxxopts/include)
set(ABSL_BUILD_TESTING OFF)
set(ABSL_USE_GOOGLETEST_HEAD OFF)
#set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(ABSL_ENABLE_INSTALL ON)
add_subdirectory(ext/absl)
include_directories(ext/absl)
add_subdirectory(ext/re2)
include_directories(ext/re2)
add_subdirectory(ext/fmt)
include_directories(ext/fmt/include)

add_library(downloader downloader.hpp downloader.cpp)
target_link_libraries(downloader curlcpp::curlcpp)

add_executable(adblock2mosdns main.cpp)

target_link_libraries(adblock2mosdns nlohmann_json::nlohmann_json downloader cxxopts::cxxopts fmt::fmt re2::re2)

#add_executable(hosts2dset one_per_line.cpp)
#
#target_link_libraries(hosts2dset downloader)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/links.txt
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/keys_to_remove.txt
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})