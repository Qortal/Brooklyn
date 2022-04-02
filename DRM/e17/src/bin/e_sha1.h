#ifdef E_TYPEDEFS

#else
#ifndef E_SHA1_H
#define E_SHA1_H

#ifndef E_API
# ifdef WIN32
#  ifdef BUILDING_DLL
#   define E_API __declspec(dllexport)
#  else
#   define E_API __declspec(dllimport)
#  endif
# else
#  ifdef __GNUC__
#   if __GNUC__ >= 4
/* BROKEN in gcc 4 on amd64 */
#if 0
#   pragma GCC visibility push(hidden)
#endif
#    define E_API __attribute__ ((visibility("default")))
#   else
#    define E_API
#   endif
#  else
#   define E_API
#  endif
# endif
#endif

E_API int e_sha1_sum(unsigned char *data, int size, unsigned char *dst);

#endif
#endif
