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

add_executable( inheritance
${SOURCES}
)
