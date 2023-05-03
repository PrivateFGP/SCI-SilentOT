add_library(SCI-LinearOT linear-ot.cpp)
target_link_libraries(SCI-LinearOT
    PUBLIC
    SCI-Cheetah-BuildingBlocks
    Eigen3::Eigen
)

target_compile_definitions(SCI-LinearOT PRIVATE USE_CHEETAH=1)


