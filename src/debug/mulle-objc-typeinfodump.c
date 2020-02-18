//
//  mulle-objc.typeinfodump.c
//  mulle-objc-runtime
//
//  Created by Nat! on 18.02.20
//  Copyright (c) 2020 Nat! - Mulle kybernetiK.
//  Copyright (c) 2020 Codeon GmbH.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//

#include "mulle-objc-typeinfodump.h"


void   mulle_objc_typeinfo_dump_to_file( struct mulle_objc_typeinfo *info,
                                         char *indent,
                                         FILE *fp)
{
   fprintf( fp, "%stype=%.*s\n", indent, (int) (info->pure_type_end - info->type), info->type);
   fprintf( fp, "%sinvocation_offset=%d\n", indent, (int) info->invocation_offset);
   fprintf( fp, "%snatural_size=%u\n", indent, (unsigned int) info->natural_size);
   fprintf( fp, "%sbits_size=%u\n", indent, (unsigned int) info->bits_size);
   fprintf( fp, "%sbits_struct_alignment=%u\n", indent, (unsigned int) info->bits_struct_alignment);
   fprintf( fp, "%snatural_alignment=%u\n", indent, (unsigned int) info->natural_alignment);
   fprintf( fp, "%sn_members= %d\n", indent, info->n_members);
   fprintf( fp, "%shas_object= %d\n", indent, info->has_object);
}