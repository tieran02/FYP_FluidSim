cmake_minimum_required(VERSION 3.17)
project(FYP_FluidSim)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenCL REQUIRED) #Nvidia only support OpenCL 1.2 via the CUDA SDK
add_subdirectory(vendor/glad) #include glad
add_subdirectory(vendor/glfw) #include glfw

file(GLOB_RECURSE  SRC_FILES ${PROJECT_SOURCE_DIR}/src/*)

add_executable(FYP_FluidSim ${SRC_FILES})
target_link_libraries(FYP_FluidSim OpenCL::OpenCL glfw glad)
target_include_directories(FYP_FluidSim PUBLIC ${PROJECT_SOURCE_DIR}/src vendor/glad/include/ vendor/glfw/include/ vendor/glm/glm/ vendor/spdlog/include)

add_custom_command(
        TARGET FYP_FluidSim POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:FYP_FluidSim>/resources)