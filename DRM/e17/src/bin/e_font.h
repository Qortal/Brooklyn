#ifdef E_TYPEDEFS

typedef struct _E_Font_Default E_Font_Default;
typedef struct _E_Font_Fallback E_Font_Fallback;
typedef struct _E_Font_Available E_Font_Available;
typedef struct _E_Font_Properties E_Font_Properties;

#else
#ifndef E_FONT_H
#define E_FONT_H

struct _E_Font_Default
{
   const char     *text_class;
   const char     *font;
   Evas_Font_Size  size;
};

struct _E_Font_Fallback
{
   const char *name;
};

struct _E_Font_Available
{
   const char *name;
};

struct _E_Font_Properties
{
   const char *name;
   Eina_List *styles;
};

EINTERN int		e_font_init(void);
EINTERN int		e_font_shutdown(void);
E_API void		e_font_apply(void);
E_API Eina_List         *e_font_available_list(void);
E_API void		e_font_available_list_free(Eina_List *available);
E_API Eina_Hash         *e_font_available_list_parse(Eina_List *list);
E_API void               e_font_available_hash_free(Eina_Hash *hash);
E_API E_Font_Properties *e_font_fontconfig_name_parse(const char *font);
E_API const char        *e_font_fontconfig_name_get(const char *name, const char *style);
E_API void               e_font_properties_free(E_Font_Properties *efp);

/* global font fallbacks */
E_API void		e_font_fallback_clear(void);
E_API void		e_font_fallback_append(const char *font);
E_API void		e_font_fallback_prepend(const char *font);
E_API void		e_font_fallback_remove(const char *font);
E_API Eina_List         *e_font_fallback_list(void);

/* setup edje text classes */
E_API void		e_font_default_set(const char *text_class, const char *font, Evas_Font_Size size);
E_API E_Font_Default    *e_font_default_get(const char *text_class);
E_API void		e_font_default_remove(const char *text_class);
E_API Eina_List         *e_font_default_list(void);
E_API const char        *e_font_default_string_get(const char *text_class, Evas_Font_Size *size_ret);

#endif
#endif
