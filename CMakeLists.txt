cmake_minimum_required(VERSION 3.17)
project(FYP_FluidSim)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenCL REQUIRED) #Nvidia only support OpenCL 1.2 via the CUDA SDK
add_subdirectory(vendor/glad) #include glad
add_subdirectory(vendor/glfw) #include glfw

file(GLOB_RECURSE  SRC_FILES ${PROJECT_SOURCE_DIR}/src/*)

add_executable(FYP_FluidSim ${SRC_FILES})
target_link_libraries(FYP_FluidSim OpenCL::OpenCL glfw glad)
target_include_directories(FYP_FluidSim PUBLIC vendor/glad/include/ vendor/glfw/include/ vendor/glm/glm/)