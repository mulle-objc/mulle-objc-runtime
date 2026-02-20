int  main(void) {
  __attribute__((mulle_confined_loop))
  do {
     __attribute__((mulle_confined_loop))
     do {
       __attribute__((mulle_confined_return))   // nested! fail!
       return( 0);
     } while(0);
  } while(0);
  return( 0);
}
