#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>




static void  test( char *signature, size_t size, unsigned int align)
{
   struct mulle_objc_typeinfo    info;

   mulle_objc_signature_supply_typeinfo( signature, NULL, &info);

   printf( "\t(p) type              = %s\n",   signature);
   printf( "\tmember_type_start     = %s\n",   info.member_type_start ? info.member_type_start : "NULL");
 // printf( "\tinvocation_offset     = %td\n", (ptrdiff_t) info.invocation_offset);
   printf( "\tnatural_size          = %zd\n",  info.natural_size);
   printf( "\tbits_size             = %zd\n",  info.bits_size);
   printf( "\tbits_struct_alignment = %d\n",   (int) info.bits_struct_alignment);
   printf( "\tnatural_alignment     = %d\n",   (int) info.natural_alignment);
   printf( "\tn_members             = %ld\n",  (long) info.n_members);
   printf( "\thas_object            = %d\n",   info.has_object);
   printf( "\thas_retainable_type   = %d\n\n", info.has_retainable_type);

   if( info.natural_size != size)
      printf( "error: size diff: %zd vs expected %zd\n", info.natural_size, size);
   if( info.natural_alignment != align)
      printf( "error: align diff: %u vs expected %u\n", info.natural_alignment, align);
}


struct odd_members
{
   struct
   {
      char    a;
      double  b;
   } c[ 3];
};


struct key_value
{
   char    *key;
   id      value;
   struct
   {
      char  *keys[ 2];
      id    values[ 2];
   } inner;
};


int  main( void)
{
   test( @encode( struct odd_members), sizeof( struct odd_members), alignof( struct odd_members));
   test( @encode( struct key_value), sizeof( struct key_value), alignof( struct key_value));

   return( 0);
}

