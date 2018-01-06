cmake_minimum_required(VERSION 3.6)

function(make_source_groups _IN_SOURCES _IN_ROOT_DIR _IN_ROOT_GROUP)
	foreach(_source IN ITEMS ${${_IN_SOURCES}})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${_IN_ROOT_DIR}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		if (_IN_ROOT_GROUP)
			set(_group_path ${_IN_ROOT_GROUP}\\${_group_path})
		endif()
		source_group("${_group_path}" FILES "${_source}")
	endforeach()
endfunction(make_source_groups)

project(psd_ockham)
set (TARGET_NAME psd_ockham)

set(CMAKE_CONFIGURATION_TYPES Debug;Release)

#List source directory
set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}/src")
file(
	GLOB_RECURSE PROJECT_SOURCES 
	LIST_DIRECTORIES false
	"${_src_root_path}/*.c"
	"${_src_root_path}/*.h"
)

make_source_groups(PROJECT_SOURCES ${_src_root_path} "")

#Add headers
include_directories(
	"src/libpsd/include"
)

#Target
add_executable(${TARGET_NAME} ${PROJECT_SOURCES})

if (WIN32)
	set (TARGET_NAME psd_ockham_gui)

	set(CMAKE_CONFIGURATION_TYPES Debug;Release)

	set(PROJECT_DEFINITIONS _UNICODE UNICODE)

	#List source directory
	file(
		GLOB_RECURSE C_PROJECT_SOURCES
		LIST_DIRECTORIES false
		"${_src_root_path}/libpsd/*.c"
		"${_src_root_path}/libpsd/*.h"
	)
	make_source_groups(${C_PROJECT_SOURCES} ${_src_root_path})
	set(C_PROJECT_SOURCES ${C_PROJECT_SOURCES} ${_src_root_path}/version.h)
	source_group("libpsd" FILES ${_src_root_path}/version.h)

	set(_src_gui_root_path "${CMAKE_CURRENT_SOURCE_DIR}/src-gui/win")
	file(
		GLOB_RECURSE CPP_PROJECT_SOURCES
		LIST_DIRECTORIES false
		"${_src_gui_root_path}/*.cpp"
		"${_src_gui_root_path}/*.h"
	)
	make_source_groups(CPP_PROJECT_SOURCES ${_src_gui_root_path} "src")

	#Add headers
	include_directories(
		"src/libpsd/include"
		"src"
	)

	file(GLOB_RECURSE PROJECT_RESOURCES
		LIST_DIRECTORIES false
		"${_src_gui_root_path}/*.ico"
		"${_src_gui_root_path}/*.rc"
	)
	make_source_groups(PROJECT_RESOURCES ${_src_gui_root_path} "resources")

	#Target
	add_executable(${TARGET_NAME} ${C_PROJECT_SOURCES} ${CPP_PROJECT_SOURCES} ${PROJECT_RESOURCES})

	set_target_properties(${TARGET_NAME} PROPERTIES
		RESOURCE "${PROJECT_RESOURCES}"
		LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif()