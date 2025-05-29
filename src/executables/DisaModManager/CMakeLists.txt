cmake_minimum_required(VERSION 3.5)
set(MODULES Disa)
set(PRIVATE_LINK_LIBRARIES pg_engine::pg_engine NFD::NFD Json::Json)
include(${CMAKE_MODULE_PATH}/generators/PG_Executable.cmake)


get_filename_component(DEBUG_DLL_DIR "${EXTERNAL_PG_ENGINE_LIBRARIES_DEBUG}" DIRECTORY)
get_filename_component(RELEASE_DLL_DIR "${EXTERNAL_PG_ENGINE_LIBRARIES_RELEASE}" DIRECTORY)

message("DEBUG_DLL_DIR" ${DEBUG_DLL_DIR})
set_target_properties(DisaMapEditor PROPERTIES
  VS_DEBUGGER_ENVIRONMENT
    "$<$<CONFIG:Debug>:PATH=${DEBUG_DLL_DIR};%PATH%>\$<$<CONFIG:Release>:PATH=${RELEASE_DLL_DIR};%PATH%>"
)