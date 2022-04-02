#ifdef E_TYPEDEFS

typedef struct _E_Pixmap E_Pixmap;

typedef enum
{
   E_PIXMAP_TYPE_X,
   E_PIXMAP_TYPE_WL,
   E_PIXMAP_TYPE_NONE,
} E_Pixmap_Type;

#else
# ifndef E_PIXMAP_H
# define E_PIXMAP_H

E_API int e_pixmap_free(E_Pixmap *cp);
E_API E_Pixmap *e_pixmap_ref(E_Pixmap *cp);
E_API E_Pixmap *e_pixmap_new(E_Pixmap_Type type, ...);
E_API E_Pixmap_Type e_pixmap_type_get(const E_Pixmap *cp);
E_API void *e_pixmap_resource_get(E_Pixmap *cp);
E_API void e_pixmap_resource_set(E_Pixmap *cp, void *resource);
E_API void e_pixmap_parent_window_set(E_Pixmap *cp, Ecore_Window win);
E_API void e_pixmap_visual_cmap_set(E_Pixmap *cp, void *visual, unsigned int cmap);
E_API unsigned int e_pixmap_failures_get(const E_Pixmap *cp);
E_API void *e_pixmap_visual_get(const E_Pixmap *cp);
E_API uint32_t e_pixmap_pixmap_get(const E_Pixmap *cp);
E_API Eina_Bool e_pixmap_dirty_get(E_Pixmap *cp);
E_API void e_pixmap_clear(E_Pixmap *cp);
E_API void e_pixmap_usable_set(E_Pixmap *cp, Eina_Bool set);
E_API Eina_Bool e_pixmap_usable_get(const E_Pixmap *cp);
E_API void e_pixmap_dirty(E_Pixmap *cp);
E_API Eina_Bool e_pixmap_refresh(E_Pixmap *cp);
E_API Eina_Bool e_pixmap_size_changed(E_Pixmap *cp, int w, int h);
E_API Eina_Bool e_pixmap_size_get(E_Pixmap *cp, int *w, int *h);
E_API void e_pixmap_client_set(E_Pixmap *cp, E_Client *ec);
E_API E_Client *e_pixmap_client_get(E_Pixmap *cp);
E_API E_Pixmap *e_pixmap_find(E_Pixmap_Type type, ...);
E_API E_Client *e_pixmap_find_client(E_Pixmap_Type type, ...);
E_API int64_t e_pixmap_window_get(E_Pixmap *cp);
E_API Ecore_Window e_pixmap_parent_window_get(E_Pixmap *cp);
E_API Eina_Bool e_pixmap_is_pixels(E_Pixmap *cp);
E_API Eina_Bool e_pixmap_native_surface_init(E_Pixmap *cp, Evas_Native_Surface *ns);
E_API void e_pixmap_image_clear(E_Pixmap *cp, Eina_Bool cache);
E_API Eina_Bool e_pixmap_image_refresh(E_Pixmap *cp);
E_API Eina_Bool e_pixmap_image_exists(const E_Pixmap *cp);
E_API Eina_Bool e_pixmap_image_is_argb(const E_Pixmap *cp);
E_API void *e_pixmap_image_data_get(E_Pixmap *cp);
E_API Eina_Bool e_pixmap_image_data_argb_convert(E_Pixmap *cp, void *pix, void *ipix, Eina_Rectangle *r, int stride);
E_API Eina_Bool e_pixmap_image_draw(E_Pixmap *cp, const Eina_Rectangle *r);

E_API void e_pixmap_image_opaque_set(E_Pixmap *cp, int x, int y, int w, int h);
E_API void e_pixmap_image_opaque_get(E_Pixmap *cp, int *x, int *y, int *w, int *h);

E_API void e_pixmap_alias(E_Pixmap *cp, E_Pixmap_Type type, ...);

#ifdef HAVE_WAYLAND
E_API Eina_Bool e_pixmap_dmabuf_test(struct linux_dmabuf_buffer *);
E_API Eina_Bool e_pixmap_dmabuf_formats_query(int **formats, int *num_formats);
E_API Eina_Bool e_pixmap_dmabuf_modifiers_query(int format, uint64_t **modifiers, int *num_modifiers);

#endif

static inline Eina_Bool
e_pixmap_is_x(const E_Pixmap *cp)
{
   return cp && e_pixmap_type_get(cp) == E_PIXMAP_TYPE_X;
}

# endif

#endif
