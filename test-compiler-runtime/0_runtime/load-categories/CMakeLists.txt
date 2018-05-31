
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

if( DEPENDENCIES_DIR)
   include_directories( ${DEPENDENCIES_DIR}/include)
endif()
if( ADDICTIONS_DIR)
   include_directories( ${ADDICTIONS_DIR}/include)
endif()


add_executable( load-categories
${SOURCES}
)


# needed to do it like this, because gcc is old
# and cmake prepends the flags and gcc then
# has nothing to link against yet

target_link_libraries( load-categories
${TEST_LIBRARIES}
)

