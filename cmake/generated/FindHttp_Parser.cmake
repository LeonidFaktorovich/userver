# AUTOGENERATED, DON'T CHANGE THIS FILE!

 
                  
 
 

include(FindPackageHandleStandardArgs)

find_library(Http_Parser_LIBRARIES_http_parser_libhttp_parser NAMES
  http_parser libhttp_parser
  )
list(APPEND Http_Parser_LIBRARIES
  ${Http_Parser_LIBRARIES_http_parser_libhttp_parser}
)
  
find_path(Http_Parser_INCLUDE_DIRS_http_parser_h NAMES
  http_parser.h
  )
list(APPEND Http_Parser_INCLUDE_DIRS
  ${Http_Parser_INCLUDE_DIRS_http_parser_h}
)
  
 

if(UNIX AND NOT APPLE)
  find_program(DPKG_QUERY_BIN dpkg-query)
  if(DPKG_QUERY_BIN)
    execute_process(
      COMMAND dpkg-query --showformat=\${Version} --show libhttp-parser-dev
      OUTPUT_VARIABLE Http_Parser_version_output
      ERROR_VARIABLE Http_Parser_version_error
      RESULT_VARIABLE Http_Parser_version_result
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(Http_Parser_version_result EQUAL 0)
      set(Http_Parser_VERSION ${Http_Parser_version_output})
      message(STATUS "Installed version libhttp-parser-dev: ${Http_Parser_VERSION}")
    endif(Http_Parser_version_result EQUAL 0)
  endif(DPKG_QUERY_BIN)
endif(UNIX AND NOT APPLE)
 
if(APPLE)
  find_program(BREW_BIN brew)
  if(BREW_BIN)
    execute_process(
      COMMAND brew list --versions http-parser
      OUTPUT_VARIABLE Http_Parser_version_output
      ERROR_VARIABLE Http_Parser_version_error
      RESULT_VARIABLE Http_Parser_version_result
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(Http_Parser_version_result EQUAL 0)
      if (Http_Parser_version_output MATCHES "^(.*) (.*)$")
        set(Http_Parser_VERSION ${CMAKE_MATCH_2})
        message(STATUS "Installed version http-parser: ${Http_Parser_VERSION}")
      else()
        set(Http_Parser_VERSION "NOT_FOUND")
      endif()
    else()
      message(WARNING "Failed execute brew: ${Http_Parser_version_error}")
    endif(Http_Parser_version_result EQUAL 0)
  endif(BREW_BIN)
endif(APPLE)
 
 
 
find_package_handle_standard_args(
  Http_Parser
    REQUIRED_VARS
      Http_Parser_LIBRARIES
      Http_Parser_INCLUDE_DIRS
      
    FAIL_MESSAGE
      "Could not find `Http_Parser` package. Debian: sudo apt update && sudo apt install libhttp-parser-dev MacOS: brew install http-parser Version: 2.8"
)
mark_as_advanced(
  Http_Parser_LIBRARIES
  Http_Parser_INCLUDE_DIRS
  
)
 
if(NOT Http_Parser_FOUND)
  message(FATAL_ERROR "Could not find `Http_Parser` package. Debian: sudo apt update && sudo apt install libhttp-parser-dev MacOS: brew install http-parser Version: 2.8")
elseif(Http_Parser_version_result)
  if(Http_Parser_version_result EQUAL 0)
    if(Http_Parser_VERSION VERSION_LESS 2.8)
      message(FATAL_ERROR "Found but version is Http_Parser_VERSION. Could not find `Http_Parser` package. Debian: sudo apt update && sudo apt install libhttp-parser-dev MacOS: brew install http-parser Version: 2.8")
    endif()
  else()
    message(WARNING "Can not determine version. ${Http_Parser_version_error}")
  endif()
endif()

 
if (NOT TARGET Http_Parser)
  add_library(Http_Parser INTERFACE IMPORTED GLOBAL)
   target_include_directories(Http_Parser INTERFACE ${Http_Parser_INCLUDE_DIRS})
   target_link_libraries(Http_Parser INTERFACE ${Http_Parser_LIBRARIES})
  endif(NOT TARGET Http_Parser)