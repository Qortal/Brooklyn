#include "e.h"

struct _E_Config_Dialog_Data
{
   int         engine;
   int         indirect;
   int         texture_from_pixmap;
   int         smooth_windows;
   int         grab;
   int         vsync;
   int         dither;
   int         swap_mode;

   const char *shadow_style;

   struct
   {
      int disable_popups;
      int disable_borders;
      int disable_overrides;
      int disable_menus;
      int disable_objects;
      int disable_all;
      int toggle_changed E_BITFIELD;
   } match;

   Evas_Object *styles_il;
   Evas_Object *vsync_check;
   Evas_Object *dither_check;
   Evas_Object *texture_check;

   Evas_Object *swap_auto_radio;
   Evas_Object *swap_full_radio;
   Evas_Object *swap_copy_radio;
   Evas_Object *swap_double_radio;
   Evas_Object *swap_triple_radio;

   int          keep_unmapped;
   int          max_unmapped_pixels;
   int          max_unmapped_time;
   int          min_unmapped_time;
   int          send_flush;
   int          send_dump;
   int          nocomp_fs;
   int          nofade;

   int          fps_show;
   int          fps_corner;
   double       first_draw_delay;
   int enable_advanced_features;
};

/* Protos */
static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd,
                               E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd,
                                          Evas *evas,
                                          E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd,
                                      E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd,
                                          Evas *evas,
                                          E_Config_Dialog_Data *cfdata);
static int          _advanced_apply_data(E_Config_Dialog *cfd,
                                      E_Config_Dialog_Data *cfdata);

E_API E_Config_Dialog *
e_int_config_comp(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "appearance/comp")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;

   cfd = e_config_dialog_new(NULL, _("Composite Settings"),
                             "E", "appearance/comp", "preferences-composite", 0, v, NULL);
   return cfd;
}

static void *
_create_data(E_Config_Dialog *cfd EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   E_Comp_Config *conf = e_comp_config_get();

   cfdata = E_NEW(E_Config_Dialog_Data, 1);

   cfdata->engine = conf->engine;
   if ((cfdata->engine != E_COMP_ENGINE_SW) &&
       (cfdata->engine != E_COMP_ENGINE_GL))
     cfdata->engine = E_COMP_ENGINE_SW;

   cfdata->enable_advanced_features = conf->enable_advanced_features;

   cfdata->indirect = conf->indirect;
   cfdata->texture_from_pixmap = conf->texture_from_pixmap;
   cfdata->smooth_windows = conf->smooth_windows;
   cfdata->grab = conf->grab;
   cfdata->vsync = conf->vsync;
   cfdata->dither = !(!!conf->no_dither);
   cfdata->swap_mode = conf->swap_mode;
   cfdata->shadow_style = eina_stringshare_add(conf->shadow_style);

   cfdata->keep_unmapped = conf->keep_unmapped;
   cfdata->max_unmapped_pixels = conf->max_unmapped_pixels;
   cfdata->max_unmapped_time = conf->max_unmapped_time;
   cfdata->min_unmapped_time = conf->min_unmapped_time;
   cfdata->send_flush = conf->send_flush;
   cfdata->send_dump = conf->send_dump;
   cfdata->nocomp_fs = conf->nocomp_fs;
   cfdata->nofade = conf->nofade;

   cfdata->fps_show = conf->fps_show;
   cfdata->fps_corner = conf->fps_corner;

   cfdata->first_draw_delay = conf->first_draw_delay;

   return cfdata;
}

static void
_comp_engine_toggle(void *data, Evas_Object *o EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;

   e_widget_disabled_set(cfdata->vsync_check,
                         (cfdata->engine == E_COMP_ENGINE_SW));
   e_widget_disabled_set(cfdata->dither_check,
                         (cfdata->engine == E_COMP_ENGINE_SW));

   if (cfdata->texture_check)
     e_widget_disabled_set(cfdata->texture_check,
                           (cfdata->engine == E_COMP_ENGINE_SW));

   if (cfdata->swap_auto_radio)
     e_widget_disabled_set(cfdata->swap_auto_radio,
                           (cfdata->engine == E_COMP_ENGINE_SW));

   if (cfdata->swap_full_radio)
     e_widget_disabled_set(cfdata->swap_full_radio,
                           (cfdata->engine == E_COMP_ENGINE_SW));

   if (cfdata->swap_copy_radio)
     e_widget_disabled_set(cfdata->swap_copy_radio,
                           (cfdata->engine == E_COMP_ENGINE_SW));

   if (cfdata->swap_double_radio)
     e_widget_disabled_set(cfdata->swap_double_radio,
                           (cfdata->engine == E_COMP_ENGINE_SW));

   if (cfdata->swap_triple_radio)
     e_widget_disabled_set(cfdata->swap_triple_radio,
                           (cfdata->engine == E_COMP_ENGINE_SW));
}

