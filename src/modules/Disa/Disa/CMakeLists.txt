set(PRIVATE_LINK_LIBRARIES pg_engine::pg_engine Json::Json TinyGLTF::TinyGLTF ZIPLIB::ZIPLIB NFD::NFD)
include(${CMAKE_MODULE_PATH}/generators/PG_Module.cmake)