if( MULLE_TRACE_INCLUDE)
   MESSAGE( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# these definitions are just used when compiling the
# runtime itself, with a regular C compiler
#
get_filename_component( C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME_WE)
message( STATUS "C_COMPILER_NAME is ${C_COMPILER_NAME}")

message( STATUS "Objective-C TPS enabled")
add_definitions( -D__MULLE_OBJC_TPS__)     ## tagged pointers runtime (alternative: __MULLE_OBJC_NO_TPS__)
message( STATUS "Objective-C FCS enabled")
add_definitions( -D__MULLE_OBJC_FCS__)     ## fast method calls (alternative: __MULLE_OBJC_NO_FCS__)
if( CMAKE_BUILD_TYPE MATCHES "^Debug")
   message( STATUS "Objective-C TAO enabled")
   add_definitions( -D__MULLE_OBJC_TAO__)  ## thread affine object (alternative: __MULLE_OBJC_NO_TAO__)
else()
   message( STATUS "Objective-C TAO disabled")
   add_definitions( -D__MULLE_OBJC_NO_TAO__)  ## thread affine object (alternative: __MULLE_OBJC_NO_TAO__)
endif()
## see below also for compiler specifica
#if( NOT "${C_COMPILER_NAME}" MATCHES "mulle-cl*")
#else()
#   if( MSVC)
#      add_compile_options( /clang:-ObjC)
#   else()
#      add_compile_options( -x objective-c)
#   endif()
#endif()
#
#
#
#add_definitions( -DMULLE_C11_NO_NOOB_WARNINGS=1)
#