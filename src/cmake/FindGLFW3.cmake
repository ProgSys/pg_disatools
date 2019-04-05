include(FindPackageHandleStandardArgs)

find_path(GLFW3_INCLUDE_DIR GLFW/glfw3.h
  PATH_SUFFIXES include GLFW
)

find_library(GLFW3_LIBRARIES
  NAMES glfw3 glfw
  PATH_SUFFIXES lib
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLFW3 DEFAULT_MSG
                                  GLFW3_LIBRARIES GLFW3_INCLUDE_DIR)
