
# make the order inconvenient
set( SOURCES
Root.m
Foo3.m
ProtoClass2.m
Foo1.m
Base.m
ProtoClass1.m
Foo2.m
main.m)


if( DEPENDENCIES_DIR)
   include_directories( ${DEPENDENCIES_DIR}/include)
endif()
if( ADDICTIONS_DIR)
   include_directories( ${ADDICTIONS_DIR}/include)
endif()

add_executable( load-classes
${SOURCES}
)

# needed to do it like this, because gcc is old
# and cmake prepends the flags and gcc then
# has nothing to link against yet

target_link_libraries( load-classes
${TEST_LIBRARIES}
)


message( "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
message( "CMAKE_C_FLAGS_DEBUG = ${CMAKE_C_FLAGS_DEBUG}")
message( "CMAKE_C_FLAGS_RELEASE = ${CMAKE_C_FLAGS_RELEASE}")
message( "ADDICTIONS_DIR = ${ADDICTIONS_DIR}")
message( "DEPENDENCIES_DIR = ${DEPENDENCIES_DIR}")
