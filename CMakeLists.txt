cmake_minimum_required (VERSION 3.10)

project (SCI)
set(NAME "SCI")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g2")

option(SCI_BUILD_TESTS "Build tests" OFF)
message(STATUS "Option: SCI_BUILD_TESTS = ${SCI_BUILD_TESTS}")

option(SCI_BUILD_NETWORKS "Build networks" OFF)
message(STATUS "Option: SCI_BUILD_NETWORKS = ${SCI_BUILD_NETWORKS}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Eigen3 3.3.3 REQUIRED)
find_package(SEAL REQUIRED)
find_package(emp-tool REQUIRED PATHS /usr/local/ NO_DEFAULT_PATH)
find_package(emp-ot REQUIRED PATHS /usr/local/ NO_DEFAULT_PATH)
include_directories(${EIGEN3_INCLUDE_DIR}
                    ${EMP-OT_INCLUDE_DIRS} 
                    ${EMP-TOOL_INCLUDE_DIRS})

## cheetah-core
add_library(gemini SHARED)
include_directories(include)
add_subdirectory(include)
target_link_libraries(gemini SEAL::seal Eigen3::Eigen Threads::Threads)

if (USE_APPROX_RESHARE)
  target_compile_definitions(gemini PUBLIC USE_APPROX_RESHARE=1)
else()
  target_compile_definitions(gemini PUBLIC USE_APPROX_RESHARE=0)
endif()

add_subdirectory(src)

if (SCI_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if (SCI_BUILD_NETWORKS)
    add_subdirectory(networks)
endif()

set(INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
set(BUILD_DIR "${PROJECT_SOURCE_DIR}/build")
include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/SCIConfig.cmake.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/SCIConfig.cmake"
	INSTALL_DESTINATION lib/cmake/SCI
    PATH_VARS INSTALL_DIR BUILD_DIR)
install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SCIConfig.cmake"
        DESTINATION lib/cmake/SCI)