static void
_advanced_features_changed(E_Comp_Config *conf)
{
   conf->enable_advanced_features = !conf->enable_advanced_features;
   if (conf->enable_advanced_features)
     e_util_dialog_internal(_("WARNING"),
                            _("This option WILL break your desktop if you don't know what you're doing.<ps/>"
                              "Do not file bugs about anything that occurs with this option enabled.<ps/>"
                              "You have been warned."));
}

static void
_free_data(E_Config_Dialog *cfd  EINA_UNUSED,
           E_Config_Dialog_Data *cfdata)
{
   eina_stringshare_del(cfdata->shadow_style);
   free(cfdata);
}

static void
_advanced_matches_edit(void *data, void *d EINA_UNUSED)
{
   E_Config_Dialog *cfd = data;

   e_int_config_comp_match(cfd->dia->win, NULL);
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *ob,*ol, *of, *otb, *oi, *orec0;
   E_Radio_Group *rg;

   elm_win_center(cfd->dia->win, 1, 1);
   orec0 = evas_object_rectangle_add(evas);
   evas_object_name_set(orec0, "style_shadows");

   otb = e_widget_toolbook_add(evas, 48 * e_scale, 48 * e_scale);

   ///////////////////////////////////////////
   ol = e_widget_list_add(evas, 0, 0);

   ob = e_widget_button_add(evas, _("Edit window matches"), NULL, _advanced_matches_edit, cfd, NULL);
   e_widget_list_object_append(ol, ob, 0, 0, 0.5);

   of = e_widget_frametable_add(evas, _("Select default style"), 0);
   e_widget_frametable_content_align_set(of, 0.5, 0.5);
   cfdata->styles_il = oi = e_comp_style_selector_create(evas, &(cfdata->shadow_style));
   e_widget_frametable_object_append(of, oi, 0, 0, 1, 1, 1, 1, 1, 1);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   e_widget_toolbook_page_append(otb, NULL, _("Styles"), ol, 1, 1, 1, 1, 0.5, 0.0);

   ///////////////////////////////////////////
   ol = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Behavior"), 0);
   ob = e_widget_check_add(evas, _("Smooth scaling"), &(cfdata->smooth_windows));
   e_widget_framelist_object_append(of, ob);
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        ob = e_widget_check_add(evas, _("Don't composite fullscreen windows"), &(cfdata->nocomp_fs));
        e_widget_framelist_object_append(of, ob);
     }
   ob = e_widget_check_add(evas, _("Don't fade backlight"), &(cfdata->nofade));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Engine"), 0);
   rg = e_widget_radio_group_new(&(cfdata->engine));
   ob = e_widget_radio_add(evas, _("Software"), E_COMP_ENGINE_SW, rg);
   evas_object_smart_callback_add(ob, "changed", _comp_engine_toggle, cfdata);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("OpenGL"), E_COMP_ENGINE_GL, rg);
   evas_object_smart_callback_add(ob, "changed", _comp_engine_toggle, cfdata);
   e_widget_framelist_object_append(of, ob);
   if ((e_comp->comp_type == E_PIXMAP_TYPE_X) && (!getenv("ECORE_X_NO_XLIB")))
     {
        if (ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_OPENGL_X11))
          {
             ob = e_widget_label_add(evas, _("OpenGL options:"));
             e_widget_framelist_object_append(of, ob);

             ob = e_widget_check_add(evas, _("Tear-free updates (VSynced)"), &(cfdata->vsync));
             e_widget_framelist_object_append(of, ob);
             cfdata->vsync_check = ob;
             e_widget_disabled_set(ob, (cfdata->engine == E_COMP_ENGINE_SW));

             ob = e_widget_check_add(evas, _("Texture from pixmap"), &(cfdata->texture_from_pixmap));
             e_widget_framelist_object_append(of, ob);
             cfdata->texture_check = ob;
             e_widget_disabled_set(ob, (cfdata->engine == E_COMP_ENGINE_SW));

             ob = e_widget_label_add(evas, _("Assume swapping method:"));
             e_widget_framelist_object_append(of, ob);

             rg = e_widget_radio_group_new(&(cfdata->swap_mode));

             ob = e_widget_radio_add(evas, _("Auto"), ECORE_EVAS_GL_X11_SWAP_MODE_AUTO, rg);
             e_widget_framelist_object_append(of, ob);
             cfdata->swap_auto_radio = ob;
             e_widget_disabled_set(ob, (cfdata->engine == E_COMP_ENGINE_SW));

             ob = e_widget_radio_add(evas, _("Invalidate (full redraw)"), ECORE_EVAS_GL_X11_SWAP_MODE_FULL, rg);
             e_widget_framelist_object_append(of, ob);
             cfdata->swap_full_radio = ob;
             e_widget_disabled_set(ob, (cfdata->engine == E_COMP_ENGINE_SW));

             ob = e_widget_radio_add(evas, _("Copy from back to front"), ECORE_EVAS_GL_X11_SWAP_MODE_COPY, rg);
             e_widget_framelist_object_append(of, ob);
             cfdata->swap_copy_radio = ob;
             e_widget_disabled_set(ob, (cfdata->engine == E_COMP_ENGINE_SW));

             ob = e_widget_radio_add(evas, _("Double buffered swaps"), ECORE_EVAS_GL_X11_SWAP_MODE_DOUBLE, rg);
             e_widget_framelist_object_append(of, ob);
             cfdata->swap_double_radio = ob;
             e_widget_disabled_set(ob, (cfdata->engine == E_COMP_ENGINE_SW));

             ob = e_widget_radio_add(evas, _("Triple buffered swaps"), ECORE_EVAS_GL_X11_SWAP_MODE_TRIPLE, rg);
             e_widget_framelist_object_append(of, ob);
             cfdata->swap_triple_radio = ob;
             e_widget_disabled_set(ob, (cfdata->engine == E_COMP_ENGINE_SW));

// lets not offer this anymore
//             ob = e_widget_check_add(evas, _("Indirect OpenGL (EXPERIMENTAL)"), &(cfdata->indirect));
//             e_widget_framelist_object_append(of, ob);
          }
     }
   ob = e_widget_check_add(evas, _("Dithering"), &(cfdata->dither));
   e_widget_framelist_object_append(of, ob);
   cfdata->dither_check = ob;
   e_widget_disabled_set(ob, (cfdata->engine == E_COMP_ENGINE_SW));
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   e_widget_toolbook_page_append(otb, NULL, _("Rendering"), ol, 0, 0, 0, 0, 0.5, 0.0);

   ///////////////////////////////////////////
   ol = e_widget_list_add(evas, 0, 0);
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        of = e_widget_framelist_add(evas, _("X Messages"), 0);
        ob = e_widget_check_add(evas, _("Send flush"), &(cfdata->send_flush));
        e_widget_framelist_object_append(of, ob);
        ob = e_widget_check_add(evas, _("Send dump"), &(cfdata->send_dump));
        e_widget_framelist_object_append(of, ob);
        e_widget_list_object_append(ol, of, 1, 1, 0.5);

        of = e_widget_framelist_add(evas, _("Sync"), 0);
        ob = e_widget_check_add(evas, _("Grab Server during draw"), &(cfdata->grab));
        e_widget_framelist_object_append(of, ob);
        ob = e_widget_label_add(evas, _("Initial draw timeout for newly mapped windows"));
        e_widget_framelist_object_append(of, ob);
        ob = e_widget_slider_add(evas, 1, 0, _("%1.2f Seconds"), 0.01, 0.5, 0.01, 0, &(cfdata->first_draw_delay), NULL, 150);
        e_widget_framelist_object_append(of, ob);
        e_widget_list_object_append(ol, of, 1, 1, 0.5);
     }
   of = e_widget_framelist_add(evas, _("DANGEROUS"), 0);
   ob = e_widget_check_add(evas, _("Enable advanced compositing features"), &(cfdata->enable_advanced_features));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Misc"), ol, 0, 0, 0, 0, 0.5, 0.0);

   ///////////////////////////////////////////
