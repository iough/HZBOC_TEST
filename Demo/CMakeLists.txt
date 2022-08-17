cmake_minimum_required(VERSION 3.20) 
get_filename_component(PROJ_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJ_NAME})
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Installing on Windows System.")
else()
    message(FATAL_ERROR "Not support ${CMAKE_SYSTEM_NAME} yet.")
endif()

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/bin)
endforeach()

#set(CMAKE_SUPPRESS_REGENERATION TRUE)
#message(STATUS "${CMAKE_SUPPRESS_REGENERATION}")

set(APP_NAME "${PROJ_NAME}")
SETUP_APP(${APP_NAME})

#configure_file(ProjectConfig.h.in ${PROJECT_SOURCE_DIR}/src/ProjectConfig.h)
#source_group(Config FILES ${PROJECT_SOURCE_DIR}/src/ProjectConfig.h)

#target_compile_definitions(${APP_NAME} PUBLIC "TEST_APP")

add_library(shaders INTERFACE)
file(GLOB SHADER_SRC ${PROJECT_SOURCE_DIR}/../resource/Shaders/*.*)
target_sources(shaders INTERFACE ${SHADER_SRC})
target_link_libraries(${APP_NAME} PRIVATE shaders)
source_group(Shaders FILES ${SHADER_SRC})

add_library(glad INTERFACE)
target_sources(glad INTERFACE ${PROJECT_SOURCE_DIR}/../thirdParty/GLAD/src/glad.c)
target_include_directories(glad INTERFACE ${PROJECT_SOURCE_DIR}/../thirdParty/GLAD/include)
target_link_libraries(${APP_NAME} PRIVATE glad)

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

#set(PROJ_RES
#	Objects/Car.obj
#	Shaders/Offline.vert
#)
#ADD_SOURCES(UpdateResources PROJ_RES)
add_custom_target(UpdateRes VERBATIM 
	COMMAND ${CMAKE_COMMAND} -E copy_directory 
		"${PROJECT_SOURCE_DIR}/../resource/Objects" 
		"${PROJECT_BINARY_DIR}/bin/resource/Objects"
	COMMAND ${CMAKE_COMMAND} -E copy_directory 
		"${PROJECT_SOURCE_DIR}/../resource/Shaders" 
		"${PROJECT_BINARY_DIR}/bin/resource/Shaders"
)