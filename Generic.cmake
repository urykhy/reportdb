
#INCLUDE(TestForSTDNamespace)

#
# Threads
#
#INCLUDE(FindThreads)

# project() option must be used later
IF(WITH_LLVM)
	MESSAGE(STATUS "llvm/clang enabled")
	SET(CMAKE_C_COMPILER clang)
	SET(CMAKE_CXX_COMPILER clang++)
ENDIF(WITH_LLVM)
enable_language(CXX C)

IF(NOT CMAKE_BUILD_TYPE)
    SET (CMAKE_BUILD_TYPE "RELEASE")
    MESSAGE(STATUS "default build type set to RELEASE")
ELSE(NOT CMAKE_BUILD_TYPE)
	MESSAGE(STATUS "configuring for ${CMAKE_BUILD_TYPE}...")
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(CMAKE_CXX_FLAGS "${CXXFLAGS} ")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual " )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W -pedantic -Wno-long-long -Wformat ")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-variadic-macros " )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal -Wdisabled-optimization " )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter " )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-default-inline " )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -D_THREAD_SAFE -pipe" )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE" )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -ggdb" )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed" )
SET(CMAKE_CXX_FLAGS_DEBUG   "-D_DEBUG -O0 -D_FORTIFY_SOURCE=2 -fno-strict-aliasing -fno-omit-frame-pointer")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

IF(WITH_PGO STREQUAL "generate")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-generate")
    MESSAGE(STATUS "${WITH_PGO} pgo profile...")
ELSE(WITH_PGO STREQUAL "generate")
	IF(WITH_PGO STREQUAL "use")
		SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-use")
		MESSAGE(STATUS "${WITH_PGO} pgo profile...")
	ENDIF(WITH_PGO STREQUAL "use")
ENDIF(WITH_PGO STREQUAL "generate")

IF(WITH_TRACE)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DWITH_TRACE")
	SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -DWITH_TRACE")
	MESSAGE(STATUS "tracing enabled")
ENDIF(WITH_TRACE)

IF(WITH_MUDFLAP)
	FIND_PATH(MUDFLAP_LIBRARY
	NAMES /lib/libmudflapth.so.0 /usr/lib/libmudflapth.so.0 )
	IF(MUDFLAP_LIBRARY)
		MESSAGE(STATUS "mudflap enabled")
		SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -fmudflapth")
		SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fmudflapth")
	ELSE(MUDFLAP_LIBRARY)
		MESSAGE(STATUS "mudflap requested but not found")
	ENDIF(MUDFLAP_LIBRARY)
ENDIF(WITH_MUDFLAP)

INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckLibraryExists)
SET(EXECUTABLE_OUTPUT_PATH bin)
SET(LIBRARY_OUTPUT_PATH lib)

INCLUDE_DIRECTORIES(include)

#
# Doxygen
#

FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
    ADD_CUSTOM_TARGET (doxygen ${DOXYGEN_EXECUTABLE} WORKING_DIRECTORY ${PROJ_SOURCE_DIR} COMMENT "Generating documentation")
ELSE(DOXYGEN_FOUND)
    MESSAGE(STATUS "doxygen documentation unavailable")
ENDIF(DOXYGEN_FOUND)

#
# ctags
#
ADD_CUSTOM_TARGET (tags ctags -R WORKING_DIRECTORY ${PROJ_SOURCE_DIR} COMMENT "Generating tags")

#
# Debian package
#
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "urykhy")
INCLUDE(CPack)

#
# OS detect
#

#
# debug path
#
#IF(${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
#    LINK_DIRECTORIES(/usr/lib/debug)
#ENDIF(${CMAKE_BUILD_TYPE} MATCHES "DEBUG")

#
# lib-tut TESTS
#

FIND_PATH(LIBTUT_INCLUDE
	NAMES	tut.hpp
    PATHS   /usr/include/tut/
            /opt/include/tut/
            /usr/local/include/tut/
            /usr/local/tut/include
    DOC     "tut.hpp ")

IF(LIBTUT_INCLUDE)

	ENABLE_TESTING()
    INCLUDE_DIRECTORIES(${LIBTUT_INCLUDE})

	MACRO(GEN_ADD_TEST binname srcname lib )
		ADD_EXECUTABLE(${binname} ${srcname} )
		TARGET_LINK_LIBRARIES(${binname} ${lib} ${LLIB})
		ADD_TEST(${binname} ./bin/${binname} )
		MESSAGE(STATUS "libtut test added :: ${srcname}")
	ENDMACRO(GEN_ADD_TEST)

ELSE(LIBTUT_INCLUDE)

	MACRO(GEN_ADD_TEST binname srcname lib )
		MESSAGE(STATUS "libtut unavailable, skipped :: ${srcname}")
	ENDMACRO(GEN_ADD_TEST)

ENDIF(LIBTUT_INCLUDE)

#
# scan LLIB variable
#

GET_DIRECTORY_PROPERTY(PROJECT_LINK LINK_DIRECTORIES )
FOREACH(LNAME ${LLIB})
	FIND_LIBRARY( HAVE_${LNAME}
		NAMES ${LNAME}
		PATHS
			${PROJECT_LINK}
		    /usr/lib
            /opt/lib
            /usr/local/lib
		PATH_SUFFIXES ${SEARCHIN})

	IF(  HAVE_${LNAME} )
		GET_FILENAME_COMPONENT(HAVE_${LNAME}_IN ${HAVE_${LNAME}} PATH)
		MESSAGE(STATUS "Found ${LNAME} IN ${HAVE_${LNAME}_IN}")
		LINK_DIRECTORIES( ${HAVE_${LNAME}_IN} )
	ELSE(HAVE_${LNAME})
		MESSAGE(FATAL_ERROR "${LNAME} library not found")
	ENDIF(HAVE_${LNAME})

ENDFOREACH(LNAME)

#
# scan HDRS variable
#

GET_DIRECTORY_PROPERTY(PROJECT_INCLUDES INCLUDE_DIRECTORIES )
FOREACH(HNAME ${HDRS})
	FIND_PATH( HAVE_${HNAME}
		NAMES ${HNAME}
		PATHS
			${PROJECT_INCLUDES}
			/usr/include
            /opt/include
            /usr/local/include
		PATH_SUFFIXES ${SEARCHIN})

	IF(HAVE_${HNAME})
		MESSAGE(STATUS "Found ${HNAME} in ${HAVE_${HNAME}}")
		INCLUDE_DIRECTORIES( ${HAVE_${HNAME}} )
	ELSE(HAVE_${HNAME})
		MESSAGE(FATAL_ERROR "${HNAME} not found")
	ENDIF(HAVE_${HNAME})

ENDFOREACH(HNAME)

IF(MUDFLAP_LIBRARY)
	SET (LLIB ${LLIB} mudflapth)
ENDIF(MUDFLAP_LIBRARY)

IF(WITH_TCMALLOC)
	MESSAGE(STATUS "tcmalloc enabled")
	SET (LLIB ${LLIB} tcmalloc)
ENDIF(WITH_TCMALLOC)

