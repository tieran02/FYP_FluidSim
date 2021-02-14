cmake_minimum_required(VERSION 3.17)
project(FYP_FluidSim)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCL REQUIRED) #Nvidia only support OpenCL 1.2 via the CUDA SDK
find_package(OpenMP) #openMD for parallel code
add_subdirectory(vendor/glad) #include glad
add_subdirectory(vendor/glfw) #include glfw

file(GLOB_RECURSE  SRC_FILES ${PROJECT_SOURCE_DIR}/src/*)
list(REMOVE_ITEM SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)

file(GLOB_RECURSE  TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/tests/*)


add_executable(FYP_FluidSim ${PROJECT_SOURCE_DIR}/src/main.cpp ${SRC_FILES})
target_link_libraries(FYP_FluidSim OpenCL::OpenCL glfw glad OpenMP::OpenMP_CXX)
target_compile_definitions(FYP_FluidSim PUBLIC "$<$<CONFIG:DEBUG>:CORE_DEBUG>")

add_executable(FYP_FluidSimTests ${TEST_SRC_FILES} ${SRC_FILES})
target_link_libraries(FYP_FluidSimTests OpenCL::OpenCL glfw glad OpenMP::OpenMP_CXX)
target_compile_definitions(FYP_FluidSimTests PUBLIC "$<$<CONFIG:DEBUG>:CORE_DEBUG>")

if (CMAKE_BUILD_TYPE EQUAL "Debug")
    target_include_directories(FYP_FluidSim PUBLIC ${PROJECT_SOURCE_DIR}/src vendor/glad/debug/include/ vendor/glfw/include/ vendor/glm/glm/ vendor/spdlog/include)
    target_include_directories(FYP_FluidSimTests PUBLIC ${PROJECT_SOURCE_DIR}/src vendor/glad/debug/include/ vendor/glfw/include/ vendor/glm/glm/ vendor/spdlog/include)
else(CMAKE_BUILD_TYPE EQUAL "Release")
    target_include_directories(FYP_FluidSim PUBLIC ${PROJECT_SOURCE_DIR}/src vendor/glad/release/include/ vendor/glfw/include/ vendor/glm/glm/ vendor/spdlog/include)
    target_include_directories(FYP_FluidSimTests PUBLIC ${PROJECT_SOURCE_DIR}/src vendor/glad/release/include/ vendor/glfw/include/ vendor/glm/glm/ vendor/spdlog/include)
endif()

add_custom_command(
        TARGET FYP_FluidSim POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:FYP_FluidSim>/resources)