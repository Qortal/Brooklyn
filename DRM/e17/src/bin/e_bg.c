#include "e.h"

/* local subsystem functions */
static void _e_bg_signal(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_bg_event_bg_update_free(void *data, void *event);
static void e_bg_handler_set(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *path);
static int  e_bg_handler_test(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *path);
static void _e_bg_handler_image_imported(const char *image_path, void *data);

/* local subsystem globals */
E_API int E_EVENT_BG_UPDATE = 0;
static E_Fm2_Mime_Handler *bg_hdl = NULL;

/* externally accessible functions */
EINTERN int
e_bg_init(void)
{
   Eina_List *l = NULL;
   E_Config_Desktop_Background *cfbg = NULL;

   /* Register mime handler */
   bg_hdl = e_fm2_mime_handler_new(_("Set As Background"),
                                   "preferences-desktop-wallpaper",
                                   e_bg_handler_set, NULL,
                                   e_bg_handler_test, NULL);
   if (bg_hdl)
     {
        e_fm2_mime_handler_glob_add(bg_hdl, "*.edj");
        e_fm2_mime_handler_mime_add(bg_hdl, "image/png");
        e_fm2_mime_handler_mime_add(bg_hdl, "image/jpeg");
     }

   /* Register files in use */
   if (e_config->desktop_default_background)
     e_filereg_register(e_config->desktop_default_background);

   EINA_LIST_FOREACH(e_config->desktop_backgrounds, l, cfbg)
     {
        if (!cfbg) continue;
        e_filereg_register(cfbg->file);
     }

   E_EVENT_BG_UPDATE = ecore_event_type_new();
   return 1;
}

EINTERN int
e_bg_shutdown(void)
{
   Eina_List *l = NULL;
   E_Config_Desktop_Background *cfbg = NULL;

   /* Deregister mime handler */
   if (bg_hdl)
     {
        e_fm2_mime_handler_glob_del(bg_hdl, "*.edj");
        e_fm2_mime_handler_free(bg_hdl);
     }

   /* Deregister files in use */
   if (e_config->desktop_default_background)
     e_filereg_deregister(e_config->desktop_default_background);

   EINA_LIST_FOREACH(e_config->desktop_backgrounds, l, cfbg)
     {
        if (!cfbg) continue;
        e_filereg_deregister(cfbg->file);
     }

   return 1;
}

/**
 * Find the configuration for a given desktop background
 * Use -1 as a wild card for each parameter.
 * The most specific match will be returned
 */
E_API const E_Config_Desktop_Background *
e_bg_config_get(int zone_num, int desk_x, int desk_y)
{
   Eina_List *l, *entries;
   E_Config_Desktop_Background *bg = NULL, *cfbg = NULL;
   const char *bgfile = "";
   char *entry;
   int current_spec = 0; /* how specific the setting is - we want the least general one that applies */

   /* look for desk specific background. */
   if (zone_num >= 0 || desk_x >= 0 || desk_y >= 0)
     {
        EINA_LIST_FOREACH(e_config->desktop_backgrounds, l, cfbg)
          {
             int spec;

             if (!cfbg) continue;
             spec = 0;
             if (cfbg->zone == zone_num) spec++;
             else if (cfbg->zone >= 0)
               continue;
             if (cfbg->desk_x == desk_x) spec++;
             else if (cfbg->desk_x >= 0)
               continue;
             if (cfbg->desk_y == desk_y) spec++;
             else if (cfbg->desk_y >= 0)
               continue;

             if (spec <= current_spec) continue;
             bgfile = cfbg->file;
             if (bgfile)
               {
                  if (bgfile[0] != '/')
                    {
                       const char *bf;

                       bf = e_path_find(path_backgrounds, bgfile);
                       if (bf) bgfile = bf;
                    }
               }
             if (eina_str_has_extension(bgfile, ".edj"))
               {
                  entries = edje_file_collection_list(bgfile);
                  EINA_LIST_FREE(entries, entry)
                    {
                       if (!strcmp(entry, "e/desktop/background"))
                         {
                            bg = cfbg;
                            current_spec = spec;
                            break;
                         }
                       eina_stringshare_del(entry);
                    }
                  E_FREE_LIST(entries, eina_stringshare_del);
               }
             else
               {
                  bg = cfbg;
                  current_spec = spec;
               }
          }
     }
   return bg;
}

E_API Eina_Stringshare *
e_bg_file_get(int zone_num, int desk_x, int desk_y)
{
   const E_Config_Desktop_Background *cfbg;
   const char *bgfile = NULL;
   int ok = 0;

   cfbg = e_bg_config_get(zone_num, desk_x, desk_y);

   /* fall back to default */
   if (cfbg)
     {
        const char *bf;

        bgfile = eina_stringshare_ref(cfbg->file);
        if (!bgfile) return NULL;
        if (bgfile[0] == '/') return bgfile;
        bf = e_path_find(path_backgrounds, bgfile);
        if (!bf) return bgfile;
        eina_stringshare_del(bgfile);
        return bf;
     }
   bgfile = e_config->desktop_default_background;
   if (bgfile)
     {
        if (bgfile[0] != '/')
          {
             const char *bf;

             bf = e_path_find(path_backgrounds, bgfile);
             if (bf) bgfile = bf;
          }
        else
          eina_stringshare_ref(bgfile);
     }
   if (bgfile && eina_str_has_extension(bgfile, ".edj"))
     {
        ok = edje_file_group_exists(bgfile, "e/desktop/background");
     }
   else if ((bgfile) && (bgfile[0]))
     ok = 1;
   if (!ok)
     eina_stringshare_replace(&bgfile, e_theme_edje_file_get("base/theme/background",
                                                             "e/desktop/background"));

   return bgfile;
}

E_API void
e_bg_zone_update(E_Zone *zone, E_Bg_Transition transition)
{
   Evas_Object *o;
   const char *bgfile = "";
   const char *trans = "";
   E_Desk *desk;

   if (transition == E_BG_TRANSITION_START) trans = e_config->transition_start;
   else if (transition == E_BG_TRANSITION_DESK)
     trans = e_config->transition_desk;
   else if (transition == E_BG_TRANSITION_CHANGE)
     trans = e_config->transition_change;
   if ((!trans) || (!trans[0])) transition = E_BG_TRANSITION_NONE;

   desk = e_desk_current_get(zone);
   if (desk)
     bgfile = e_bg_file_get(zone->num, desk->x, desk->y);
   else
     bgfile = e_bg_file_get(zone->num, -1, -1);

   if (zone->bg_object)
     {
        const char *pfile = NULL;

        edje_object_file_get(zone->bg_object, &pfile, NULL);
        if (!pfile) e_icon_file_get(zone->bg_object, &pfile, NULL);
        if (!pfile) pfile = e_video_file_get(zone->bg_object);
        if (pfile)
          {
             if (!e_util_strcmp(pfile, bgfile)) goto end;
          }
     }

   if (transition == E_BG_TRANSITION_NONE)
     E_FREE_FUNC(zone->bg_object, evas_object_del);
   else
     {
        char buf[4096];

        if (zone->bg_object)
          {
             E_FREE_FUNC(zone->prev_bg_object, evas_object_del);
             zone->prev_bg_object = zone->bg_object;
             zone->bg_object = NULL;
             E_FREE_FUNC(zone->transition_object, evas_object_del);
          }
        o = edje_object_add(e_comp->evas);
        evas_object_repeat_events_set(o, 1);
        zone->transition_object = o;
        evas_object_name_set(zone->transition_object, "zone->transition_object");
        /* FIXME: segv if zone is deleted while up??? */
        evas_object_data_set(o, "e_zone", zone);
        snprintf(buf, sizeof(buf), "e/transitions/%s", trans);
        e_theme_edje_object_set(o, "base/theme/transitions", buf);
        edje_object_preload(o, EINA_FALSE);
        edje_object_signal_callback_add(o, "e,state,done", "*", _e_bg_signal, zone);
        evas_object_move(o, zone->x, zone->y);
        evas_object_resize(o, zone->w, zone->h);
        evas_object_layer_set(o, E_LAYER_BG);
        evas_object_clip_set(o, zone->bg_clip_object);
        evas_object_show(o);
     }
   if (eina_str_has_extension(bgfile, ".edj"))
     {
        o = edje_object_add(e_comp->evas);
        edje_object_file_set(o, bgfile, "e/desktop/background");
        edje_object_preload(o, EINA_FALSE);
        if (edje_object_data_get(o, "noanimation"))
          edje_object_animation_set(o, EINA_FALSE);
     }
   else if ((eina_str_has_extension(bgfile, ".gif")) ||
            (eina_str_has_extension(bgfile, ".png")) ||
            (eina_str_has_extension(bgfile, ".jpg")) ||
            (eina_str_has_extension(bgfile, ".jpeg")) ||
            (eina_str_has_extension(bgfile, ".bmp")))
     {
        o = e_icon_add(e_comp->evas);
        if (!(eina_str_has_extension(bgfile, ".gif")))
          e_icon_preload_set(o, EINA_TRUE);
        e_icon_scale_size_set(o, 0);
        e_icon_fill_inside_set(o, 0);
        e_icon_file_key_set(o, bgfile, NULL);
     }
   else
     {
        o = e_video_add(e_comp->evas, bgfile, EINA_FALSE);
     }
   evas_object_data_set(o, "e_zone", zone);
   evas_object_repeat_events_set(o, 1);
   zone->bg_object = o;
   evas_object_name_set(zone->bg_object, "zone->bg_object");
   if (transition == E_BG_TRANSITION_NONE)
     {
        evas_object_move(o, zone->x, zone->y);
        evas_object_resize(o, zone->w, zone->h);
        evas_object_layer_set(o, E_LAYER_BG);
     }
   evas_object_clip_set(o, zone->bg_clip_object);
   evas_object_show(o);

   if (transition != E_BG_TRANSITION_NONE)
     {
        edje_object_part_swallow(zone->transition_object, "e.swallow.bg.old",
                                 zone->prev_bg_object);
        edje_object_part_swallow(zone->transition_object, "e.swallow.bg.new",
                                 zone->bg_object);
        edje_object_signal_emit(zone->transition_object, "e,action,start", "e");
        evas_object_name_set(zone->transition_object, "zone->transition_object");
        evas_object_move(zone->transition_object, zone->x, zone->y);
        evas_object_resize(zone->transition_object, zone->w, zone->h);
     }
   if (zone->bg_object) evas_object_name_set(zone->bg_object, "zone->bg_object");
   if (zone->prev_bg_object) evas_object_name_set(zone->prev_bg_object, "zone->prev_bg_object");
   e_comp_canvas_zone_update(zone);
end:
   eina_stringshare_del(bgfile);
}

E_API void
e_bg_default_set(const char *file)
{
   E_Event_Bg_Update *ev;
   Eina_Bool changed;

   file = eina_stringshare_add(file);
   changed = file != e_config->desktop_default_background;

   if (!changed)
     {
        eina_stringshare_del(file);
        return;
     }

   if (e_config->desktop_default_background)
     {
        e_filereg_deregister(e_config->desktop_default_background);
        eina_stringshare_del(e_config->desktop_default_background);
     }

   if (file)
     {
        e_filereg_register(file);
        e_config->desktop_default_background = file;
     }
   else
     e_config->desktop_default_background = NULL;

   ev = E_NEW(E_Event_Bg_Update, 1);
   ev->zone = -1;
   ev->desk_x = -1;
   ev->desk_y = -1;
   ecore_event_add(E_EVENT_BG_UPDATE, ev, _e_bg_event_bg_update_free, NULL);
}

E_API void
e_bg_add(int zone, int desk_x, int desk_y, const char *file)
{
   const Eina_List *l;
   E_Config_Desktop_Background *cfbg;
   E_Event_Bg_Update *ev;

   file = eina_stringshare_add(file);

   EINA_LIST_FOREACH(e_config->desktop_backgrounds, l, cfbg)
     {
        if ((cfbg) &&
            (cfbg->zone == zone) &&
            (cfbg->desk_x == desk_x) &&
            (cfbg->desk_y == desk_y) &&
            (cfbg->file == file))
          {
             eina_stringshare_del(file);
             return;
          }
     }

   e_bg_del(zone, desk_x, desk_y);
   cfbg = E_NEW(E_Config_Desktop_Background, 1);
   cfbg->zone = zone;
   cfbg->desk_x = desk_x;
   cfbg->desk_y = desk_y;
   cfbg->file = file;
   e_config->desktop_backgrounds = eina_list_append(e_config->desktop_backgrounds, cfbg);

   e_filereg_register(cfbg->file);

   ev = E_NEW(E_Event_Bg_Update, 1);
   ev->zone = zone;
   ev->desk_x = desk_x;
   ev->desk_y = desk_y;
   ecore_event_add(E_EVENT_BG_UPDATE, ev, _e_bg_event_bg_update_free, NULL);
}

E_API void
e_bg_del(int zone, int desk_x, int desk_y)
{
   Eina_List *l;
   E_Config_Desktop_Background *cfbg;
   E_Event_Bg_Update *ev;

   EINA_LIST_FOREACH(e_config->desktop_backgrounds, l, cfbg)
     {
        if (!cfbg) continue;
        if ((cfbg->zone == zone) && (cfbg->desk_x == desk_x) && (cfbg->desk_y == desk_y))
          {
             e_config->desktop_backgrounds = eina_list_remove_list(e_config->desktop_backgrounds, l);
             e_filereg_deregister(cfbg->file);
             if (cfbg->file) eina_stringshare_del(cfbg->file);
             free(cfbg);
             break;
          }
     }

   ev = E_NEW(E_Event_Bg_Update, 1);
   ev->zone = zone;
   ev->desk_x = desk_x;
   ev->desk_y = desk_y;
   ecore_event_add(E_EVENT_BG_UPDATE, ev, _e_bg_event_bg_update_free, NULL);
}

E_API void
e_bg_update(void)
{
   const Eina_List *l;
   E_Zone *zone;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     e_zone_bg_reconfigure(zone);
}

/* local subsystem functions */

/**
 * Set background to image, as required in e_fm2_mime_handler_new()
 */
static void
e_bg_handler_set(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *path)
{
   char buf[4096];
   int copy = 1;

   if (!path) return;

   if (!eina_str_has_extension(path, "edj"))
     {
        e_import_config_dialog_show(NULL, path, (Ecore_End_Cb)_e_bg_handler_image_imported, NULL);
        return;
     }

   /* if not in system dir or user dir, copy to user dir */
   e_prefix_data_concat_static(buf, "data/backgrounds");
   if (!strncmp(buf, path, strlen(buf)))
     copy = 0;
   if (copy)
     {
        e_user_dir_concat_static(buf, "backgrounds");
        if (!strncmp(buf, path, strlen(buf)))
          copy = 0;
     }
   if (copy)
     {
        const char *file;
        char *name;

        file = ecore_file_file_get(path);
        name = ecore_file_strip_ext(file);

        e_user_dir_snprintf(buf, sizeof(buf), "backgrounds/%s-%f.edj", name, ecore_time_unix_get());
        free(name);

        if (!ecore_file_exists(buf))
          {
             ecore_file_cp(path, buf);
             e_bg_default_set(buf);
          }
        else
          e_bg_default_set(path);
     }
   else
     e_bg_default_set(path);

   e_bg_update();
   e_config_save_queue();
}

/**
 * Test if possible to set background to file, as required in
 * e_fm2_mime_handler_new()
 *
 * This handler tests for files that would be acceptable for setting
 * background.
 *
 * You should just register it with "*.edj" (glob matching extension)
 * or "image/" (mimetypes)that are acceptable with Evas loaders.
 *
 * Just edje files with "e/desktop/background" group are used.
 */
static int
e_bg_handler_test(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *path)
{
   if (!path) return 0;

   if (eina_str_has_extension(path, "edj"))
     {
        if (edje_file_group_exists(path, "e/desktop/background")) return 1;
        return 0;
     }

   /* it's image/png or image/jpeg, we'll import it. */
   return 1;
}

static void
_e_bg_signal(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Zone *zone = data;

   E_FREE_FUNC(zone->prev_bg_object, evas_object_del);
   E_FREE_FUNC(zone->transition_object, evas_object_del);
   e_comp_canvas_zone_update(zone);
   evas_object_move(zone->bg_object, zone->x, zone->y);
   evas_object_resize(zone->bg_object, zone->w, zone->h);
   evas_object_layer_set(zone->bg_object, E_LAYER_BG);
   evas_object_clip_set(zone->bg_object, zone->bg_clip_object);
   evas_object_show(zone->bg_object);
}

static void
_e_bg_event_bg_update_free(void *data EINA_UNUSED, void *event)
{
   free(event);
}

static void
_e_bg_handler_image_imported(const char *image_path, void *data EINA_UNUSED)
{
   e_bg_default_set(image_path);
   e_bg_update();
   e_config_save_queue();
}

