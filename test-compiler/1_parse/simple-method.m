// this is just a compiler test to see if we can pick up _param->a and
// _param->b as a + b. This is just for the parse.

@implementation Bar

+ (int) a:(int) a
        b:(int) b
{
   return( a + b);
}

@end

