#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Bar
{
   double   foo[ 6];
}

+ (instancetype) alloc;
- (void) dealloc;

@end



@implementation Bar

+ (Class) class 
{
   return( self);
}


+ (instancetype) alloc
{
#ifdef __MULLE_OBJC__
   return( mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) self));
#else
   return( class_createInstance( self, 0));
#endif
}

- (void) dealloc
{
#ifdef __MULLE_OBJC__
   mulle_objc_instance_free( self);
#else
   object_dispose( self);
#endif
}

@end


MULLE_C_NEVER_INLINE
static void   *alloc_nada( Class cls)
{
   return( 0);
}


MULLE_C_NEVER_INLINE
static void   free_nada( void *self)
{
}



#define N_OBJECTS  0x8000
#define N_LOOPS    0x1000 

static void  run_loops( unsigned int n)
{
   static id      objects[ N_OBJECTS];
   unsigned int   i;
   unsigned int   j;
   Class          BarClass;

   BarClass = [Bar class];
   for( j = 0; j < n; j++) 
   {
      for( i = 0; i < N_OBJECTS; i++)
         objects[ i] = mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) BarClass);
      for( i = 0; i < N_OBJECTS; i++)
         mulle_objc_instance_free( objects[ i]); 
   }
}


static void  run_nada( unsigned int n)
{
   static id      objects[ N_OBJECTS];
   unsigned int   i;
   unsigned int   j;
   Class          BarClass;

   BarClass = [Bar class];
   for( j = 0; j < n; j++) 
   {
      for( i = 0; i < N_OBJECTS; i++)
         objects[ i] = alloc_nada( BarClass);
      for( i = 0; i < N_OBJECTS; i++)
         free_nada( objects[ i]); 
   }
}



//
// memo: be sure to compile everything with --no-mulle-test-define and -O3
//       Change MULLE_OBJC_CLASS_REUSE_ALLOC in mulle-objc-class-convenience.h
//       and see differences
//
int   main( void)
{
   mulle_absolutetime_t   start, end;

   run_loops( 1);       // warmup 

//   start = mulle_absolutetime_now();
//   run_nada( N_LOOPS);
//   end   = mulle_absolutetime_now();
//
//   fprintf( stderr, "nada: elapsed: %.5gs\n", end - start);

#if MULLE_OBJC_CLASS_REUSE_ALLOC
   fprintf( stderr, "Infraclass reuses allocs\n");
#else      
   fprintf( stderr, "Infraclass always allocates and frees\n");
#endif   

   start = mulle_absolutetime_now();
   run_loops( N_LOOPS);
   end   = mulle_absolutetime_now();

   fprintf( stderr, "elapsed: %.5gs\n", end - start);

   return( 0);
}

// mulle-sde test clean all &&
// mulle-sde test --no-mulle-test-define craft --release &&
// mulle-sde test --no-mulle-test-define run --release class-reuse.m

//
// perf stat -d -d ./class-reuse.exe
//
// Infraclass reuses allocs
// elapsed: 1.3925s
//
//  Performance counter stats for './class-reuse.exe':
//
//           1.395,89 msec task-clock                #    1,000 CPUs utilized
//                  2      context-switches          #    1,433 /sec
//                  0      cpu-migrations            #    0,000 /sec
//                829      page-faults               #  593,885 /sec
//      5.654.745.133      cycles                    #    4,051 GHz                      (35,53%)
//          2.256.195      stalled-cycles-frontend   #    0,04% frontend cycles idle     (35,53%)
//        775.654.379      stalled-cycles-backend    #   13,72% backend cycles idle      (35,53%)
//     19.282.935.953      instructions              #    3,41  insn per cycle
//                                                   #    0,04  stalled cycles per insn  (35,53%)
//      4.786.058.951      branches                  #    3,429 G/sec                    (35,77%)
//             29.041      branch-misses             #    0,00% of all branches          (35,82%)
//      8.150.838.803      L1-dcache-loads           #    5,839 G/sec                    (35,82%)
//        351.667.856      L1-dcache-load-misses     #    4,31% of all L1-dcache accesses  (35,82%)
//    <not supported>      LLC-loads
//    <not supported>      LLC-load-misses
//         29.521.871      L1-icache-loads           #   21,149 M/sec                    (35,82%)
//            331.054      L1-icache-load-misses     #    1,12% of all L1-icache accesses  (35,82%)
//          5.705.311      dTLB-loads                #    4,087 M/sec                    (35,82%)
//             41.241      dTLB-load-misses          #    0,72% of all dTLB cache accesses  (35,82%)
//                100      iTLB-loads                #   71,639 /sec                     (35,82%)
//                 87      iTLB-load-misses          #   87,00% of all iTLB cache accesses  (35,58%)
//
//        1,396362310 seconds time elapsed
//
//        1,396399000 seconds user
//        0,000000000 seconds sys
//
// MEMO: there are only 100 iTLB-loads, the miss rate is expected
//

