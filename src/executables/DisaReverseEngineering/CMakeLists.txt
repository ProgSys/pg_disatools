set(MODULES Disa)
set(PRIVATE_LIBRARIES OpenGL GLEW)
set(PRIVATE_LINK_LIBRARIES pg_engine::pg_engine ZLIB::ZLIB ZIPLIB::ZIPLIB Json::Json)

include(${CMAKE_MODULE_PATH}/generators/PG_DLL.cmake)