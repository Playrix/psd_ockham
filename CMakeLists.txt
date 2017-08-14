cmake_minimum_required(VERSION 3.6)

project(psd_ockham)
set (TARGET_NAME psd_ockham)

set(CMAKE_CONFIGURATION_TYPES Debug;Release)

#List source directory
set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}/src")
file(
	GLOB_RECURSE project_source_list 
	LIST_DIRECTORIES false
	"${_src_root_path}/*.c*"
	"${_src_root_path}/*.h*"
)

#Group source files by folders
foreach(_source IN ITEMS ${project_source_list})
	get_filename_component(_source_path "${_source}" PATH)
	file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
	string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
	source_group("${_group_path}" FILES "${_source}")
endforeach()

#Add headers
include_directories(
	"src/libpsd/include"
)

#Target
add_executable(${TARGET_NAME} ${project_source_list})