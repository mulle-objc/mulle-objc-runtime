#
# Use test script or call it like this
# CFLAGS="..." OUTPUT=foo make
#
OUTPUT ?= foo

SOURCES=main.c \
test_category_inheritance.c \
test_class_simple.c \
test_message_forwarding.c \
test_message_sending.c \
test_method.c \
test_protocol_inheritance.c \
test_retain_release.c \
test_simple_inheritance.c

OBJECTS=$(SOURCES:.c=.o)

all:	$(OUTPUT)

$(OUTPUT):	 $(OBJECTS)
		cc $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)