/*
   ol = e_widget_list_add(evas, 0, 0);
   ol2 = e_widget_list_add(evas, 1, 1);
   of = e_widget_framelist_add(evas, _("Min hidden"), 0);
   e_widget_framelist_content_align_set(of, 0.5, 0.0);
   rg = e_widget_radio_group_new(&(cfdata->min_unmapped_time));
   ob = e_widget_radio_add(evas, _("30 Seconds"), 30, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("1 Minute"), 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("5 Minutes"), 5 * 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("30 Minutes"), 30 * 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("2 Hours"), 2 * 3600, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("10 Hours"), 10 * 3600, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Forever"), 0, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol2, of, 1, 1, 0.5);
   of = e_widget_framelist_add(evas, _("Max hidden"), 0);
   e_widget_framelist_content_align_set(of, 0.5, 0.0);
   rg = e_widget_radio_group_new(&(cfdata->max_unmapped_time));
   ob = e_widget_radio_add(evas, _("30 Seconds"), 30, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("1 Minute"), 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("5 Minutes"), 5 * 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("30 Minutes"), 30 * 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("2 Hours"), 2 * 3600, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("10 Hours"), 10 * 3600, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Forever"), 0, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol2, of, 1, 1, 0.5);
   e_widget_list_object_append(ol, ol2, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Timeouts"), ol, 0, 0, 0, 0, 0.5, 0.0);
 */
   ///////////////////////////////////////////
   ol = e_widget_list_add(evas, 0, 0);

   ob = e_widget_check_add(evas, _("Show Framerate"), &(cfdata->fps_show));
   e_widget_list_object_append(ol, ob, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, _("Corner"), 0);
   e_widget_frametable_content_align_set(of, 0.5, 0.5);
   rg = e_widget_radio_group_new(&(cfdata->fps_corner));
   ob = e_widget_radio_icon_add(evas, _("Top Left"), "preferences-position-top-left",
                                24, 24, 0, rg);
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, _("Top Right"), "preferences-position-top-right",
                                24, 24, 1, rg);
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, _("Bottom Left"), "preferences-position-bottom-left",
                                24, 24, 2, rg);
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, _("Bottom Right"), "preferences-position-bottom-right",
                                24, 24, 3, rg);
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   e_widget_toolbook_page_append(otb, NULL, _("Debug"), ol, 0, 0, 0, 0, 0.5, 0.0);

   e_widget_toolbook_page_show(otb, 0);

   return otb;
}

