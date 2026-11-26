cmake_minimum_required(VERSION 3.20)
project(PsyGoaNebulaFX VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED VST3_SDK_ROOT)
  set(VST3_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../vst3sdk" CACHE PATH "Path to Steinberg VST3 SDK")
endif()
if(NOT EXISTS "${VST3_SDK_ROOT}/CMakeLists.txt")
  message(FATAL_ERROR "VST3 SDK not found. Clone Steinberg's vst3sdk and set -DVST3_SDK_ROOT=/path/to/vst3sdk")
endif()

set(vst3sdk_SOURCE_DIR "${VST3_SDK_ROOT}")
add_subdirectory("${VST3_SDK_ROOT}" "${CMAKE_BINARY_DIR}/vst3sdk" EXCLUDE_FROM_ALL)

smtg_add_vst3plugin(PsyGoaNebulaFX
  source/plugincids.h
  source/processor.h
  source/processor.cpp
  source/controller.h
  source/controller.cpp
)
target_link_libraries(PsyGoaNebulaFX PRIVATE sdk)
target_include_directories(PsyGoaNebulaFX PRIVATE source)

set(PRESET_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/presets")
if(IS_DIRECTORY "${PRESET_RESOURCE_DIR}")
  smtg_target_add_plugin_resources(PsyGoaNebulaFX RESOURCES "${PRESET_RESOURCE_DIR}")
endif()

smtg_target_configure_version_file(PsyGoaNebulaFX)
