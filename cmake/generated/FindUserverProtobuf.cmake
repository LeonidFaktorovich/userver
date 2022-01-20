# AUTOGENERATED, DON'T CHANGE THIS FILE!

 
              
 
 

include(FindPackageHandleStandardArgs)

find_library(UserverProtobuf_LIBRARIES_protobuf NAMES
  protobuf
  )
list(APPEND UserverProtobuf_LIBRARIES
  ${UserverProtobuf_LIBRARIES_protobuf}
)
  
find_path(UserverProtobuf_INCLUDE_DIRS_google_protobuf_port_def_inc_google_protobuf_compiler_cpp_cpp_generator_h NAMES
  google/protobuf/port_def.inc google/protobuf/compiler/cpp/cpp_generator.h
  )
list(APPEND UserverProtobuf_INCLUDE_DIRS
  ${UserverProtobuf_INCLUDE_DIRS_google_protobuf_port_def_inc_google_protobuf_compiler_cpp_cpp_generator_h}
)
  
 

 
 
find_package_handle_standard_args(
  UserverProtobuf
    REQUIRED_VARS
      UserverProtobuf_LIBRARIES
      UserverProtobuf_INCLUDE_DIRS
      
    FAIL_MESSAGE
      "Could not find `UserverProtobuf` package. Debian: sudo apt update && sudo apt install protoc-dev protobuf-compiler python3-protobuf libprotoc-dev MacOS: brew install protobuf"
)
mark_as_advanced(
  UserverProtobuf_LIBRARIES
  UserverProtobuf_INCLUDE_DIRS
  
)
 
if(NOT UserverProtobuf_FOUND)
  message(FATAL_ERROR "Could not find `UserverProtobuf` package. Debian: sudo apt update && sudo apt install protoc-dev protobuf-compiler python3-protobuf libprotoc-dev MacOS: brew install protobuf")
elseif(UserverProtobuf_version_result)
  if(UserverProtobuf_version_result EQUAL 0)
    if(UserverProtobuf_VERSION VERSION_LESS None)
      message(FATAL_ERROR "Found but version is UserverProtobuf_VERSION. Could not find `UserverProtobuf` package. Debian: sudo apt update && sudo apt install protoc-dev protobuf-compiler python3-protobuf libprotoc-dev MacOS: brew install protobuf")
    endif()
  else()
    message(WARNING "Can not determine version. ${UserverProtobuf_version_error}")
  endif()
endif()

 
if (NOT TARGET UserverProtobuf)
  add_library(UserverProtobuf INTERFACE IMPORTED GLOBAL)
   target_include_directories(UserverProtobuf INTERFACE ${UserverProtobuf_INCLUDE_DIRS})
   target_link_libraries(UserverProtobuf INTERFACE ${UserverProtobuf_LIBRARIES})
  endif(NOT TARGET UserverProtobuf)