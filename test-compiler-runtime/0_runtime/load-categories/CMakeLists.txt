
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


add_executable( load-categories
${SOURCES}
)


# needed to do it like this, because gcc is old
# and cmake prepends the flags and gcc then
# has nothing to link against yet

target_link_libraries( load-categories
${TEST_LIBRARIES}
)

