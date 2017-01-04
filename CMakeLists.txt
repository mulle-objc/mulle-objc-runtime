cmake_minimum_required (VERSION 3.0)

project (mulle-objc)

# // add -D__MULLE_OBJC_TRT__ for thread local runtime
add_definitions( -DMULLE_ALLOCATOR_EXTERN_GLOBAL=extern
                 -DMULLE_OBJC_EXTERN_GLOBAL=extern
                 -D__MULLE_OBJC_TPS__  ## tagged pointers runtime (default)
#                -D__MULLE_OBJC_TRT__  ## thread local runtime 
                 )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/mulle-configuration)
message(STATUS "CMAKE_MODULE_PATH is ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  include( Debug)
else()
  include( Release)
endif()


include_directories(
src
)

set(DEBUG_HEADERS
src/debug/mulle_objc_runtime_dump_graphviz.h
src/debug/mulle_objc_runtime_dump_html.h
)

set(DEBUG_SOURCES
src/debug/mulle_objc_html.c
src/debug/mulle_objc_runtime_dump_html.c
src/debug/mulle_objc_runtime_dump_graphviz.c
)


set(HEADERS
src/mulle_objc.h
src/mulle_objc_atomicpointer.h
src/mulle_objc_builtin.h
src/mulle_objc_cache.h
src/mulle_objc_call.h
src/mulle_objc_callqueue.h
src/mulle_objc_class.h
src/mulle_objc_class_convenience.h
src/mulle_objc_class_runtime.h
src/mulle_objc_fastclasstable.h
src/mulle_objc_fastenumeration.h
src/mulle_objc_fastmethodtable.h
src/mulle_objc_ivar.h
src/mulle_objc_ivarlist.h
src/mulle_objc_kvccache.h
src/mulle_objc_load.h
src/mulle_objc_fnv1.h
src/mulle_objc_method.h
src/mulle_objc_methodlist.h
src/mulle_objc_object.h
src/mulle_objc_object_convenience.h
src/mulle_objc_objectheader.h
src/mulle_objc_property.h
src/mulle_objc_propertylist.h
src/mulle_objc_retain_release.h
src/mulle_objc_runtime.h
src/mulle_objc_runtime_struct.h
src/mulle_objc_runtime_global.h
src/mulle_objc_signature.h
src/mulle_objc_taggedpointer.h
src/mulle_objc_try_catch_finally.h
src/mulle_objc_uniqueid.h
src/mulle_objc_version.h
${DEBUG_HEADERS}
)


set(SOURCES
src/mulle_objc_cache.c
src/mulle_objc_call.c
src/mulle_objc_callqueue.c
src/mulle_objc_class.c
src/mulle_objc_class_runtime.c
src/mulle_objc_fastenumeration.c
src/mulle_objc_fastmethodtable.c
src/mulle_objc_ivar.c
src/mulle_objc_ivarlist.c
src/mulle_objc_load.c
src/mulle_objc_fnv1.c
src/mulle_objc_kvccache.c
src/mulle_objc_method.c
src/mulle_objc_methodlist.c
src/mulle_objc_property.c
src/mulle_objc_propertylist.c
src/mulle_objc_retain_release.c
src/mulle_objc_try_catch_finally.c
src/mulle_objc_runtime.c
src/mulle_objc_runtime_global.c
src/mulle_objc_signature.c
src/mulle_objc_uniqueid.c
)


add_library(mulle_objc STATIC
${SOURCES}
${DEBUG_SOURCES}
)

INSTALL( TARGETS mulle_objc DESTINATION "lib")
INSTALL( FILES ${HEADERS} DESTINATION "include/mulle_objc")


#
# standalone
#
if( MSVC)
   set( DEF_FILE mulle_objc_standalone.def)
   set_source_files_properties( ${DEF_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
   set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
   set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEF:${DEF_FILE}")
   # /NODEFAULTLIB:MSVCRT")
endif()

add_library(mulle_objc_standalone SHARED
src/mulle_objc_standalone.c
${DEF_FILE}
)

add_dependencies( mulle_objc_standalone mulle_objc)

find_library( MULLE_ABA_LIBRARY            NAMES mulle_aba)
find_library( MULLE_ALLOCATOR_LIBRARY      NAMES mulle_allocator)
find_library( MULLE_CONCURRENT_LIBRARY     NAMES mulle_concurrent)
find_library( MULLE_TEST_ALLOCATOR_LIBRARY NAMES mulle_test_allocator)
find_library( MULLE_THREAD_LIBRARY         NAMES mulle_thread)

message(STATUS "MULLE_ABA_LIBRARY is ${MULLE_ABA_LIBRARY}")
message(STATUS "MULLE_ALLOCATOR_LIBRARY is ${MULLE_ALLOCATOR_LIBRARY}")
message(STATUS "MULLE_CONCURRENT_LIBRARY is ${MULLE_CONCURRENT_LIBRARY}")
message(STATUS "MULLE_TEST_ALLOCATOR_LIBRARY is ${MULLE_TEST_ALLOCATOR_LIBRARY}")
message(STATUS "MULLE_THREAD_LIBRARY is ${MULLE_THREAD_LIBRARY}")

set( DEPENDENCY_LIBRARIES
${MULLE_ABA_LIBRARY}
${MULLE_ALLOCATOR_LIBRARY}
${MULLE_CONCURRENT_LIBRARY}
${MULLE_TEST_ALLOCATOR_LIBRARY}
${MULLE_THREAD_LIBRARY}
)


if( MSVC)
   add_custom_command( OUTPUT ${DEF_FILE}
                       COMMAND mulle-mingw-dumpdef.bat -o "${DEF_FILE}" --name "mulle_objc_standalone" --prefix "mulle" ${DEPENDENCY_LIBRARIES} $<TARGET_FILE:mulle_objc>
                       DEPENDS "${DEPENDENCY_LIBRARIES}" $<TARGET_FILE:mulle_objc>
                       VERBATIM)
endif()


TARGET_LINK_LIBRARIES( mulle_objc_standalone
${BEGIN_ALL_LOAD}
mulle_objc
${DEPENDENCY_LIBRARIES}
${END_ALL_LOAD}
)

INSTALL( TARGETS mulle_objc_standalone DESTINATION "lib")


#
# Doesn't work, don't know why
#
# add_executable( mulle-objc-uniqueid
# src/mulle-objc-uniqueid/main.c
# mulle_objc
# ${DEPENDENCY_LIBRARIES}
#)
#
#add_dependencies( mulle-objc-uniqueid mulle_objc)
#set_target_properties( mulle-objc-uniqueid PROPERTIES LINKER_LANGUAGE C)
#
#INSTALL( TARGETS mulle-objc-uniqueid DESTINATION "bin")
