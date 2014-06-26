
cmake_minimum_required(VERSION 2.6)

SET(SEARCHIN lzo bm tbb)
SET(HDRS task_scheduler_init.h lzo1x.h bm.h boost/program_options.hpp)
SET(LLIB tbb lzo2 boost_program_options)
INCLUDE(./Generic.cmake)
PROJECT(ReportDB)

IF(WITH_SSE2)
        # bitmagic can use sse2
        SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -msse2 -DBMSSE2OPT")
        SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -msse2 -DBMSSE2OPT")
        MESSAGE(STATUS "SSE2 enabled")
ENDIF(WITH_SSE2)

AUX_SOURCE_DIRECTORY(src ST)
###MESSAGE(STATUS "sources:: ${ST}")

ADD_LIBRARY(reportDB SHARED ${ST}  )
SET_TARGET_PROPERTIES(reportDB PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(reportDB PROPERTIES VERSION 1.0.1 SOVERSION 1)
TARGET_LINK_LIBRARIES(reportDB ${LLIB} )

GEN_ADD_TEST(test-mdc test/test-mdc.cpp "reportDB")
GEN_ADD_TEST(test-cdb test/test-cdb.cpp "reportDB")
GEN_ADD_TEST(test-tl test/test-tl.cpp "reportDB")
GEN_ADD_TEST(test-bfile test/test-bfile.cpp "reportDB")

ADD_EXECUTABLE(bench-cdb src/tools/bench-cdb.cpp )
TARGET_LINK_LIBRARIES(bench-cdb reportDB)

ADD_EXECUTABLE(bench-thread-cdb src/tools/bench-thread-cdb.cpp )
TARGET_LINK_LIBRARIES(bench-thread-cdb reportDB)

INSTALL(TARGETS reportDB bench-thread-cdb bench-cdb
        RUNTIME DESTINATION             bin
        LIBRARY DESTINATION             lib
        ARCHIVE DESTINATION             lib)

INSTALL(DIRECTORY   include/
		DESTINATION include/reportdb
		PATTERN ".svn" EXCLUDE)

