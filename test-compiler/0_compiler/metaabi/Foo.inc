@implementation Foo

+ (void) callChar:(char) v           { received[ 0].c = v;  }
+ (void) callShort:(short) v         { received[ 0].s = v;  }
+ (void) callInt:(int) v             { received[ 0].i = v;  }
+ (void) callLong:(long) v           { received[ 0].l = v;  }
+ (void) callLongLong:(long long) v  { received[ 0].ll = v; }
+ (void) callCharPtr:(char *) v      { received[ 0].cp = v; }
+ (void) callSmallStruct:(struct small_struct) v   { received[ 0].ss = v; }
+ (void) callLargeStruct:(struct large_struct) v   { received[ 0].ls = v; }

@end



@implementation Foo ( DoubleArgs)

+ (void) callChar:(char) v  longlong:(long long) v2
{
   received[ 0].c   = v;
   received[ 1].ll = v2;
}


+ (void) callLongLong:(long long) v char:(char) v2
{
   received[ 0].ll = v;
   received[ 1].c = v2;
}


+ (void) callCharPtr:(char *) v  longlong:(long long) v2
{
   received[ 0].cp  = v;
   received[ 1].ll = v2;
}

@end


@implementation Foo ( FatArgs)

+ (void) callLongLong:(char) v1
             longlong:(long long) v2
             longlong:(long long) v3
             longlong:(long long) v4
             longlong:(long long) v5
             longlong:(long long) v6
             longlong:(long long) v7
             longlong:(long long) v8
             longlong:(long long) v9
             longlong:(long long) v10
             longlong:(long long) v11
             longlong:(long long) v12
             longlong:(long long) v13
             longlong:(long long) v14
             longlong:(long long) v15
             longlong:(long long) v16
{
   received[ 0].ll = v1;
   received[ 1].ll = v2;
   received[ 2].ll = v3;
   received[ 3].ll = v4;
   received[ 4].ll = v5;
   received[ 5].ll = v6;
   received[ 6].ll = v7;
   received[ 7].ll = v8;
   received[ 8].ll = v9;
   received[ 9].ll = v10;
   received[10].ll = v11;
   received[11].ll = v12;
   received[12].ll = v13;
   received[13].ll = v14;
   received[14].ll = v15;
   received[15].ll = v16;
}

@end
