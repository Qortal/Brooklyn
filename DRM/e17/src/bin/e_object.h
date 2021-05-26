#ifdef E_TYPEDEFS

/* Object safety/debugging checks */
/* */
/* OBJECT_PARANOIA_CHECK is paranoid and checks pointers and traps segv's */
/* in case they dont work... very paranoid and slow. NB for backtrace to */
/* work you need gcc, glibc and you need to compile with compile options of */
/* -g -rdynamic and link with them too */
/* OBJECT_CHECK is a simple null pointer and magic number check with no */
/* debug output */

#ifndef __GLIBC__
# ifdef OBJECT_PARANOIA_CHECK
#  warning "Your system doesn't have glibc. Paranoid object checking disabled."
#  undef OBJECT_PARANOIA_CHECK
# endif
#endif

#define E_OBJECT_MAGIC                  0xe0b9ec75
#define E_OBJECT_MAGIC_FREED            0xe0bf6eed
#define E_OBJECT(x)                     ((E_Object *)(x))
#define E_OBJECT_CLEANUP_FUNC(x)        ((E_Object_Cleanup_Func)(x))
#define E_OBJECT_ALLOC(x, type, cleanup_func) e_object_alloc(sizeof(x), (type), E_OBJECT_CLEANUP_FUNC(cleanup_func))
#define E_OBJECT_DEL_SET(x, del_func)   e_object_del_func_set(E_OBJECT(x), E_OBJECT_CLEANUP_FUNC(del_func))

#ifdef OBJECT_PARANOIA_CHECK
# define E_OBJECT_CHECK(x)                       do {if (e_object_error(E_OBJECT(x))) return;} while (0)
# define E_OBJECT_CHECK_RETURN(x, ret)           do {if (e_object_error(E_OBJECT(x))) return ret;} while (0)
#  define E_OBJECT_TYPE_CHECK(x, tp)             do {if ((E_OBJECT(x)->type) != (tp)) { CRI("Object type check failed in %s", __FUNCTION__); return;} } while (0)
#  define E_OBJECT_TYPE_CHECK_RETURN(x, tp, ret) do {if ((E_OBJECT(x)->type) != tp) { CRI("Object type check failed in %s", __FUNCTION__); return ret;} } while (0)
#  define E_OBJECT_IF_NOT_TYPE(x, tp)            if (E_OBJECT(x)->type != (tp))
#else
# ifdef OBJECT_CHECK
#  define E_OBJECT_CHECK(x)                       do {if ((!E_OBJECT(x)) || (E_OBJECT(x)->magic != (int)E_OBJECT_MAGIC)) return;} while (0)
#  define E_OBJECT_CHECK_RETURN(x, ret)           do {if ((!E_OBJECT(x)) || (E_OBJECT(x)->magic != (int)E_OBJECT_MAGIC)) return ret;} while (0)
#  define E_OBJECT_TYPE_CHECK(x, tp)              do {if ((E_OBJECT(x)->type) != (int)(tp)) { CRI("Object type check failed in %s", __FUNCTION__); return;} } while (0)
#  define E_OBJECT_TYPE_CHECK_RETURN(x, tp, ret)  do {if ((E_OBJECT(x)->type) != (int)(tp)) { CRI("Object type check failed in %s", __FUNCTION__); return ret;} } while (0)
#  define E_OBJECT_IF_NOT_TYPE(x, tp)             if (E_OBJECT(x)->type != (int)(tp))
# else
#  define E_OBJECT_CHECK(x)
#  define E_OBJECT_CHECK_RETURN(x, ret)
#  define E_OBJECT_TYPE_CHECK(x, type)
#  define E_OBJECT_TYPE_CHECK_RETURN(x, type, ret)
#  define E_OBJECT_IF_NOT_TYPE(x, type)
# endif
#endif

typedef void (*E_Object_Cleanup_Func) (void *obj);

typedef struct _E_Object       E_Object;
typedef struct _E_Object_Delfn E_Object_Delfn;

#else
#ifndef E_OBJECT_H
#define E_OBJECT_H

struct _E_Object
{
   int                      magic;
   int                      type;
   int                      references;
   E_Object_Cleanup_Func    del_func;
   E_Object_Cleanup_Func    cleanup_func;
   E_Object_Cleanup_Func    free_att_func;
   E_Object_Cleanup_Func    del_att_func;
   E_Object_Cleanup_Func    del_delay_func;
   Eina_Inlist             *del_fn_list;
   void                    *data;
   Ecore_Job               *delay_del_job;
   int                      walking_list;
   Eina_Bool                ref_debug E_BITFIELD;
   Eina_Bool                deleted E_BITFIELD;
};

struct _E_Object_Delfn
{
   EINA_INLIST;

   void (*func) (void *data, void *obj);
   void  *data;
   Eina_Bool delete_me E_BITFIELD;
};

E_API void *e_object_alloc               (int size, int type, E_Object_Cleanup_Func cleanup_func);
E_API void  e_object_del                 (E_Object *obj);
E_API void  e_object_delay_del_set       (E_Object *obj, void *func);
E_API int   e_object_is_del              (E_Object *obj);
E_API void  e_object_del_func_set        (E_Object *obj, E_Object_Cleanup_Func del_func);
E_API void  e_object_type_set            (E_Object *obj, int type);
E_API void  e_object_free                (E_Object *obj);
E_API int   e_object_ref                 (E_Object *obj);
E_API int   e_object_unref               (E_Object *obj);
E_API int   e_object_ref_get             (E_Object *obj);
E_API int   e_object_error               (E_Object *obj);
E_API void  e_object_data_set            (E_Object *obj, const void *data);
E_API void *e_object_data_get            (E_Object *obj);
E_API void  e_object_free_attach_func_set(E_Object *obj, E_Object_Cleanup_Func func);
E_API void  e_object_del_attach_func_set (E_Object *obj, E_Object_Cleanup_Func func);

E_API E_Object_Delfn *e_object_delfn_add (E_Object *obj, void (*func) (void *data, void *obj), void *data);
E_API void            e_object_delfn_del (E_Object *obj, E_Object_Delfn *dfn);
E_API void            e_object_delfn_clear(E_Object *obj);

E_API void e_object_ref_debug_set(E_Object *obj, Eina_Bool set);

/*
E_API void  e_object_breadcrumb_add      (E_Object *obj, char *crumb);
E_API void  e_object_breadcrumb_del      (E_Object *obj, char *crumb);
E_API void  e_object_breadcrumb_debug    (E_Object *obj);
*/

#endif
#endif
