set(MODULES Disa)
set(PRIVATE_LINK_LIBRARIES pg_engine::pg_engine NFD::NFD Json::Json)
include(${CMAKE_MODULE_PATH}/generators/PG_Executable.cmake)


get_filename_component(DEBUG_DLL_DIR "${EXTERNAL_PG_ENGINE_LIBRARIES_DEBUG}" DIRECTORY)
get_filename_component(RELEASE_DLL_DIR "${EXTERNAL_PG_ENGINE_LIBRARIES_RELEASE}" DIRECTORY)

set_target_properties(${ProjectId} PROPERTIES
  VS_DEBUGGER_ENVIRONMENT
    "$<$<CONFIG:Debug>:PATH=${DEBUG_DLL_DIR};%PATH%>\$<$<CONFIG:Release>:PATH=${RELEASE_DLL_DIR};%PATH%>"
)

if(MSVC)
  target_link_options(${ProjectId}  PRIVATE "/MANIFEST:NO")
endif()
