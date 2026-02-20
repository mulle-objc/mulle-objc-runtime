
int main(void)
{
  // INCORRECT: After 'do' keyword - attribute is ignored or misapplied
  // no way to catch that though or ?
  do __attribute__((mulle_confined_loop))
  {
     return( 0);
  }
  while(0);  // No error - attribute doesn't work here
  return( 1);
}

