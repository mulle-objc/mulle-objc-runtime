
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


if( DEPENDENCY_DIR)
   include_directories( ${DEPENDENCY_DIR}/include)
endif()
if( ADDICTION_DIR)
   include_directories( ${ADDICTION_DIR}/include)
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
message( "ADDICTION_DIR = ${ADDICTION_DIR}")
message( "DEPENDENCY_DIR = ${DEPENDENCY_DIR}")
