# Finds:
# EXTERNAL_GLM_INCLUDE_DIR

include(FindPackageHandleStandardArgs)

find_path(EXTERNAL_GLM_INCLUDE_DIR glm/glm.hpp
  PATH_SUFFIXES include
  PATHS ${PROJECT_EXTERNALS_PATH}/glm
  DOC "The path to the GLM header files."
)

if( EXTERNAL_GLM_INCLUDE_DIR STREQUAL "" )
	message("Failed to find EXTERNAL_GLM_INCLUDE_DIR! Using default path!")
endif()

message("EXTERNAL_GLM_INCLUDE_DIR: " ${EXTERNAL_GLM_INCLUDE_DIR})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLM DEFAULT_MSG EXTERNAL_GLM_INCLUDE_DIR)
