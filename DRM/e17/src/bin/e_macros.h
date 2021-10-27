#ifndef E_MACROS_H
# define E_MACROS_H


# ifdef E_API
#  undef E_API
# endif
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
#    if 0
#     pragma GCC visibility push(hidden)
#    endif
#    define E_API __attribute__ ((visibility("default")))
#   else
#    define E_API
#   endif
#  else
#   define E_API
#  endif
# endif

# ifdef EINTERN
#  undef EINTERN
# endif
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EINTERN __attribute__ ((visibility("hidden")))
#  else
#   define EINTERN
#  endif
# else
#  define EINTERN
# endif

/* convenience macro to compress code and avoid typos */
#ifndef MAX
# define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
# define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

# define E_FREE_FUNC(_h, _fn) do { if (_h) { _fn((void*)_h); _h = NULL; } } while (0)
# define E_INTERSECTS(x, y, w, h, xx, yy, ww, hh) \
  (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && (((x) + (w)) > (xx)) && (((y) + (h)) > (yy)))
# define E_INSIDE(x, y, xx, yy, ww, hh) \
  (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && ((x) >= (xx)) && ((y) >= (yy)))
# define E_CONTAINS(x, y, w, h, xx, yy, ww, hh) \
  (((xx) >= (x)) && (((x) + (w)) >= ((xx) + (ww))) && ((yy) >= (y)) && (((y) + (h)) >= ((yy) + (hh))))
# define E_SPANS_COMMON(x1, w1, x2, w2) \
  (!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))
# define E_REALLOC(p, s, n)   p = (s *)realloc(p, sizeof(s) * n)
# define E_NEW(s, n)          (s *)calloc(n, sizeof(s))
# define E_NEW_RAW(s, n)      (s *)malloc(n * sizeof(s))
# define E_FREE(p)            do { free(p); p = NULL; } while (0)
# define E_FREE_LIST(list, free)    \
  do                                \
    {                               \
       void *_tmp_;                 \
       EINA_LIST_FREE(list, _tmp_) \
         {                          \
            free(_tmp_);            \
         }                          \
    }                               \
  while (0)

# define E_LIST_REVERSE_FREE(list, data)         \
  for (data = eina_list_last_data_get(list);          \
       list;                                     \
       list = eina_list_remove_list(list, eina_list_last(list)), \
       data = eina_list_last_data_get(list))

# define E_LIST_FOREACH(list, func)    \
  do                                \
    {                               \
       void *_tmp_;                 \
       const Eina_List *_list, *_list2;  \
       EINA_LIST_FOREACH_SAFE(list, _list, _list2, _tmp_) \
         {                          \
            func(_tmp_);            \
         }                          \
    }                               \
  while (0)

# define E_LIST_HANDLER_APPEND(list, type, callback, data) \
  do \
    { \
       Ecore_Event_Handler *_eh; \
       _eh = ecore_event_handler_add(type, (void *)callback, data); \
       assert(_eh); \
       list = eina_list_append(list, _eh); \
    } \
  while (0)
# define E_LIST_HANDLER_APPEND_PRE(list, type, callback, data) \
  do \
    { \
       Ecore_Event_Handler *_eh; \
       _eh = ecore_event_handler_prepend(type, (void *)callback, data); \
       assert(_eh); \
       list = eina_list_append(list, _eh); \
    } \
  while (0)

# define E_CLAMP(x, min, max) (x < min ? min : (x > max ? max : x))
# define E_RECTS_CLIP_TO_RECT(_x, _y, _w, _h, _cx, _cy, _cw, _ch) \
  {                                                               \
     if (E_INTERSECTS(_x, _y, _w, _h, _cx, _cy, _cw, _ch))        \
       {                                                          \
          if ((int)_x < (int)(_cx))                               \
            {                                                     \
               _w += _x - (_cx);                                  \
               _x = (_cx);                                        \
               if ((int)_w < 0) _w = 0;                           \
            }                                                     \
          if ((int)(_x + _w) > (int)((_cx) + (_cw)))              \
            _w = (_cx) + (_cw) - _x;                              \
          if ((int)_y < (int)(_cy))                               \
            {                                                     \
               _h += _y - (_cy);                                  \
               _y = (_cy);                                        \
               if ((int)_h < 0) _h = 0;                           \
            }                                                     \
          if ((int)(_y + _h) > (int)((_cy) + (_ch)))              \
            _h = (_cy) + (_ch) - _y;                              \
       }                                                          \
     else                                                         \
       {                                                          \
          _w = 0; _h = 0;                                         \
       }                                                          \
  }

#define E_WEIGHT evas_object_size_hint_weight_set
#define E_ALIGN evas_object_size_hint_align_set
#define E_EXPAND(X) E_WEIGHT((X), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND)
#define E_FILL(X) E_ALIGN((X), EVAS_HINT_FILL, EVAS_HINT_FILL)

#ifdef E_RELEASE_BUILD
# define E_BITFIELD :1
#else
# define E_BITFIELD
#endif

#endif
