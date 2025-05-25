cmake_minimum_required(VERSION 3.5)
set(PRIVATE_LINK_LIBRARIES pg_engine::pg_engine Json::Json TinyGLTF::TinyGLTF ZIPLIB::ZIPLIB)
include(${CMAKE_MODULE_PATH}/generators/PG_Module.cmake)