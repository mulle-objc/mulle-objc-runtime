### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.


# Original:
#
# https://github.com/ros2/rosidl/blob/master/rosidl_cmake/cmake/string_camel_case_to_lower_case_underscore.cmake
# Apache License, Version 2.
#
if( NOT __STRING_CASE__CMAKE__)
   set( __STRING_CASE__CMAKE__ ON)
   function( snakeCaseString str var)
     # convert this to one word If present
     string( REGEX REPLACE "ObjC" "Objc" value "${str}")

     # turns mulle-scion into MULLE__SCION to distinguish from
     # MulleScion -> MULLE_SCION
     string( REGEX REPLACE "-" "__" value "${value}")

     # insert an underscore before any upper case letter
     # which is not followed by another upper case letter
     string( REGEX REPLACE "(.)([A-Z][a-z]+)" "\\1_\\2" value "${value}")

     # insert an underscore before any upper case letter
     # which is preseded by a lower case letter or number
     string( REGEX REPLACE "([a-z0-9])([A-Z])" "\\1_\\2" value "${value}")

     # replace non-identifier characters with an '_', for UTF8 this
     # may produce multiple _s we can't get easily rid off
     string( REGEX REPLACE "[^A-Za-z0-9$_]" "_" value "${value}")

     set( ${var} "${value}" PARENT_SCOPE)
   endfunction()
endif()
