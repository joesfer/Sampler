cmake_minimum_required(VERSION 2.6)

project(Sampler)

set ( MAYA_PLUGIN_NAME "Sampler" )

# Platform specific definitions
if(WIN32)
    if(MSVC)
        # To enable /MP, parallel build on MSVC
        option(WIN32_USE_MP "Set to OFF to diable /MP." ON)
        MARK_AS_ADVANCED(WIN32_USE_MP)
        if(WIN32_USE_MP)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        endif(WIN32_USE_MP)

        # Other MSVC compilation flags
        add_definitions(-D_SCL_SECURE_NO_WARNINGS)
        add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
		add_definitions(-D NOMINMAX) # prevent windef.h min/max macros from interfering with stl's
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

    endif(MSVC)
endif (WIN32)

# Set the Maya version and architecture (default values)
set(MAYA_VERSION 2011 CACHE STRING "Maya Version")
set(MAYA_ARCH x64 CACHE STRING "HW Architecture")

if ( "${MAYA_ARCH}" STREQUAL "x64")
	set(x64_BUILD true)
endif()

if(WIN32)
	set( MAYA_USER_DIR $ENV{USERPROFILE}/Documents/maya CACHE PATH "Maya user home directory" )
	set( MAYA_SDK_DEFINITIONS "_AFXDLL;_MBCS;NT_PLUGIN;REQUIRE_IOSTREAM" )
	if(x64_BUILD)
		set( MAYA_ROOT $ENV{PROGRAMW6432}/Autodesk/Maya${MAYA_VERSION} CACHE PATH "Maya installation directory" )
	else()
		SET(PROGRAMFILESx86 "ProgramFiles(x86)")
		SET(PF $ENV{${PROGRAMFILESx86}})
		message("PF " ${PF})
		set( MAYA_ROOT ${PF}/Autodesk/Maya${MAYA_VERSION} CACHE PATH "Maya installation directory" )	
	endif()

else()
	set( MAYA_USER_DIR $ENV{HOME}/maya CACHE PATH "Maya user home directory" )
	set( MAYA_ROOT /usr/autodesk/maya${MAYA_VERSION}-${MAYA_ARCH} )
endif()

find_package(OpenGL REQUIRED)

set( GLEW_DIR "C:/glew-1.5.8" CACHE STRING "path to glew installation folder" )
set( GLEW_INCLUDE_DIR ${GLEW_DIR}/include )
set( GLEW_LIBRARY_DIR ${GLEW_DIR}/lib ) 

set( MAYA_HEADERS_DIR ${MAYA_ROOT}/include )
set( MAYA_LIBRARY_DIR ${MAYA_ROOT}/lib )

set ( LOCAL_WARNING_FLAGS /W3 )
set ( LOCAL_RTTI_FLAGS /GR )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin/${MAYA_ARCH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin/${MAYA_ARCH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin/${MAYA_ARCH})

# add base to include directories
include_directories ( ${PUBLIC_INCLUDE_DIRS} ${MAYA_HEADERS_DIR} ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})
link_directories ( ${PUBLIC_LIBRARY_DIRS} ${MAYA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR})

set( OPENGL_LIB opengl32.lib glu32.lib)
set( MAYA_DEFINITIONS "_AFXDLL,_MBCS,NT_PLUGIN,REQUIRE_IOSTREAM" )
set( MAYASDK_LIBRARIES "Foundation.lib" "OpenMaya.lib" "OpenMayaUI.lib" "OpenMayaAnim.lib" "OpenMayaFX.lib" "OpenMayaRender.lib" )

# 64-bit compilation options
if (x64_BUILD)
	message( "64-bit build")
	set( MAYA_DEFINITIONS ${MAYA_DEFINITIONS},Bits64_ )
	set( GLEW_LIB glew64.lib )
	if(UNIX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
	endif(UNIX)
else()
	message( "32-bit build")
	set( GLEW_LIB glew32.lib )
endif()

# print search paths and options
message("Looking for Maya on " ${MAYA_ROOT})
message("Looking for GLEW on " ${GLEW_DIR})
message("Compiling for Maya" ${MAYA_VERSION}-${MAYA_ARCH} )

# specify app sources
file(GLOB_RECURSE SOURCE_FILES src/*.c src/*.cpp src/*.h src/*.inl src/*.hpp src/*.glsl src/*.ui)

add_library( ${MAYA_PLUGIN_NAME} SHARED ${SOURCE_FILES} )
target_link_libraries( ${MAYA_PLUGIN_NAME} ${MAYASDK_LIBRARIES} ${OPENGL_LIB} ${GLEW_LIB} ${GLUT_LIBRARIES})

set_target_properties( ${MAYA_PLUGIN_NAME} PROPERTIES COMPILE_DEFINITIONS ${MAYA_DEFINITIONS} )
set_target_properties( ${MAYA_PLUGIN_NAME} PROPERTIES OUTPUT_NAME ${MAYA_PLUGIN_NAME} )
set_target_properties( ${MAYA_PLUGIN_NAME} PROPERTIES SUFFIX ".mll" )
set_target_properties( ${MAYA_PLUGIN_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1 )
set_target_properties( ${MAYA_PLUGIN_NAME} PROPERTIES LINK_FLAGS "/export:initializePlugin /export:uninitializePlugin" )

