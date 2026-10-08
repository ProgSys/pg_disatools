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

#temporary for testing: the MCP module of the engine, started with --mcp
set(DISA_DATA_EDITOR_MCP_LIB "${CMAKE_SOURCE_DIR}/../../pg_engine_v2/build/x64-Debug/lib/MCP.lib")
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND EXISTS "${DISA_DATA_EDITOR_MCP_LIB}")
  target_include_directories(${ProjectId} PRIVATE "${CMAKE_SOURCE_DIR}/../../pg_engine_v2/src/modules/MCP")
  target_link_libraries(${ProjectId} PRIVATE "${DISA_DATA_EDITOR_MCP_LIB}" ws2_32 crypt32)
  target_compile_definitions(${ProjectId} PRIVATE DISA_DATA_EDITOR_MCP)
endif()
