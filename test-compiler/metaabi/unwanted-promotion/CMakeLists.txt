cmake_minimum_required( VERSION 3.13...99.99)

set(CMAKE_C_COMPILER_WORKS 1)

project( unwanted-promotion C)


# make the order inconvenient
set( SOURCES
main.m
functions.c
)


add_executable( unwanted-promotion.exe
${SOURCES}
)


# needed to do it like this, because gcc is old
# and cmake prepends the flags and gcc then
# has nothing to link against yet
get_filename_component( C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME_WE)

if( NOT "${C_COMPILER_NAME}" MATCHES "mulle-cl*")
	add_definitions( -D__MULLE_OBJC_TPS__)     ## tagged pointers runtime (alternative: __MULLE_OBJC_NO_TPS__)
	add_definitions( -D__MULLE_OBJC_FCS__)     ## fast method calls (alternative: __MULLE_OBJC_NO_FMC__)
   message( STATUS "TPS enabled via macro")
   message( STATUS "FCS enabled via macro")

   if( CMAKE_BUILD_TYPE MATCHES "^Debug|^Test")
      add_definitions( -D__MULLE_OBJC_TAO__)  ## thread affine object (alternative: __MULLE_OBJC_NO_TAO__)
      message( STATUS "TAO enabled via macro")
   else()
      add_definitions( -D__MULLE_OBJC_NO_TAO__)  ## thread affine object (alternative: __MULLE_OBJC_NO_TAO__)
      message( STATUS "TAO disabled via macro")
   endif()
else()
   target_compile_options( "unwanted-promotion.exe" PRIVATE -ObjC)
   if( CMAKE_BUILD_TYPE MATCHES "^Debug|^Test")
      target_compile_options( "unwanted-promotion.exe" PRIVATE -fobjc-tao)
      message( STATUS "TAO enabled via compiler flag")
   endif()
endif()


target_link_libraries( unwanted-promotion.exe
   ${TEST_LIBRARIES}
)


