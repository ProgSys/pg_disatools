set(PRIVATE_LINK_LIBRARIES pg_engine::pg_engine Json::Json TinyGLTF::TinyGLTF ZIPLIB::ZIPLIB NFD::NFD)
include(${CMAKE_MODULE_PATH}/generators/PG_Module.cmake)

# The module installs j.dll and checks mods against it, so it carries the j.dll version, not one of its own.
if(NOT PG_APP_VERSION_DisaReverseEngineering)
	message(FATAL_ERROR "Disa: no version for DisaReverseEngineering registered, see src/Versions.cmake.")
endif()
set(DISA_MOD_VERSION ${PG_APP_VERSION_DisaReverseEngineering})
pg_split_version(${DISA_MOD_VERSION} DISA_MOD_VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/PG_DisaVersion.h.in" "${CMAKE_CURRENT_BINARY_DIR}/generated/Disa/PG_DisaVersion.h" @ONLY)
target_include_directories(${ProjectId} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
