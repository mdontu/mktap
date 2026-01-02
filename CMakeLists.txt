# SPDX-License-Identifier: GPL-2.0
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree built. Please use the -B <build-dir> option")
endif()

cmake_minimum_required(VERSION 3.19)

project(mktap VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_POSITION_INDEPENDENT_CODE True)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION True)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_FLAGS_DEBUG "-Og -D_DEBUG -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,nodlopen,-z,defs,-z,now,-z,noexecstack,-z,relro,-O,1,--gc-sections,--build-id")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,nodlopen,-z,defs,-z,now,-z,noexecstack,-z,relro,-O,1,--gc-sections,--build-id")

include(GNUInstallDirs)

add_executable(mktap src/mktap.c)
target_compile_definitions(mktap PRIVATE -D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE)
target_compile_options(mktap PRIVATE -Wall -Wextra -Wformat -Wformat=2 -Wconversion -Werror=format-security -ggdb -fvisibility=hidden -ffunction-sections -fdata-sections -fstack-protector-strong -fno-strict-aliasing -fstack-clash-protection -pipe)

install(TARGETS mktap)