static int
_advanced_apply_data(E_Config_Dialog *cfd  EINA_UNUSED,
                     E_Config_Dialog_Data *cfdata)
{
   E_Comp_Config *conf = e_comp_config_get();

   if ((cfdata->smooth_windows != conf->smooth_windows) ||
       (cfdata->grab != conf->grab) ||
       (cfdata->keep_unmapped != conf->keep_unmapped) ||
       (cfdata->nocomp_fs != conf->nocomp_fs) ||
       (cfdata->nofade != conf->nofade) ||
       (cfdata->shadow_style != conf->shadow_style) ||
       (cfdata->max_unmapped_pixels != conf->max_unmapped_pixels) ||
       (cfdata->max_unmapped_time != conf->max_unmapped_time) ||
       (cfdata->min_unmapped_time != conf->min_unmapped_time) ||
       (cfdata->send_flush != conf->send_flush) ||
       (cfdata->send_dump != conf->send_dump) ||
       (cfdata->fps_show != conf->fps_show) ||
       (cfdata->fps_corner != conf->fps_corner) ||
       (!EINA_DBL_EQ(cfdata->first_draw_delay, conf->first_draw_delay)) ||
       (conf->enable_advanced_features != cfdata->enable_advanced_features)
       )
     {
        if (conf->enable_advanced_features != cfdata->enable_advanced_features)
          _advanced_features_changed(conf);
        conf->smooth_windows = cfdata->smooth_windows;
        conf->grab = cfdata->grab;
        conf->keep_unmapped = cfdata->keep_unmapped;
        conf->nocomp_fs = cfdata->nocomp_fs;
        conf->nofade = cfdata->nofade;
        conf->max_unmapped_pixels = cfdata->max_unmapped_pixels;
        conf->max_unmapped_time = cfdata->max_unmapped_time;
        conf->min_unmapped_time = cfdata->min_unmapped_time;
        conf->send_flush = cfdata->send_flush;
        conf->send_dump = cfdata->send_dump;
        conf->fps_show = cfdata->fps_show;
        conf->fps_corner = cfdata->fps_corner;
        conf->first_draw_delay = cfdata->first_draw_delay;
        if (conf->shadow_style)
          eina_stringshare_del(conf->shadow_style);
        conf->shadow_style = eina_stringshare_ref(cfdata->shadow_style);
        e_comp_shadows_reset();
     }
   if ((cfdata->engine != conf->engine) ||
       (cfdata->indirect != conf->indirect) ||
       (cfdata->texture_from_pixmap != conf->texture_from_pixmap) ||
       (cfdata->vsync != conf->vsync) ||
       (cfdata->dither != !(!!conf->no_dither)) ||
       (cfdata->swap_mode != conf->swap_mode))
     {
        E_Action *a;

        conf->engine = cfdata->engine;
        conf->indirect = cfdata->indirect;
        conf->texture_from_pixmap = cfdata->texture_from_pixmap;
        conf->vsync = cfdata->vsync;
        conf->no_dither = !(!!cfdata->dither);
        conf->swap_mode = cfdata->swap_mode;

        a = e_action_find("restart");
        if ((a) && (a->func.go)) a->func.go(NULL, NULL);
     }
   return e_comp_internal_save();
}

