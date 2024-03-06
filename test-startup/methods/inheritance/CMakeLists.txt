cmake_minimum_required( VERSION 3.14)

project( inheritance C)

# make the order inconvenient
set( SOURCES
main.c
test_category_inheritance.c
test_class_simple.c
test_message_forwarding.c
test_message_sending.c
test_method.c
test_protocol_inheritance.c
test_retain_release.c
test_simple_inheritance.c
)


add_executable( inheritance.exe
${SOURCES}
)


# needed to do it like this, because gcc is old
# and cmake prepends the flags and gcc then
# has nothing to link against yet
get_filename_component( C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME_WE)

if( NOT "${C_COMPILER_NAME}" MATCHES "mulle-cl*")
	add_definitions( -D__MULLE_OBJC_TPS__)     ## tagged pointers runtime (alternative: __MULLE_OBJC_NO_TPS__)
	add_definitions( -D__MULLE_OBJC_FCS__)     ## fast method calls (alternative: __MULLE_OBJC_NO_FMC__)
   if( CMAKE_BUILD_TYPE MATCHES "^Debug")
      add_definitions( -D__MULLE_OBJC_TAO__)  ## thread affine object (alternative: __MULLE_OBJC_NO_TAO__)
   else()
      add_definitions( -D__MULLE_OBJC_NO_TAO__)  ## thread affine object (alternative: __MULLE_OBJC_NO_TAO__)
   endif()
endif()

if( "${C_COMPILER_NAME}" MATCHES "mulle-cl*")
   target_compile_options( "inheritance.exe" PRIVATE -ObjC)
endif()


target_link_libraries( inheritance.exe
   ${TEST_LIBRARIES}
)


