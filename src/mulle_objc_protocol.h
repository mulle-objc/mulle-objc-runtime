//
//  mulle_objc_protocol.h
//  mulle-objc
//
//  Created by Nat! on 28.02.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//

#ifndef mulle_objc_protocol_h__
#define mulle_objc_protocol_h__

#include "mulle_objc_uniqueid.h"


static inline mulle_objc_protocolid_t   mulle_objc_protocolid_from_string( char *s)
{
   return( mulle_objc_uniqueid_from_string( s));
}

#endif
