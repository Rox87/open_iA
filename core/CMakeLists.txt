FILE( GLOB_RECURSE open_iA_core_SRCS "src/*.cpp" "src/*.h" )
FILE( GLOB_RECURSE open_iA_core_test_srcs "src/*Test.cpp" "src/*Test.cxx" )
IF ( NOT "${open_iA_core_test_srcs}" STREQUAL "" )
	LIST( REMOVE_ITEM open_iA_core_SRCS ${open_iA_core_test_srcs} )
ENDIF ()
FILE( GLOB_RECURSE open_iA_core_UI "src/*.ui" )
SET( open_iA_core_QRC open_iA.qrc)

QT5_WRAP_UI( open_iA_core_SRCS ${open_iA_core_UI})
QT5_ADD_RESOURCES( open_iA_core_SRCS ${open_iA_core_QRC} )

SET (LIB_TYPE SHARED)

# Generate core dll file / shared object:
add_library(${CORE_LIBRARY_NAME} ${LIB_TYPE} ${open_iA_core_SRCS} ${open_iA_core_QRC} )
TARGET_LINK_LIBRARIES(${CORE_LIBRARY_NAME} PUBLIC ${QT_LIBRARIES} ${VTK_LIBRARIES} ${ITK_LIBRARIES})
IF (HDF5_FOUND)
	# as HDF5 is required only in core, we could link privately, but under Linux this leads
	# to gui and cmd also requiring linking to it separately, it's easier to link PUBLIC here:
	TARGET_LINK_LIBRARIES(${CORE_LIBRARY_NAME} PUBLIC ${HDF5_LIBRARY})
	# make sure HDF5 is included before itk (which brings its own hdf5 libraries in a different version):
	TARGET_INCLUDE_DIRECTORIES(${CORE_LIBRARY_NAME} BEFORE PRIVATE ${HDF5_INCLUDE_DIR})
	ADD_DEFINITIONS(-DUSE_HDF5)
ENDIF()
IF (WIN32)
	# apparently required for VS 2015, and doesn't hurt for VS2013:
	TARGET_LINK_LIBRARIES(${CORE_LIBRARY_NAME} PUBLIC Opengl32)
ENDIF (WIN32)
IF(OpenMP_CXX_FOUND)
	TARGET_LINK_LIBRARIES(${CORE_LIBRARY_NAME} PUBLIC OpenMP::OpenMP_CXX)
	IF (MSVC)
		TARGET_COMPILE_OPTIONS(${CORE_LIBRARY_NAME} PUBLIC /Zc:twoPhase-)
	ENDIF()
ENDIF()

IF(APPLE)
	INCLUDE_DIRECTORIES( /System/Library/Frameworks )
	FIND_LIBRARY(COCOA_LIBRARY Cocoa)
	FIND_LIBRARY(GLUT_LIBRARY GLUT )
	FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
	MARK_AS_ADVANCED(COCOA_LIBRARY
					 GLUT_LIBRARY
					 OpenGL_LIBRARY)
	SET(EXTRA_LIBS ${COCOA_LIBRARY} ${GLUT_LIBRARY} ${OpenGL_LIBRARY})
	TARGET_LINK_LIBRARIES(${CORE_LIBRARY_NAME} PRIVATE ${EXTRA_LIBS})
	MESSAGE(STATUS "Extra Core: ${EXTRA_LIBS}")
ENDIF (APPLE)

INCLUDE(Modules)
MODULE_GENERATE_EXPORT_HEADER ( ${CORE_LIBRARY_NAME} )

TARGET_INCLUDE_DIRECTORIES(${CORE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})

IF (openiA_PRECOMPILE)
	IF (CMAKE_MAJOR_VERSION GREATER 4 OR
	(CMAKE_MAJOR_VERSION EQUAL 3 AND CMAKE_MINOR_VERSION GREATER 15))
		INCLUDE(precompile.cmake)
		target_precompile_headers(${CORE_LIBRARY_NAME} PRIVATE ${OPENIA_PRECOMPILE_HEADERS})
	ENDIF()
ENDIF()


IF (openiA_TESTING_ENABLED)
	# StringHelperTest
	ADD_EXECUTABLE(StringHelperTest src/iAStringHelperTest.cpp src/iAStringHelper.cpp)
	TARGET_INCLUDE_DIRECTORIES(StringHelperTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})
	TARGET_LINK_LIBRARIES(StringHelperTest PRIVATE ${QT_LIBRARIES})
	target_compile_definitions(StringHelperTest PRIVATE NO_DLL_LINKAGE)
	ADD_TEST(NAME StringHelperTest COMMAND StringHelperTest)
	IF (MSVC)
		STRING(REGEX REPLACE "/" "\\\\" QT_WIN_DLL_DIR ${QT_LIB_DIR})
		SET_TESTS_PROPERTIES(StringHelperTest PROPERTIES ENVIRONMENT "PATH=${QT_WIN_DLL_DIR};$ENV{PATH}")
	ENDIF()

	# Vec3Test
	ADD_EXECUTABLE(Vec3Test src/iAVec3Test.cpp)
	TARGET_INCLUDE_DIRECTORIES(Vec3Test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})
	target_compile_definitions(Vec3Test PRIVATE NO_DLL_LINKAGE)
	ADD_TEST(NAME Vec3Test COMMAND Vec3Test)
	
	ADD_EXECUTABLE(MathUtilTest src/iAMathUtilityTest.cpp src/iAMathUtility.cpp)
	TARGET_INCLUDE_DIRECTORIES(MathUtilTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})
	TARGET_LINK_LIBRARIES(MathUtilTest PRIVATE ${VTK_LIBRARIES})
	target_compile_definitions(MathUtilTest PRIVATE NO_DLL_LINKAGE)
	ADD_TEST(NAME MathUtilTest COMMAND MathUtilTest)

	IF (openiA_USE_IDE_FOLDERS)
		SET_PROPERTY(TARGET StringHelperTest PROPERTY FOLDER "Tests")
		SET_PROPERTY(TARGET Vec3Test PROPERTY FOLDER "Tests")
		SET_PROPERTY(TARGET MathUtilTest PROPERTY FOLDER "Tests")
	ENDIF()

ENDIF()

# Compiler Flags
IF (CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	# see Ulrich Drepper - How to write Shared Libraries
	#   - fPIC vs. fpic
	#   - why fvisibility=hidden
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -fvisibility=hidden")
	SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic -fvisibility=hidden")
ENDIF()
IF( "${CMAKE_SYSTEM_NAME}" MATCHES "Darwin" )
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework OpenGL")
ENDIF()

# Installation
IF (WIN32)
	INSTALL (TARGETS ${CORE_LIBRARY_NAME} RUNTIME DESTINATION .)
ELSEIF (FLATPAK_BUILD)
	INSTALL (TARGETS ${CORE_LIBRARY_NAME} LIBRARY DESTINATION lib)
ELSE ()
	INSTALL (TARGETS ${CORE_LIBRARY_NAME} LIBRARY DESTINATION .)
ENDIF(WIN32)

IF (MSVC)
	# Apply file grouping based on regular expressions for Visual Studio IDE.
	SOURCE_GROUP("UI Files" REGULAR_EXPRESSION "[.](ui|qrc)$")
ENDIF()
