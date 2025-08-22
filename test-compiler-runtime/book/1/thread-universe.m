// Test for Chapter 1: Thread-Local Universe Access
// Demonstrates accessing thread-local storage and struct _mulle_objc_threadinfo

#import "include.h"
#include <stdio.h>
#include <pthread.h>

static void *thread_function(void *arg)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_threadinfo *threadinfo;
    
    // Get the default universe (ensures setup)
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Setup thread context (call only once per thread lifetime)
    mulle_objc_thread_setup_threadinfo(universe);
    
    // Now we can safely access threadinfo
    threadinfo = _mulle_objc_thread_get_threadinfo(universe);
    
    if (threadinfo)
    {
        printf("Thread %p: threadinfo=%p, thread_nr=%lu\n", 
               (void*)pthread_self(), threadinfo, 
               _mulle_objc_threadinfo_get_nr(threadinfo));
    }
    else
    {
        printf("Thread %p: no threadinfo\n", (void*)pthread_self());
    }
    
    return NULL;
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_threadinfo *threadinfo;
    pthread_t thread1, thread2;
    
    // Main thread - setup is automatic for main thread
    universe = mulle_objc_global_get_defaultuniverse();
    threadinfo = _mulle_objc_thread_get_threadinfo(universe);
    
    printf("Main thread: threadinfo=%p, thread_nr=%lu\n", 
           threadinfo, threadinfo ? _mulle_objc_threadinfo_get_nr(threadinfo) : 0);
    
    // Test with multiple threads
    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    return 0;
}