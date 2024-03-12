# This file will be regenerated by `mulle-match-to-cmake` via
# `mulle-sde reflect` and any edits will be lost.
#
# This file will be included by cmake/share/sources.cmake
#
if( MULLE_TRACE_INCLUDE)
   MESSAGE( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# contents selected with patternfile ??-source--signature-sources
#
set( SIGNATURE_SOURCES
src/mulle-objc-signature/main.c
)

#
# contents selected with patternfile ??-source--sources
#
set( SOURCES
src/mulle-objc-cache.c
src/mulle-objc-call.c
src/mulle-objc-callqueue.c
src/mulle-objc-class.c
src/mulle-objc-class-initialize.c
src/mulle-objc-class-lookup.c
src/mulle-objc-classpair.c
src/mulle-objc-class-search.c
src/mulle-objc-csvdump.c
src/mulle-objc-fastenumeration.c
src/mulle-objc-fastmethodtable.c
src/mulle-objc-impcache.c
src/mulle-objc-infraclass.c
src/mulle-objc-infraclass-reuse.c
src/mulle-objc-ivar.c
src/mulle-objc-ivarlist.c
src/mulle-objc-kvccache.c
src/mulle-objc-load.c
src/mulle-objc-loadinfo.c
src/mulle-objc-metaclass.c
src/mulle-objc-method.c
src/mulle-objc-methodlist.c
src/mulle-objc-property.c
src/mulle-objc-propertylist.c
src/mulle-objc-protocol.c
src/mulle-objc-protocollist.c
src/mulle-objc-retain-release.c
src/mulle-objc-signature.c
src/mulle-objc-super.c
src/mulle-objc-symbolizer.c
src/mulle-objc-try-catch-finally.c
src/mulle-objc-uniqueidarray.c
src/mulle-objc-uniqueid.c
src/mulle-objc-universe.c
src/mulle-objc-universe-class.c
src/mulle-objc-universe-exception.c
src/mulle-objc-universe-fail.c
src/mulle-objc-universe-global.c
)

#
# contents selected with patternfile ??-source--standalone-sources
#
set( STANDALONE_SOURCES
src/mulle-objc-runtime-standalone.c
)

#
# contents selected with patternfile ??-source--uniqueid-sources
#
set( UNIQUEID_SOURCES
src/mulle-objc-uniqueid/main.c
)
