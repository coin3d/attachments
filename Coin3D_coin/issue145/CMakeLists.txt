set(SOURCES 
	StandardTests.cpp
	TestSuiteMisc.cpp
	TestSuiteInit.cpp
	TestSuiteUtils.cpp
)

#extract test cases from cpp files and store them in CMAKE_CURRENT_BINARY_DIR
message(STATUS "Extracting test case files")
foreach(TEST_SUITE_CANDIDATE ${COIN_TEST_SUITE_CANDIDATES})
	#if a filter is defined add only those files that match the filter string
	string(REGEX REPLACE "/" "_" TEST_SUITE_CANDIDATE_OUT "${TEST_SUITE_CANDIDATE}")
	if(COIN_TEST_SUITE_FILTER)
		string(REGEX MATCH ".*${COIN_TEST_SUITE_FILTER}.*" _filter_does_match "${TEST_SUITE_CANDIDATE_OUT}")
	else()
		set(_filter_does_match 1)
	endif()

	if(_filter_does_match)
		get_filename_component(_class_name "${CMAKE_SOURCE_DIR}/src/${TEST_SUITE_CANDIDATE}" NAME_WE)
		file(READ "${CMAKE_SOURCE_DIR}/src/${TEST_SUITE_CANDIDATE}" _TEST_SUITE_CANDIDATE_CONTENTS)
		#find candidates for the testsuite that contain the test suite identification string
		string(REGEX MATCH "COIN_TEST_SUITE" _is_candidate "${_TEST_SUITE_CANDIDATE_CONTENTS}")
		if(_is_candidate)
			#message(STATUS "Extracting test cases from ${CMAKE_SOURCE_DIR}/src/${TEST_SUITE_CANDIDATE}")
			#preserving semicolons, and empty lines in _TEST_SUITE_CANDIDATE_CONTENTS
			string(REGEX REPLACE ";" "\\\;" _TEST_SUITE_CANDIDATE_CONTENTS "${_TEST_SUITE_CANDIDATE_CONTENTS}")
			string(REGEX REPLACE "\r?\n" "\\n;" _TEST_SUITE_CANDIDATE_CONTENTS "${_TEST_SUITE_CANDIDATE_CONTENTS}")
			set(COIN_STR_SOURCE_CODE)
			set(COIN_TESTSUITE_DECLARATION_HEADER)
			set(COIN_TESTSUITE_OTHER_HEADERS)
			set(_start_found 0)
			set(_line_number 1)
			foreach(_line ${_TEST_SUITE_CANDIDATE_CONTENTS})
				math(EXPR _line_number "${_line_number} + 1")
				if(NOT COIN_TESTSUITE_DECLARATION_HEADER)
					string(REGEX MATCH "^#include.*" _first_name "${_line}")
					#first header is assumed to be the declaration header but exclude config.h for some reason
					string(REGEX MATCH "^#include[ \\t]+\"config.h\"" _found_name "${_first_name}")
					if(NOT _found_name)
						set(COIN_TESTSUITE_DECLARATION_HEADER "${_first_name}")
					endif()
				endif()
				if(_start_found)
					#find ending directive of TEST_SUITE block
					string(REGEX MATCH "#endif.*COIN_TEST_SUITE" _is_end_found "${_line}")
					if(_is_end_found)
						break()
					endif()
					#include everything but include directives in the COIN_TEST_SUITE section
					string(REGEX MATCH "#include.*" _include_name "${_line}")
					if(NOT _include_name)
						set(COIN_STR_SOURCE_CODE "${COIN_STR_SOURCE_CODE}${_line}")
					else()
						set(COIN_TESTSUITE_OTHER_HEADERS "${COIN_TESTSUITE_OTHER_HEADERS}${_include_name}")
						set(COIN_STR_SOURCE_CODE "${COIN_STR_SOURCE_CODE}#line ${_line_number} \"${CMAKE_SOURCE_DIR}/src/${TEST_SUITE_CANDIDATE}\"\n")
					endif()
				endif()
				#find starting directive of TEST_SUITE block
				string(REGEX MATCH "#if.*COIN_TEST_SUITE" _is_start_found "${_line}")
				if(_is_start_found)
					set(_start_found 1)
					set(COIN_STR_SOURCE_CODE "${COIN_STR_SOURCE_CODE}#line ${_line_number} \"${CMAKE_SOURCE_DIR}/src/${TEST_SUITE_CANDIDATE}\"\n")
				endif()
			endforeach()
			if(COIN_STR_SOURCE_CODE)
				configure_file(testfile.cpp.cmake.in ${TEST_SUITE_CANDIDATE_OUT})
			endif()
			set(SOURCES ${SOURCES} ${TEST_SUITE_CANDIDATE_OUT})
		endif(_is_candidate)
	endif()
endforeach()

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/include
	${CMAKE_SOURCE_DIR}/testsuite
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include/Inventor/annex
)

if(COIN_BUILD_SHARED_LIB)
	set(COIN_EXTRA_CPPFLAGS "${COIN_EXTRA_CPPFLAGS} -DCOIN_DLL")
else()
	set(COIN_EXTRA_CPPFLAGS "${COIN_EXTRA_CPPFLAGS} -DCOIN_NOT_DLL")
	set(COIN_EXTRA_LIBS "${COIN_EXTRA_LIBS} $LIBS")
endif()

set(COIN_TESTSUITE_EXTRA_CPPFLAGS "/Fdtestsuite.pdb -DCOIN_INT_TEST_SUITE") # /EHsc
set(COIN_TESTSUITE_EXTRA_LDFLAGS "/SUBSYSTEM:console /DEBUG /NODEFAULTLIB:libcmt")

foreach(_compile_definition ${COIN_TARGET_COMPILE_DEFINITIONS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${_compile_definition}")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D${_compile_definition}")
endforeach()

add_executable(testsuite ${SOURCES})
set_target_properties(testsuite PROPERTIES COMPILE_FLAGS "${COIN_EXTRA_CPPFLAGS} ${COIN_TESTSUITE_EXTRA_CPPFLAGS}")
set_target_properties(testsuite PROPERTIES LINK_FLAGS "${COIN_TESTSUITE_EXTRA_LDFLAGS}")
target_link_libraries(testsuite Coin)
add_test(Coin_TestSuite testsuite)
