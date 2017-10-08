cmake_minimum_required( VERSION 3.0)

project( mulle-objc-runtime C)




set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/mulle-configuration/cmake)
message(STATUS "CMAKE_MODULE_PATH is ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")

if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  include( Debug)
else()
  include( Release)
endif()

set( MULLE_OBJC_RUNTIME_LIBRARY ON)
set( MULLE_OBJC_RUNTIME_STARTUP_LIBRARY ON)

include( CMakeSourcesAndHeaders.txt)
include( CMakeDependencies.txt)

include_directories( ${INCLUDE_DIRS})

#
# these definitions are just used when compiling the
# runtime itself, with a regular C compiler
#
add_definitions( -D__MULLE_OBJC_TPS__)     ## tagged pointers runtime (default: on)
add_definitions( -D__MULLE_OBJC_NO_TRT__)  ## thread local runtime (default: off)
add_definitions( -D__MULLE_OBJC_FMC__)     ## fast method calls (default: on)
add_definitions( -DMULLE_C11_NO_NOOB_WARNINGS=1)


add_library( mulle_objc_runtime STATIC
${SOURCES}
${DEBUG_SOURCES}
${C_DEPENDENCY_LIBRARIES}
)

target_compile_definitions( mulle_objc_runtime
PRIVATE ${MULLE_OBJC_RUNTIME_DEFINITIONS}
)


add_library( mulle_objc_runtime_startup STATIC
${STARTUP_SOURCES}
)

add_executable( mulle-objc-uniqueid
src/mulle-objc-uniqueid/main.c
)


target_compile_definitions( mulle-objc-uniqueid
PRIVATE ${MULLE_OBJC_RUNTIME_DEFINITIONS}
)


add_dependencies( mulle-objc-uniqueid
   mulle_objc_runtime
)


target_link_libraries( mulle-objc-uniqueid
$<TARGET_FILE:mulle_objc_runtime>
${C_DEPENDENCY_LIBRARIES}
${OS_SPECIFIC_LIBRARIES}
)


set( STANDALONE_BASE_NAME "mulle_objc_runtime")
set( STANDALONE_NAME "mulle_objc_runtime_standalone")
set( STANDALONE_DEFINITIONS ${MULLE_OBJC_RUNTIME_DEFINITIONS})
set( STANDALONE_DEPENDENCIES
   mulle_objc_runtime
   mulle_objc_runtime_startup
)

set( STANDALONE_ALL_LOAD_LIBRARIES
$<TARGET_FILE:mulle_objc_runtime>
${C_DEPENDENCY_LIBRARIES}
$<TARGET_FILE:mulle_objc_runtime_startup>
${OS_SPECIFIC_LIBRARIES}
)

include( Standalone)


install( TARGETS mulle_objc_runtime mulle_objc_runtime_startup mulle_objc_runtime_standalone DESTINATION "lib")
install( TARGETS mulle-objc-uniqueid DESTINATION "bin")
install( FILES ${PUBLIC_HEADERS} DESTINATION "include/mulle_objc_runtime")
install( FILES CMakeDependencies.txt DESTINATION "include/mulle_objc_runtime")

include( MulleTest)
