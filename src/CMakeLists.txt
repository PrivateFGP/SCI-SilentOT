set(CMAKE_FIND_DEBUG_MODE 1)
include_directories(${CMAKE_CURRENT_LIST_DIR})

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_subdirectory(utils)
add_subdirectory(OT)
add_subdirectory(Millionaire)
add_subdirectory(BuildingBlocks)

add_subdirectory(LinearOT)
add_subdirectory(LinearHE)
add_subdirectory(NonLinear)
add_subdirectory(Math)
add_subdirectory(FloatingPoint)
add_library(SCI-common INTERFACE)

find_package(Threads REQUIRED)
target_link_libraries(SCI-common
    INTERFACE
    SCI-NonLinear
    Threads::Threads
)

target_include_directories(SCI-common
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:./>
)

add_library(SCI-HE library_fixed_uniform.cpp globals.cpp)
target_link_libraries(SCI-HE PUBLIC SCI-common SCI-LinearHE SCI-BuildingBlocks SCI-Math Eigen3::Eigen)
target_compile_definitions(SCI-HE PUBLIC SCI_HE=1 SCI_CHEETAH=0)

# Cheetah releated
add_subdirectory(cheetah)
add_library(SCI-Cheetah library_fixed_uniform_cheetah.cpp library_fixed_uniform.cpp library_fixed.cpp globals.cpp cleartext_library_fixed.cpp)
target_link_libraries(SCI-Cheetah PUBLIC SCI-common Cheetah-Linear SCI-Cheetah-BuildingBlocks SCI-Math SCI-FloatingPoint Eigen3::Eigen)
target_compile_definitions(SCI-Cheetah PUBLIC SCI_OT=1 USE_CHEETAH=1)

if (OPENMP_FOUND)
    target_link_libraries(SCI-HE PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(SCI-Cheetah PUBLIC OpenMP::OpenMP_CXX)
endif()

install(TARGETS
        SCI-HE
        SCI-common
        SCI-BuildingBlocks
        SCI-Cheetah-BuildingBlocks
        Cheetah-Linear
        SCI-OTPrimitive
        SCI-LinearOT
        SCI-LinearHE
        SCI-NonLinear
        SCI-Math
        SCI-Millionaire
        SCI-utils
        SCI-FloatingPoint
        SCI-Cheetah
        EXPORT SCITargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

install(EXPORT SCITargets
        FILE SCITargets.cmake
        NAMESPACE SCI::
        DESTINATION lib/cmake/SCI)

install(DIRECTORY
        utils
        OT
        Millionaire
        NonLinear
        BuildingBlocks
        LinearOT
        LinearHE
        Math
        FloatingPoint
        DESTINATION include
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp")

install(FILES
        "defines.h"
        "defines_uniform.h"
        "globals.h"
        "library_fixed.h"
        "library_fixed_uniform.h"
        "cleartext_library_fixed.h"
        "cleartext_library_fixed_uniform.h"
        DESTINATION include)

install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/utils/cmake/FindGMP.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/utils/cmake/source_of_randomness.cmake"
        DESTINATION lib/cmake/SCI)