static void
_basic_comp_style_toggle(void *data, Evas_Object *o)
{
   E_Config_Dialog_Data *cfdata = data;

   e_widget_disabled_set(cfdata->styles_il, e_widget_check_checked_get(o));
   cfdata->match.toggle_changed = 1;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd,
                      Evas *evas,
                      E_Config_Dialog_Data *cfdata)
{
   Evas_Object *ob,*ol, *of, *otb, *oi, *orec0, *tab;
   E_Radio_Group *rg;

   e_dialog_resizable_set(cfd->dia, 0);
   elm_win_center(cfd->dia->win, 1, 1);
   orec0 = evas_object_rectangle_add(evas);
   evas_object_name_set(orec0, "style_shadows");

   tab = e_widget_table_add(e_win_evas_win_get(evas), 0);
   otb = e_widget_toolbook_add(evas, 48 * e_scale, 48 * e_scale);

   ///////////////////////////////////////////
   ol = e_widget_list_add(evas, 0, 0);

   ob = e_widget_check_add(evas, _("Don't fade backlight"), &(cfdata->nofade));
   e_widget_list_object_append(ol, ob, 1, 0, 0.5);

   of = e_widget_frametable_add(evas, _("Select default style"), 0);
   e_widget_frametable_content_align_set(of, 0.5, 0.5);
   cfdata->styles_il = oi = e_comp_style_selector_create(evas, &(cfdata->shadow_style));
   e_widget_frametable_object_append(of, oi, 0, 0, 1, 1, 1, 1, 1, 1);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   e_widget_on_change_hook_set(ob, _basic_comp_style_toggle, cfdata);

   e_widget_toolbook_page_append(otb, NULL, _("General"), ol, 1, 1, 1, 1, 0.5, 0.0);

   ///////////////////////////////////////////
   ol = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("Behavior"), 0);

   if (ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_OPENGL_X11))
     {
        ob = e_widget_check_add(evas, _("Tear-free updates (VSynced)"), &(cfdata->vsync));
        e_widget_framelist_object_append(of, ob);
        cfdata->vsync_check = ob;
        e_widget_disabled_set(ob, (cfdata->engine == E_COMP_ENGINE_SW));
     }
   ob = e_widget_check_add(evas, _("Dithering"), &(cfdata->dither));
   e_widget_framelist_object_append(of, ob);
   cfdata->dither_check = ob;
   e_widget_disabled_set(ob, (cfdata->engine == E_COMP_ENGINE_SW));

   ob = e_widget_check_add(evas, _("Smooth scaling of window content"), &(cfdata->smooth_windows));
   e_widget_framelist_object_append(of, ob);

   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        ob = e_widget_check_add(evas, _("Don't composite fullscreen windows"), &(cfdata->nocomp_fs));
        e_widget_framelist_object_append(of, ob);
     }

   e_widget_list_object_append(ol, of, 1, 0, 0.5);

   of = e_widget_framelist_add(evas, _("Engine"), 0);
   rg = e_widget_radio_group_new(&(cfdata->engine));
   ob = e_widget_radio_add(evas, _("Software"), E_COMP_ENGINE_SW, rg);
   e_widget_framelist_object_append(of, ob);
   evas_object_smart_callback_add(ob, "changed", _comp_engine_toggle, cfdata);
   ob = e_widget_radio_add(evas, _("OpenGL"), E_COMP_ENGINE_GL, rg);
   e_widget_framelist_object_append(of, ob);
   evas_object_smart_callback_add(ob, "changed", _comp_engine_toggle, cfdata);

   e_widget_list_object_append(ol, of, 1, 0, 0.5);

   e_widget_toolbook_page_append(otb, NULL, _("Rendering"), ol, 0, 0, 0, 0, 0.5, 0.0);

   e_widget_toolbook_page_show(otb, 0);


   e_widget_table_object_append(tab, otb, 0, 0, 1, 1, 1, 1, 1, 1);
   return tab;
}

