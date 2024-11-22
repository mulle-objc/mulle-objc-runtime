cmake_minimum_required(VERSION 3.15)


set(CMAKE_C_COMPILER_WORKS 1)

project( load-categories C)

# make the order inconvenient
set( SOURCES
Base+C1.m
Base+C2.m
ProtoClass1.m
Base.m
Foo+C1.m
Foo+C2.m
Foo.m
Root.m
Foo+C3.m
ProtoClass2.m
main.m)

if( DEPENDENCY_DIR)
   include_directories( ${DEPENDENCY_DIR}/include)
endif()
if( ADDICTION_DIR)
   include_directories( ${ADDICTION_DIR}/include)
endif()


add_executable( load-categories.exe
${SOURCES}
)


# needed to do it like this, because gcc is old
# and cmake prepends the flags and gcc then
# has nothing to link against yet
get_filename_component( C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME_WE)

if( "${C_COMPILER_NAME}" MATCHES "mulle-cl*")
   if( CMAKE_BUILD_TYPE MATCHES "^Debug|^Test")
      target_compile_options( "load-categories.exe" PRIVATE -fobjc-tao)
      message( STATUS "TAO enabled via compiler flag")
   endif()
endif()

#
# needed to do it like this, because gcc is old
# and cmake prepends the flags and gcc then
# has nothing to link against yet
#
target_link_libraries( load-categories.exe
${TEST_LIBRARIES}
)

