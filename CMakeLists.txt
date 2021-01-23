cmake_minimum_required(VERSION 3.17)
project(FYP_FluidSim)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenCL REQUIRED) #Nvidia only support OpenCL 1.2 via the CUDA SDK
add_subdirectory(vendor/glad) #include glad

add_executable(FYP_FluidSim src/main.cpp)
target_link_libraries(FYP_FluidSim OpenCL::OpenCL glad)
target_include_directories(FYP_FluidSim PUBLIC vendor/glad/include/)