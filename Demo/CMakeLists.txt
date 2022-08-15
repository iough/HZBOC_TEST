cmake_minimum_required(VERSION 3.20) 
get_filename_component(PROJNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJNAME})
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Installing on Windows System.")
else()
    message(FATAL_ERROR "Not support ${CMAKE_SYSTEM_NAME} yet.")
endif()

message(STATUS "Top level CMakeLists.txt")

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/bin)
endforeach()

include(./CommonMacros.txt)
set(APP_NAME "Hiz-Culling")
SETUP_APP(${APP_NAME})

#configure_file(ProjectConfig.h.in ${PROJECT_SOURCE_DIR}/src/ProjectConfig.h)
#source_group(Config FILES ${PROJECT_SOURCE_DIR}/src/ProjectConfig.h)

#target_compile_definitions(${APP_NAME} PUBLIC "TEST_APP")

#add thirdParty
add_library(glad INTERFACE)
target_sources(glad INTERFACE ${PROJECT_SOURCE_DIR}/../thirdParty/GLAD/src/glad.c)
target_include_directories(glad INTERFACE ${PROJECT_SOURCE_DIR}/../thirdParty/GLAD/include)
target_link_libraries(${APP_NAME} PRIVATE glad)
#source_group(Config FILES ${PROJECT_SOURCE_DIR}/../thirdParty/GLAD/src/glad.c)

add_library(glfw INTERFACE)
target_include_directories(glfw INTERFACE  ${PROJECT_SOURCE_DIR}/../thirdParty/GLFW/include)
target_link_libraries(${APP_NAME} PRIVATE glfw)
target_link_libraries(${APP_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../thirdParty/GLFW/lib/glfw3.lib)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE  ${PROJECT_SOURCE_DIR}/../thirdParty/glm)
target_link_libraries(${APP_NAME} PRIVATE general glm)

file(GLOB meshopt_SRC ${PROJECT_SOURCE_DIR}/../thirdParty/meshoptimizer/src/*.cpp)
add_library(meshopt INTERFACE)
target_include_directories(meshopt INTERFACE  ${PROJECT_SOURCE_DIR}/../thirdParty/meshoptimizer/src)
target_sources(meshopt INTERFACE ${meshopt_SRC})
target_link_libraries(${APP_NAME} PRIVATE meshopt)
source_group(Meshoptimizer FILES ${meshopt_SRC})

add_library(tinyobj INTERFACE)
target_include_directories(tinyobj INTERFACE ${PROJECT_SOURCE_DIR}/../thirdParty/tinyobjloader)
target_link_libraries(${APP_NAME} PRIVATE tinyobj)

add_custom_target(UpdateRes VERBATIM COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/../resource" "${PROJECT_BINARY_DIR}/resource")