#ifdef E_TYPEDEFS
#else
# ifndef E_VIDEO_H
#  define E_VIDEO_H

E_API Evas_Object *e_video_add(Evas_Object *parent, const char *file, Eina_Bool lowq);
E_API const char *e_video_file_get(Evas_Object *obj);

# endif
#endif