// Infraclass reuses allocs
// elapsed: 2.0444s
//
//  Performance counter stats for './class-reuse.exe':
//
//           2.047,71 msec task-clock                #    1,000 CPUs utilized
//                  2      context-switches          #    0,977 /sec
//                  0      cpu-migrations            #    0,000 /sec
//                829      page-faults               #  404,843 /sec
//      8.437.519.476      cycles                    #    4,120 GHz                      (35,32%)
//          4.247.283      stalled-cycles-frontend   #    0,05% frontend cycles idle     (35,51%)
//      5.772.488.048      stalled-cycles-backend    #   68,41% backend cycles idle      (35,71%)
//     13.785.398.157      instructions              #    1,63  insn per cycle
//                                                   #    0,42  stalled cycles per insn  (35,90%)
//      4.012.918.754      branches                  #    1,960 G/sec                    (36,10%)
//             22.500      branch-misses             #    0,00% of all branches          (36,14%)
//      5.152.372.337      L1-dcache-loads           #    2,516 G/sec                    (36,14%)
//        343.328.212      L1-dcache-load-misses     #    6,66% of all L1-dcache accesses  (36,14%)
//    <not supported>      LLC-loads
//    <not supported>      LLC-load-misses
//         26.876.415      L1-icache-loads           #   13,125 M/sec                    (35,97%)
//            324.066      L1-icache-load-misses     #    1,21% of all L1-icache accesses  (35,77%)
//          5.666.027      dTLB-loads                #    2,767 M/sec                    (35,58%)
//             32.723      dTLB-load-misses          #    0,58% of all dTLB cache accesses  (35,38%)
//                125      iTLB-loads                #   61,044 /sec                     (35,19%)
//                  2      iTLB-load-misses          #    1,60% of all iTLB cache accesses  (35,16%)
//
//        2,048220674 seconds time elapsed
//
//        2,048213000 seconds user
//        0,000000000 seconds sys
//
// stalls too much: must be because of atomics
//

//
// Infraclass always allocates and frees
// elapsed: 2.3491s
//
//  Performance counter stats for './class-reuse.exe':
//
//           2.352,10 msec task-clock                #    1,000 CPUs utilized
//                  6      context-switches          #    2,551 /sec
//                  0      cpu-migrations            #    0,000 /sec
//                828      page-faults               #  352,026 /sec
//      9.611.145.684      cycles                    #    4,086 GHz                      (35,72%)
//          2.240.423      stalled-cycles-frontend   #    0,02% frontend cycles idle     (35,72%)
//      3.343.049.619      stalled-cycles-backend    #   34,78% backend cycles idle      (35,72%)
//     40.488.707.970      instructions              #    4,21  insn per cycle
//                                                   #    0,08  stalled cycles per insn  (35,72%)
//      9.886.125.750      branches                  #    4,203 G/sec                    (35,72%)
//             77.781      branch-misses             #    0,00% of all branches          (35,71%)
//     15.049.353.869      L1-dcache-loads           #    6,398 G/sec                    (35,71%)
//        372.406.999      L1-dcache-load-misses     #    2,47% of all L1-dcache accesses  (35,71%)
//    <not supported>      LLC-loads
//    <not supported>      LLC-load-misses
//         40.035.947      L1-icache-loads           #   17,021 M/sec                    (35,71%)
//            485.561      L1-icache-load-misses     #    1,21% of all L1-icache accesses  (35,71%)
//          5.717.769      dTLB-loads                #    2,431 M/sec                    (35,71%)
//             54.450      dTLB-load-misses          #    0,95% of all dTLB cache accesses  (35,71%)
//                 44      iTLB-loads                #   18,707 /sec                     (35,71%)
//                  5      iTLB-load-misses          #   11,36% of all iTLB cache accesses  (35,71%)
//
//        2,352702559 seconds time elapsed
//
//        2,352579000 seconds user
//        0,000000000 seconds sys
