cmake_minimum_required(VERSION 3.15)


set(CMAKE_C_COMPILER_WORKS 1)

project( load-alias C)

#
# Consumer+Cat.m is listed AFTER Consumer.m on purpose: the category (which
# supplies the alias target +baseValue) must load after the aliasing class.
#
set( SOURCES
Base.m
Consumer.m
Consumer+Cat.m
main.m)

if( DEPENDENCY_DIR)
   include_directories( ${DEPENDENCY_DIR}/include)
endif()
if( ADDICTION_DIR)
   include_directories( ${ADDICTION_DIR}/include)
endif()


add_executable( load-alias.exe
${SOURCES}
)


get_filename_component( C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME_WE)

if( CMAKE_BUILD_TYPE MATCHES "^Debug|^Test")
   target_compile_options( "load-alias.exe" PRIVATE -fobjc-tao)
   message( STATUS "TAO enabled via compiler flag")
endif()

target_link_libraries( load-alias.exe
${TEST_LIBRARIES}
)

# Export symbols needed for Windows DLL dynamic linking
if( WIN32)
   target_link_options( load-alias.exe
      PUBLIC
         "-Wl,--export-all-symbols"
   )
endif()
