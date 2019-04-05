# Finds:
# EXTERNAL_OpenGL3_INCLUDE_DIR
# EXTERNAL_OpenGL3_LIBRARIES

include(FindPackageHandleStandardArgs)

IF (WIN32)
  FIND_PATH( EXTERNAL_OpenGL3_INCLUDE_DIR 
    NAMES GL/glcorearb.h 
	HINTS ${PROJECT_EXTERNALS_PATH}/OpenGL
    PATH_SUFFIXES include
  )
  SET(EXTERNAL_OpenGL3_LIBRARIES OpenGL32)

ELSEIF (APPLE)
 FIND_PATH(EXTERNAL_OpenGL3_INCLUDE_DIR OpenGL/gl3.h 
   OpenGL_ROOT_ENV/OpenGL/)
 SET(EXTERNAL_OpenGL3_LIBRARIES "-framework Cocoa -framework OpenGL -framework IOKit" CACHE STRING "OpenGL lib for OSX")
 
ELSE()

SET(EXTERNAL_OpenGL3_LIBRARIES "GL" CACHE STRING "OpenGL lib for Linux")
    FIND_PATH(OpenGL3_INCLUDE_DIR GL/gl.h
      /usr/share/doc/NVIDIA_GLX-1.0/include
      /usr/openwin/share/include
      /opt/graphics/OpenGL/include /usr/X11R6/include
    )
ENDIF ()

message("EXTERNAL_OpenGL3_INCLUDE_DIR: " ${EXTERNAL_OpenGL3_INCLUDE_DIR})
message("EXTERNAL_OpenGL3_LIBRARIES: " ${EXTERNAL_OpenGL3_LIBRARIES})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenGL3 DEFAULT_MSG
                                  EXTERNAL_OpenGL3_LIBRARIES EXTERNAL_OpenGL3_INCLUDE_DIR)
  
#set(ALL_LIBRARIES ${OpenGL3_LIBRARIES} ${ALL_LIBRARIES})