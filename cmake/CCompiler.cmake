if( MULLE_TRACE_INCLUDE)
   MESSAGE( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# these definitions are just used when compiling the
# runtime itself, with a regular C compiler
#
get_filename_component( C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME_WE)

# see below also for compiler specifica
if( NOT "${C_COMPILER_NAME}" MATCHES "mulle-cl*")
   add_definitions( -D__MULLE_OBJC_TPS__)     ## tagged pointers runtime (alternative: __MULLE_OBJC_NO_TPS__)
   add_definitions( -D__MULLE_OBJC_FCS__)     ## fast method calls (alternative: __MULLE_OBJC_NO_FCS__)
endif()

add_definitions( -DMULLE_C11_NO_NOOB_WARNINGS=1)
