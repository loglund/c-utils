cmake_minimum_required(VERSION 3.20)

project(c-utils VERSION 0.1.0 LANGUAGES C)

add_library(c-utils INTERFACE)
add_library(c-utils::c-utils ALIAS c-utils)

target_include_directories(c-utils
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

include(GNUInstallDirs)

install(
	TARGETS c-utils
	EXPORT c-utilsTargets
)

install(
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
	EXPORT c-utilsTargets
	FILE c-utilsTargets.cmake
	NAMESPACE c-utils::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/c-utils
)

option(C_UTILS_BUILD_TESTING "Build GoogleTest-based tests" ${PROJECT_IS_TOP_LEVEL})
if(C_UTILS_BUILD_TESTING)
	enable_language(CXX)
	include(CTest)
	enable_testing()

	include(FetchContent)

	FetchContent_Declare(
		googletest
		URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
	)

	# Never install test dependencies.
	set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

	# Prevent GoogleTest from overriding the parent project's compiler/linker settings on Windows.
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

	FetchContent_MakeAvailable(googletest)

	# Never install test targets from this project.
	set(_c_utils_skip_install_rules "${CMAKE_SKIP_INSTALL_RULES}")
	set(CMAKE_SKIP_INSTALL_RULES ON)
	add_subdirectory(tests)
	set(CMAKE_SKIP_INSTALL_RULES "${_c_utils_skip_install_rules}")
endif()
