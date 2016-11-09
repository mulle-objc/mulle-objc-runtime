#! /bin/sh


x=0
while [ "$x" -lt 1000 ]
do
   method_id="`mulle-objc-uniqueid "f$x"`"

   cat <<EOF1
void   *f$x( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   mulle_objc_methodid_t   sel;

   sel = MULLE_OBJC_METHODID( 0x$method_id);  // @selector( f$x)
   assert( _cmd == sel);
   return( (void *) (uintptr_t) sel);
}
EOF1
   x=`expr $x + 1`
done


cat <<EOF2

mulle_objc_methodimplementation_t    f[] =
{
EOF2


x=0
while [ "$x" -lt 999 ]
do
   echo "f$x,"
   x=`expr $x + 1`
done

   echo "f999"


cat <<EOF3
};
EOF3