static int
_basic_apply_data(E_Config_Dialog *cfd  EINA_UNUSED,
                  E_Config_Dialog_Data *cfdata)
{
   E_Comp_Config *conf = e_comp_config_get();

   if (cfdata->match.toggle_changed ||
       (cfdata->smooth_windows != conf->smooth_windows) ||
       (cfdata->grab != conf->grab) ||
       (cfdata->nofade != conf->nofade) ||
       (cfdata->keep_unmapped != conf->keep_unmapped) ||
       (cfdata->nocomp_fs != conf->nocomp_fs) ||
       (cfdata->shadow_style != conf->shadow_style) ||
       (cfdata->max_unmapped_pixels != conf->max_unmapped_pixels) ||
       (cfdata->max_unmapped_time != conf->max_unmapped_time) ||
       (cfdata->min_unmapped_time != conf->min_unmapped_time) ||
       (cfdata->send_flush != conf->send_flush) ||
       (cfdata->send_dump != conf->send_dump) ||
       (cfdata->fps_show != conf->fps_show) ||
       (cfdata->fps_corner != conf->fps_corner) ||
       (!EINA_DBL_EQ(cfdata->first_draw_delay, conf->first_draw_delay))
       )
     {
        conf->smooth_windows = cfdata->smooth_windows;
        conf->grab = cfdata->grab;
        conf->nofade = cfdata->nofade;
        conf->keep_unmapped = cfdata->keep_unmapped;
        conf->nocomp_fs = cfdata->nocomp_fs;
        conf->max_unmapped_pixels = cfdata->max_unmapped_pixels;
        conf->max_unmapped_time = cfdata->max_unmapped_time;
        conf->min_unmapped_time = cfdata->min_unmapped_time;
        if (conf->enable_advanced_features != cfdata->enable_advanced_features)
          _advanced_features_changed(conf);
        conf->send_flush = cfdata->send_flush;
        conf->send_dump = cfdata->send_dump;
        conf->fps_show = cfdata->fps_show;
        conf->fps_corner = cfdata->fps_corner;
        conf->first_draw_delay = cfdata->first_draw_delay;
        if (conf->shadow_style)
          eina_stringshare_del(conf->shadow_style);
        conf->shadow_style = NULL;
        if (cfdata->shadow_style)
          conf->shadow_style = eina_stringshare_add(cfdata->shadow_style);
        e_comp_shadows_reset();
     }
   if ((cfdata->engine != conf->engine) ||
       (cfdata->indirect != conf->indirect) ||
       (cfdata->texture_from_pixmap != conf->texture_from_pixmap) ||
       (cfdata->vsync != conf->vsync) ||
       (cfdata->dither != !(!!conf->no_dither)))
     {
        E_Action *a;

        conf->engine = cfdata->engine;
        conf->indirect = cfdata->indirect;
        conf->texture_from_pixmap = cfdata->texture_from_pixmap;
        conf->vsync = cfdata->vsync;
        conf->no_dither = !(!!cfdata->dither);

        a = e_action_find("restart");
        if ((a) && (a->func.go)) a->func.go(NULL, NULL);
     }
   return e_comp_internal_save();
}

