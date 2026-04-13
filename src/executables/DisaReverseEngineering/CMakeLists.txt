set(MODULES Disa)
set(PRIVATE_LIBRARIES OpenGL GLEW)
set(PRIVATE_LINK_LIBRARIES pg_engine::pg_engine ZLIB::ZLIB ZIPLIB::ZIPLIB Json::Json OpenGL::GLU)

include(${CMAKE_MODULE_PATH}/generators/PG_DLL.cmake)

add_custom_command(TARGET DisaReverseEngineering POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:DisaReverseEngineering>
        "D:/Steam/SteamApps/common/Disgaea PC/j.dll"
)