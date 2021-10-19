//
//  main.m
//  test-meta-abi
//
//  Created by Nat! on 31.10.15.
//  Copyright © 2015 Mulle kybernetiK. All rights reserved.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <limits.h>
#include <stdio.h>

struct small_struct
{
   intptr_t   value;
};

struct large_struct
{
   intptr_t   value1;
   intptr_t   value2;
};


struct _received
{
   char   c;
   short  s;
   int    i;
   long   l;
   long long ll;
   char   *cp;
   struct small_struct   ss;
   struct large_struct   ls;
} received[ 16];


@interface Foo
@end


#include "Foo.inc"


void  single_args()
{
   struct small_struct   ss;
   struct large_struct   ls;

   [Foo callChar:CHAR_MIN];
   printf( "-callChar:CHAR_MIN %s\n",  (received[ 0].c == CHAR_MIN)  ? "PASS" : "FAIL");
   [Foo callChar:CHAR_MAX];
   printf( "-callChar:CHAR_MAX %s\n",  (received[ 0].c == CHAR_MAX)  ? "PASS" : "FAIL");

   [Foo callShort:SHRT_MIN];
   printf( "-callShort:SHRT_MIN %s\n",  (received[ 0].s== SHRT_MIN)  ? "PASS" : "FAIL");
   [Foo callShort:SHRT_MAX];
   printf( "-callShort:SHRT_MAX %s\n",  (received[ 0].s== SHRT_MAX)  ? "PASS" : "FAIL");

   [Foo callInt:INT_MIN];
   printf( "-callShort:INT_MIN %s\n",  (received[ 0].i== INT_MIN)  ? "PASS" : "FAIL");
   [Foo callInt:INT_MAX];
   printf( "-callShort:INT_MAX %s\n",  (received[ 0].i== INT_MAX)  ? "PASS" : "FAIL");

   [Foo callLong:LONG_MIN];
   printf( "-callLong:LONG_MIN %s\n",  (received[ 0].l== LONG_MIN)  ? "PASS" : "FAIL");
   [Foo callLong:LONG_MAX];
   printf( "-callLong:LONG_MAX %s\n",  (received[ 0].l == LONG_MAX)  ? "PASS" : "FAIL");

   [Foo callLongLong:LLONG_MIN];
   printf( "-callLongLong:LLONG_MIN %s\n",  (received[ 0].ll == LLONG_MIN)  ? "PASS" : "FAIL");
   [Foo callLongLong:LLONG_MAX];
   printf( "-callLongLong:LLONG_MAX %s\n",  (received[ 0].ll == LLONG_MAX) ? "PASS" : "FAIL");

   [Foo callCharPtr:(char *) INTPTR_MIN];
   printf( "-callCharPtr:INTPTR_MIN %s\n",  (received[ 0].cp == (char *) INTPTR_MIN) ? "PASS" : "FAIL");
   [Foo callCharPtr:(char *) INTPTR_MAX];
   printf( "-callCharPtr:INTPTR_MAX %s\n",  (received[ 0].cp == (char *) INTPTR_MAX) ? "PASS" : "FAIL");

   ss.value = INTPTR_MIN;
   [Foo callSmallStruct:ss];
   printf( "-callSmallStruct:{ INTPTR_MIN } %s\n",  (received[ 0].ss.value == INTPTR_MIN) ? "PASS" : "FAIL");
   ss.value = INTPTR_MAX;
   [Foo callSmallStruct:ss];
   printf( "-callSmallStruct:{ INTPTR_MAX } %s\n",  (received[ 0].ss.value == INTPTR_MAX) ? "PASS" : "FAIL");

   ls.value1 = INTPTR_MIN;
   ls.value2 = INTPTR_MAX;
   [Foo callLargeStruct:ls];
   printf( "-callLargeStruct:{ INTPTR_MIN, INTPTR_MAX } %s\n",
      (received[ 0].ls.value1 == INTPTR_MIN && received[ 0].ls.value2 == INTPTR_MAX) ? "PASS" : "FAIL");
   ls.value1 = INTPTR_MAX;
   ls.value2 = INTPTR_MIN;
   [Foo callLargeStruct:ls];
   printf( "-callLargeStruct:{ INTPTR_MAX, INTPTR_MIN } %s\n",
      (received[ 0].ls.value1 == INTPTR_MAX && received[ 0].ls.value2 == INTPTR_MIN) ? "PASS" : "FAIL");

}


static void  double_args()
{
   char   *s;

   [Foo callChar:CHAR_MIN longLong:LLONG_MIN];
   printf( "-callChar:CHAR_MIN longLong:LLONG_MIN %s\n", (received[ 0].c == CHAR_MIN && received[ 1].ll == LLONG_MIN) ? "PASS" : "FAIL");
   [Foo callChar:CHAR_MIN longLong:LLONG_MAX];
   printf( "-callChar:CHAR_MIN longLong:LLONG_MAX %s\n", (received[ 0].c == CHAR_MIN && received[ 1].ll == LLONG_MAX) ? "PASS" : "FAIL");

   [Foo callChar:CHAR_MAX longLong:LLONG_MIN];
   printf( "-callChar:CHAR_MAX longLong:LLONG_MIN %s\n", (received[ 0].c == CHAR_MAX && received[ 1].ll == LLONG_MIN) ? "PASS" : "FAIL");
   [Foo callChar:CHAR_MAX longLong:LLONG_MAX];
   printf( "-callChar:CHAR_MAX longLong:LLONG_MAX %s\n", (received[ 0].c == CHAR_MAX && received[ 1].ll == LLONG_MAX) ? "PASS" : "FAIL");

   s = "VfL Bochum 1848";
   [Foo callCharPtr:s longLong:LLONG_MIN];
   printf( "-callCharPtr:\"%s\" longLong:LLONG_MIN %s\n", s, (received[ 0].cp == s && received[ 1].ll == LLONG_MIN) ? "PASS" : "FAIL");
   [Foo callCharPtr:s longLong:LLONG_MAX];
   printf( "-callCharPtr:\"%s\" longLong:LLONG_MAX %s\n", s, (received[ 0].cp == s && received[ 1].ll == LLONG_MAX) ? "PASS" : "FAIL");
}


static void  fat_args()
{
   unsigned int   i;

   [Foo callLongLong:1
            longLong:2
            longLong:3
            longLong:4
            longLong:5
            longLong:6
            longLong:7
            longLong:8
            longLong:9
            longLong:10
            longLong:11
            longLong:12
            longLong:13
            longLong:14
            longLong:15
            longLong:16];

   for( i = 0; i < 16; i++)
      printf( "-callLongLong:... #%u %s\n",  i, (received[ i].ll == i + 1) ? "PASS" : "FAIL");
}


int main(int argc, const char * argv[])
{
   single_args();
   double_args();
   fat_args();
   return( 0);
}



