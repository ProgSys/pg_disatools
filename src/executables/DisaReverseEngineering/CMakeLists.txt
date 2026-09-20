set(MODULES Disa)
set(PRIVATE_LIBRARIES OpenGL GLEW)
set(PRIVATE_LINK_LIBRARIES pg_engine::pg_engine ZLIB::ZLIB ZIPLIB::ZIPLIB Json::Json OpenGL::GLU)

# Where the game lives. Everything below hangs off these two, so a different install
# only has to be corrected here (or in the CMake cache).
set(DISGAEA_PC_DIR "D:/Steam/SteamApps/common/Disgaea PC"
	CACHE PATH "Disgaea PC installation directory. The mod is copied there as j.dll.")
set(DISGAEA_PC_EXECUTABLE "${DISGAEA_PC_DIR}/dis1_st.exe"
	CACHE FILEPATH "Game executable started when debugging the mod.")

# The game is a 32 bit process, so only an x86 build produces a DLL it can load.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(STATUS "DisaReverseEngineering: 64 bit build - Disgaea PC (32 bit) cannot load the resulting DLL. Configure with the x86 preset for a usable j.dll.")
endif()

# Basic runtime checks: only the uninitialised variable check (/RTCu) is wanted. The
# stack frame check (/RTCs) does not survive the function hooking - it adds prologue
# code around the very functions that get patched. CMake puts /RTC1 (= /RTCsu) in the
# default debug flags, so it is removed rather than overridden, which would make cl
# warn (D9025). Directory scoped, so only this DLL is affected.
if(MSVC)
	string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
	string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

include(${CMAKE_MODULE_PATH}/generators/PG_DLL.cmake)

if(MSVC)
	target_compile_options(${ProjectId} PRIVATE $<$<CONFIG:Debug>:/RTCu>)
endif()

# The mod is linked against the engine, which the game has to load from its own
# directory too. Findpg_engine only reports the import library, so the DLL is looked up
# beside it: same base name, in the engine build tree's bin/.
foreach(LOOP_VAR DEBUG RELEASE)
	if(EXTERNAL_PG_ENGINE_LIBRARIES_${LOOP_VAR})
		get_filename_component(PG_ENGINE_LIB_DIR "${EXTERNAL_PG_ENGINE_LIBRARIES_${LOOP_VAR}}" DIRECTORY)
		get_filename_component(PG_ENGINE_BUILD_DIR "${PG_ENGINE_LIB_DIR}" DIRECTORY)
		get_filename_component(PG_ENGINE_LIB_NAME "${EXTERNAL_PG_ENGINE_LIBRARIES_${LOOP_VAR}}" NAME_WE)
		find_file(EXTERNAL_PG_ENGINE_DLL_${LOOP_VAR} NAMES "${PG_ENGINE_LIB_NAME}.dll"
			PATHS "${PG_ENGINE_BUILD_DIR}" "${PG_ENGINE_LIB_DIR}"
			PATH_SUFFIXES bin
			NO_DEFAULT_PATH
			DOC "Engine DLL installed next to j.dll (${LOOP_VAR} build).")
	endif()
endforeach()

# Debug and release engines carry different names (pg_engine32d.dll / pg_engine32.dll),
# so the configuration picks. With only one of them around there is nothing to pick.
if(EXTERNAL_PG_ENGINE_DLL_DEBUG AND EXTERNAL_PG_ENGINE_DLL_RELEASE)
	set(PG_ENGINE_DLL "$<IF:$<CONFIG:Debug>,${EXTERNAL_PG_ENGINE_DLL_DEBUG},${EXTERNAL_PG_ENGINE_DLL_RELEASE}>")
elseif(EXTERNAL_PG_ENGINE_DLL_DEBUG)
	set(PG_ENGINE_DLL "${EXTERNAL_PG_ENGINE_DLL_DEBUG}")
elseif(EXTERNAL_PG_ENGINE_DLL_RELEASE)
	set(PG_ENGINE_DLL "${EXTERNAL_PG_ENGINE_DLL_RELEASE}")
else()
	set(PG_ENGINE_DLL "")
	message(WARNING "No engine DLL found beside '${EXTERNAL_PG_ENGINE_LIBRARIES_DEBUG}' - it will not be installed next to j.dll. Build the engine and configure again.")
endif()

# The game loads the mod as "j.dll" from its own directory. The .pdb files stay in the
# build tree - the DLLs carry their absolute path, so the debugger finds them.
if(EXISTS "${DISGAEA_PC_DIR}")
	add_custom_command(TARGET ${ProjectId} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:${ProjectId}>
			"${DISGAEA_PC_DIR}/j.dll"
		COMMENT "Installing $<TARGET_FILE_NAME:${ProjectId}> as ${DISGAEA_PC_DIR}/j.dll"
	)

	if(PG_ENGINE_DLL)
		add_custom_command(TARGET ${ProjectId} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${PG_ENGINE_DLL}"
				"${DISGAEA_PC_DIR}"
			COMMENT "Installing the engine DLL into ${DISGAEA_PC_DIR}"
		)
	endif()
else()
	message(WARNING "DISGAEA_PC_DIR '${DISGAEA_PC_DIR}' does not exist - the mod will not be installed as j.dll.")
endif()

# Debugging: start the game, debug the DLL it loads. Only the Visual Studio generators
# read these properties.
set_target_properties(${ProjectId} PROPERTIES
	VS_DEBUGGER_COMMAND "${DISGAEA_PC_EXECUTABLE}"
	VS_DEBUGGER_WORKING_DIRECTORY "${DISGAEA_PC_DIR}"
)

# The presets build with Ninja, and there Visual Studio takes its debug settings from
# launch.vs.json in the workspace root instead. Seed that file with the same launch, so
# F5 starts the game with the mod under the debugger. Written only when it is absent -
# hand edits, and whatever Visual Studio adds to it later, are left alone.
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/launch.vs.json")
	set(PG_LAUNCH_NAME "Disgaea PC (j.dll)")
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/launch.vs.json.in"
		"${CMAKE_SOURCE_DIR}/launch.vs.json"
		@ONLY
	)
	message(STATUS "Wrote ${CMAKE_SOURCE_DIR}/launch.vs.json with a '${PG_LAUNCH_NAME}' debug configuration.")
endif()
