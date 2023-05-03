find_package(OpenMP REQUIRED)

add_library(SCI-FloatingPoint fixed-point.cpp bool-data.cpp)
target_link_libraries(SCI-FloatingPoint
    PUBLIC SCI-Cheetah-BuildingBlocks SCI-Math OpenMP::OpenMP_CXX
)

target_compile_definitions(SCI-FloatingPoint PRIVATE USE_CHEETAH=1)
