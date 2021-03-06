# Finds:
# EXTERNAL_GLEW_INCLUDE_DIR
# EXTERNAL_GLEW_LIBRARIES_RELEASE
# EXTERNAL_GLEW_LIBRARIES_DEBUG

include(FindPackageHandleStandardArgs)

find_path(EXTERNAL_GLEW_INCLUDE_DIR GL/glew.h
  PATH_SUFFIXES include
  PATHS "${PROJECT_EXTERNALS_PATH}/glew-cmake/include" "${PROJECT_EXTERNALS_PATH}/glew-cmake" "${PROJECT_EXTERNALS_PATH}/glew"
  DOC "The path to the GLEW header files."
)

if( EXTERNAL_GLEW_INCLUDE_DIR STREQUAL "" )
	message("Failed to find EXTERNAL_GLEW_INCLUDE_DIR! Using default path!")
endif()

if(MINGW)
#if MSVC you must build glew-cmake
find_library( EXTERNAL_GLEW_LIBRARIES_RELEASE
		  NAMES libglew
		  HINTS "${PROJECT_EXTERNALS_PATH}/glew-cmake" "${PROJECT_EXTERNALS_PATH}/glew"
		  PATH_SUFFIXES "lib" "lib/Release" "lib/Debug/x64"
		)
		
find_library( EXTERNAL_GLEW_LIBRARIES_DEBUG
        NAMES libglew
		HINTS "${CMAKE_BINARY_DIR}" "${PROJECT_EXTERNALS_PATH}/glew-cmake" "${PROJECT_EXTERNALS_PATH}/glew"
		PATH_SUFFIXES "lib" "lib/Debug" "lib/Debug/x64"
		)
elseif(MSVC)
#if MSVC you must build glew-cmake
find_library( EXTERNAL_GLEW_LIBRARIES_RELEASE
		  NAMES glew32
		  HINTS "${PROJECT_EXTERNALS_PATH}/glew-cmake" "${PROJECT_EXTERNALS_PATH}/glew"
		  PATH_SUFFIXES "lib" "lib/Release" "lib/Release/x64"
		)
		
find_library( EXTERNAL_GLEW_LIBRARIES_DEBUG
        NAMES glew32d
		HINTS "${PROJECT_EXTERNALS_PATH}/glew-cmake" "${PROJECT_EXTERNALS_PATH}/glew"
		PATH_SUFFIXES "lib" "lib/Debug" "lib/Debug/x64"
		)
else()
	message(FATAL_ERROR  "Compiler not supported")
endif()
		
message("EXTERNAL_GLEW_INCLUDE_DIR: " ${EXTERNAL_GLEW_INCLUDE_DIR})
message("EXTERNAL_GLEW_LIBRARIES_RELEASE: " ${EXTERNAL_GLEW_LIBRARIES_RELEASE})
message("EXTERNAL_GLEW_LIBRARIES_DEBUG: " ${EXTERNAL_GLEW_LIBRARIES_DEBUG})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW DEFAULT_MSG
                                  EXTERNAL_GLEW_LIBRARIES_RELEASE EXTERNAL_GLEW_LIBRARIES_DEBUG EXTERNAL_GLEW_INCLUDE_DIR)

#set(ALL_LIBRARIES ${GLEW_LIBRARIES} ${ALL_LIBRARIES})