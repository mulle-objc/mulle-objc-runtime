//
//  mulle-objc-universe-dll-loader.c
//  mulle-objc-runtime
//
//  Copyright (c) 2026 Nat! - Mulle kybernetiK.
//  All rights reserved.
//
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
#include "mulle-objc-universe-dll-loader.h"

#include "include-private.h"

#include "mulle-objc-universe.h"


#ifdef _WIN32

#include <psapi.h>

static HMODULE mulle_objc__self__handle = NULL;

BOOL WINAPI DllMain( HMODULE hModule, DWORD reason, LPVOID unused)
{
   MULLE_C_UNUSED( unused);

   if (reason == DLL_PROCESS_ATTACH)
      mulle_objc__self__handle = hModule;
   return TRUE;
}


static char   *get_module_directory( HMODULE hModule, char *buf, size_t bufsize)
{
   char   *lastSlash;

   if( ! GetModuleFileNameA( hModule, buf, bufsize))
      return( NULL);

   lastSlash = strrchr( buf, '\\');
   if( ! lastSlash)
      lastSlash = strrchr( buf, '/');
   if( lastSlash)
      *lastSlash = '\0';

   return( buf);
}


static int   should_load_dll( char *filename)
{
   char   c;

   if( ! filename || ! *filename)
      return( 0);

   c = filename[0];
   if( c >= 'A' && c <= 'Z')
      return( 1);

   if( c == 'l' && filename[1] == 'i' && filename[2] == 'b')
   {
      c = filename[3];
      if( c >= 'A' && c <= 'Z')
         return( 1);
   }

   return( 0);
}


static void   collect_loaded_dlls( struct mulle_set *loaded)
{
   DWORD      cbNeeded;
   HANDLE     hProcess;
   HMODULE    hMods[1024];
   TCHAR      szModName[MAX_PATH];
   char       *filename;
   unsigned   i;

   hProcess = GetCurrentProcess();
   if( ! EnumProcessModules( hProcess, hMods, sizeof( hMods), &cbNeeded))
      return;

   for( i = 0; i < (cbNeeded / sizeof( HMODULE)); i++)
   {
      if( GetModuleFileNameA( hMods[i], szModName, sizeof( szModName)))
      {
         filename = strrchr( szModName, '\\');
         if( ! filename)
            filename = strrchr( szModName, '/');
         filename = filename ? filename + 1 : szModName;

         mulle_set_insert( loaded, filename);
      }
   }
}


static void   collect_candidate_dlls_from_directory( struct mulle_set *loaded,
                                                     struct mulle_array *candidates,
                                                     char *dirPath,
                                                     mulle_objc_dll_filter_t *filter)
{
   HANDLE            hFind;
   WIN32_FIND_DATAA  findData;
   char              *fullPath;
   char              *searchPath;

   mulle_buffer_do( searchPathBuffer)
   {
      mulle_buffer_sprintf( searchPathBuffer, "%s\\*.dll", dirPath);
      searchPath = mulle_buffer_get_string( searchPathBuffer);

      hFind = FindFirstFileA( searchPath, &findData);
      if( hFind == INVALID_HANDLE_VALUE)
         break;

      do
      {
         if( ! filter || (*filter)( findData.cFileName))
         {
            if( ! mulle_set_member( loaded, findData.cFileName))
            {
               mulle_buffer_do( fullPathBuffer)
               {
                  mulle_buffer_sprintf( fullPathBuffer, "%s\\%s", dirPath, findData.cFileName);
                  fullPath = mulle_buffer_get_string( fullPathBuffer);
                  
                  mulle_array_add( candidates, fullPath);
               }
            }
         }
      }
      while( FindNextFileA( hFind, &findData));

      FindClose( hFind);
   }
}


int   _mulle_objc_universe_dll_loader_filtered( struct _mulle_objc_universe *universe,
                                                char *dllpath,
                                                mulle_objc_dll_filter_t *filter)
{
   char        *dirPath;
   char        *p;
   char        *sep;
   char        pathBuf[MAX_PATH];
   char        *candidate;
   size_t      length;
   size_t      n_candidates;
   size_t      n_loaded;
   
   if( ! dllpath || ! *dllpath)
      return( -1);

   n_loaded= 0;

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "_mulle_objc_universe_dll_loader loads missing classes from dormant DLLs");

   mulle_set_do( loaded, &mulle_container_keycallback_copied_cstring)
   {
      collect_loaded_dlls( loaded);
      if( universe->debug.trace.universe)
         mulle_objc_universe_trace( universe,
                                    "found %td DLLs already loaded",
                                    mulle_set_get_count( loaded));

      mulle_array_do( candidates, &mulle_container_keycallback_copied_cstring)
      {
         mulle_set_do( seen, &mulle_container_keycallback_copied_cstring)
         {
            p = dllpath;
            while( *p)
            {
               sep = strchr( p, ';');
               if( sep)
                  length = sep - p;
               else
                  length = strlen( p);

               if( length > 0)
               {
                  if( length == 8 && ! strncmp( p, "__self__", 8))
                  {
                     dirPath = get_module_directory( mulle_objc__self__handle, pathBuf, sizeof( pathBuf));
                     if( dirPath && ! mulle_set_member( seen, dirPath))
                     {
                        mulle_set_insert( seen, dirPath);
                        collect_candidate_dlls_from_directory( loaded, candidates, dirPath, filter);
                     }
                  }
                  else if( length == 7 && ! strncmp( p, "__exe__", 7))
                  {
                     dirPath = get_module_directory( NULL, pathBuf, sizeof( pathBuf));
                     if( dirPath && ! mulle_set_member( seen, dirPath))
                     {
                        mulle_set_insert( seen, dirPath);
                        collect_candidate_dlls_from_directory( loaded, candidates, dirPath, filter);
                     }
                  }
                  else
                  {
                     dirPath = alloca( length + 1);
                     memcpy( dirPath, p, length);
                     dirPath[length] = '\0';

                     if( ! mulle_set_member( seen, dirPath))
                     {
                        mulle_set_insert( seen, dirPath);
                        collect_candidate_dlls_from_directory( loaded, candidates, dirPath, filter);
                     }
                  }
               }

               if( ! sep)
                  break;
               p = sep + 1;
            }
         }

         n_candidates = mulle_array_get_count( candidates);
         if( universe->debug.trace.universe)
            mulle_objc_universe_trace( universe,
                                       "found %td DLLs to load", n_candidates);

         mulle_array_for( candidates, candidate)
         {
            if( universe->debug.trace.universe)
               mulle_objc_universe_trace( universe,
                                          "loading DLL \"%s\"",
                                          candidate);
            if( ! LoadLibraryA( candidate))
               mulle_fprintf( stderr, "mulle-objc-runtime warning: failed to load %s: %lu\n",
                              candidate, GetLastError());
            else
               ++n_loaded;
         }
      }
   }

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "_mulle_objc_universe_dll_loader finished %s",
                                           n_loaded == 0
                                           ? "without succeeding to load DLLs"
                                           : "successfully");

   return( n_loaded ? 0 : -1);
}


int   _mulle_objc_universe_dll_loader( struct _mulle_objc_universe *universe, char *dllpath)
{
   int   rval;

   rval = -1;

   //
   // execute this only once, do not block if LoadLibrary by chance executes
   // another ObjC call that would then rerun everything again
   //
   mulle_thread_once_do_noblock( once)
   {
      rval = _mulle_objc_universe_dll_loader_filtered( universe, dllpath, should_load_dll);
   }

   return( rval);
}

#endif  // _WIN32

