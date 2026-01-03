# SPDX-License-Identifier: GPL-2.0
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree built. Please use the -B <build-dir> option")
endif()

cmake_minimum_required(VERSION 3.19)

project(mktap VERSION 1.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION True)
endif()

add_library(common_exe_flags INTERFACE)
target_compile_definitions(common_exe_flags INTERFACE
	$<$<PLATFORM_ID:Linux>:_FILE_OFFSET_BITS=64>
)
target_compile_options(common_exe_flags INTERFACE
	$<$<CONFIG:Debug>:-Og>
	$<$<PLATFORM_ID:Linux>:-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -fstack-clash-protection>
	-Wall -Wextra -Wformat -Wformat=2 -Wconversion -Werror=format-security -ggdb -fvisibility=hidden -ffunction-sections -fdata-sections -fstack-protector-strong -fno-strict-aliasing
)
target_link_options(common_exe_flags INTERFACE
	$<$<PLATFORM_ID:Linux>:-Wl,-z,nodlopen,-z,defs,-z,now,-z,noexecstack,-z,relro,-O,1,--gc-sections,--build-id>
	$<$<PLATFORM_ID:Darwin>:-Wl,-dead_strip,-no_weak_imports>
	-ggdb
)

include(GNUInstallDirs)

add_executable(mktap src/mktap.c)
target_link_libraries(mktap PRIVATE common_exe_flags)

install(TARGETS mktap)
