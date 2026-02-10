cmake_minimum_required(VERSION 3.19)
project(cadsh VERSION 0.1)

#set(CMAKE_CXX_STANDARD 20)

string(TIMESTAMP BUILDDATE)
add_compile_options(-ggdb -DVERSION="${CMAKE_PROJECT_VERSION}"  -DBUILDDATE="${BUILDDATE}")

add_executable(cadsh src/cadsh.cpp )

find_package(PkgConfig REQUIRED)

#Establishes the target install directory for any built libraries:
set(BUILD_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/external/usr")


#manifold:
if (DEFINED BUILD_MANIFOLD)
	if (DEFINED HOST)
		set(CMAKECMD "${HOST}-cmake")
	else()
		set(CMAKECMD "cmake")
	endif()
	set(MANIFOLD_DEPS -lpng -lz)
	set(CMAKE_ARGUMENTS -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DMANIFOLD_DEBUG=ON -DMANIFOLD_USE_BUILTIN_CLIPPER2=ON -DCMAKE_INSTALL_PREFIX=${BUILD_PREFIX} )

	include(ExternalProject)
	if (EXISTS ${BUILD_MANIFOLD})
		message(STATUS "Configuring manifold as an external project (local file: ${BUILD_MANIFOLD}).")
		ExternalProject_Add(manifold_download
			URL ${BUILD_MANIFOLD}
			DOWNLOAD_EXTRACT_TIMESTAMP FALSE
			PREFIX external/manifold
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
		)
	else ()
		message(FATAL_ERROR "BUILD_MANIFOLD not valid: ${BUILD_MANIFOLD}" )
	endif()

	set(MANIFOLD_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/include)
	set(MANIFOLD_INC_DIR ${MANIFOLD_INCLUDE_DIR} CACHE STRING "manifold include directory" )
	set(MANIFOLD_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib)
	set(MANIFOLD_LIB ${MANIFOLD_LIB_DIR}/libmanifold.a ${MANIFOLD_LIB_DIR}/libmanifoldc.a ${MANIFOLD_LIB_DIR}/libClipper2.a)
	
	add_library(manifold STATIC IMPORTED)
	set_target_properties(manifold PROPERTIES IMPORTED_LOCATION ${MANIFOLD_LIB_DIR} ) 
	set(MANIFOLD_FOUND TRUE)
	add_dependencies(cadsh manifold_download)
	
	target_include_directories(cadsh PRIVATE ${MANIFOLD_INCLUDE_DIR})
	target_link_libraries(cadsh ${MANIFOLD_LIB}  ${MANIFOLD_DEPS})
else()
	find_package( Manifold REQUIRED )
	if (Manifold_FOUND)
		include_directories( ${Manifold_INCLUDE_DIRS} )
		target_link_libraries( cadsh ${Manifold_LIBS} )
	else()
		message(FATAL_ERROR "Manifold not found")
	endif(Manifold_FOUND)
endif ()


add_subdirectory(src)

