#
# Try to find OpenGL and include path.
# Once done this will define
#
# OpenGL3_FOUND
# OpenGL3_INCLUDE_PATH
# OpenGL3_LIBRARY
# 

if (WIN32)

	FIND_PATH( OpenGL3_INCLUDE_PATH GL/glcorearb.h
		${DEPENDENCIES_PATH}/OpenGL/
	)
	SET(OpenGL3_LIBRARY OpenGL32)
else()
 SET(OpenGL3_LIBRARY "GL" CACHE STRING "OpenGL lib for Linux")
    FIND_PATH(OpenGL3_INCLUDE_PATH GL/gl.h
      /usr/share/doc/NVIDIA_GLX-1.0/include
      /usr/openwin/share/include
      /opt/graphics/OpenGL/include /usr/X11R6/include
    )
endif ()


SET(OpenGL3_FOUND "NO")
if (OpenGL3_INCLUDE_PATH)
	SET(ALL_LIBRARIES ${ALL_LIBRARIES} ${OpenGL3_LIBRARY})
	SET(ALL_INCLUDE_PATHS ${ALL_INCLUDE_PATHS} ${OpenGL3_INCLUDE_PATH})
	SET(OpenGL3_LIBRARIES ${OpenGL3_LIBRARY})
	SET(OpenGL3_FOUND "YES")
    message("EXTERNAL LIBRARY 'OpenGL3' FOUND")
    message("OpenGL3_LIBRARY: " ${OpenGL3_LIBRARY})
    message("OpenGL3_INCLUDE_PATH: " ${OpenGL3_INCLUDE_PATH})
else()
    message(SEND_ERROR "ERROR: EXTERNAL LIBRARY 'OpenGL3' NOT FOUND")
endif (OpenGL3_INCLUDE_PATH)
