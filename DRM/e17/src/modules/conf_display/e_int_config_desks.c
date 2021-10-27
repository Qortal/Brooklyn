#include "e.h"

/* PROTOTYPES - same all the time */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void _cb_slider_change(void *data, Evas_Object *obj);

/* Actual config data we will be playing with whil the dialog is active */
struct _E_Config_Dialog_Data
{
   /*- BASIC -*/
   int x;
   int y;
   int edge_flip_dragging;
   int flip_wrap;
   int flip_mode;
   int flip_interp;

   Eina_List *comp_effects;

   /*- GUI -*/
   Evas_Object *preview;
};

/* a nice easy setup function that does the dirty work */
E_Config_Dialog *
e_int_config_desks(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "screen/virtual_desktops")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   /* methods */
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->basic.check_changed = _basic_check_changed;
   v->advanced.apply_cfdata = NULL;
   v->advanced.create_widgets = NULL;
   v->advanced.check_changed = NULL;
   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(NULL, _("Virtual Desktops Settings"),
			     "E", "screen/virtual_desktops",
			     "preferences-desktop", 0, v, NULL);
   return cfd;
}

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   Eina_List *l;
   Eina_Stringshare *grp;

   cfdata->x = e_config->zone_desks_x_count;
   cfdata->y = e_config->zone_desks_y_count;
   cfdata->edge_flip_dragging = e_config->edge_flip_dragging;
   cfdata->flip_wrap = e_config->desk_flip_wrap;
   cfdata->flip_interp = e_config->desk_flip_animate_interpolation;

   cfdata->flip_mode = 0;
   cfdata->comp_effects = e_theme_collection_items_find("base/theme/borders", "e/comp/effects/auto");
   if (!cfdata->comp_effects) return;
   /* comp effects must be prefixed with "auto" or they won't work and things will break! */
   EINA_LIST_FOREACH(cfdata->comp_effects, l, grp)
     {
        Eina_Stringshare *g;
        char buf[1024];

        snprintf(buf, sizeof(buf), "auto/%s", grp);
        g = eina_stringshare_add(buf);
        eina_stringshare_del(grp);
        eina_list_data_set(l, g);
     }
   cfdata->comp_effects = eina_list_prepend(cfdata->comp_effects, eina_stringshare_add("none"));
   if (!e_util_strcmp(e_config->desk_flip_animate_type, "none")) return;
   EINA_LIST_FOREACH(cfdata->comp_effects->next, l, grp)
     {
        cfdata->flip_mode++;
        if (grp == e_config->desk_flip_animate_type)
          return;
     }
   cfdata->flip_mode = 0; // not found
}

static void *
_create_data(E_Config_Dialog *cfd EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   E_FREE_LIST(cfdata->comp_effects, eina_stringshare_del);
   free(cfdata);
}

/**--APPLY--**/

static int
_basic_apply_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   /* Actually take our cfdata settings and apply them in real life */
   if ((cfdata->x != e_config->zone_desks_x_count) ||
       (cfdata->y != e_config->zone_desks_y_count))
     {
        const Eina_List *l;
        E_Zone *zone;

        EINA_LIST_FOREACH(e_comp->zones, l, zone)
          e_zone_desk_count_set(zone, cfdata->x, cfdata->y);
     }

   eina_stringshare_replace(&e_config->desk_flip_animate_type, NULL);
   if (cfdata->flip_mode)
     e_config->desk_flip_animate_type = eina_stringshare_ref(eina_list_nth(cfdata->comp_effects, cfdata->flip_mode));
   e_config->desk_flip_animate_interpolation = cfdata->flip_interp;

   e_config->edge_flip_dragging = cfdata->edge_flip_dragging;
   e_config->desk_flip_wrap = cfdata->flip_wrap;

   e_config_save_queue();
   return 1; /* Apply was OK */
}

static int
_basic_check_changed(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   const Eina_List *l;
   E_Zone *zone;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        int x, y;

        e_zone_desk_count_get(zone, &x, &y);
        if ((x != cfdata->x) || (y != cfdata->y))
        return 1;
     }

   return ((e_util_strcasecmp(eina_list_nth(cfdata->comp_effects, cfdata->flip_mode), e_config->desk_flip_animate_type)) ||
	   (e_config->desk_flip_animate_interpolation != cfdata->flip_interp) ||
	   (e_config->edge_flip_dragging != cfdata->edge_flip_dragging) ||
	   (e_config->desk_flip_wrap != cfdata->flip_wrap));
}

/**--GUI--**/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *ob, *of, *otb;
   E_Radio_Group *rg;
   Eina_List *l;
   Eina_Stringshare *s;
   int mode = 0;

   otb = e_widget_toolbook_add(evas, (48 * e_scale), (48 * e_scale));

   of = e_widget_frametable_add(evas, _("Number of Desktops"), 0);
   e_widget_frametable_content_align_set(of, 0.5, 0.0);

   ob = e_widget_label_add(evas, _("Click to change wallpaper and name"));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);

   ob = e_widget_bgpreview_add(evas, cfdata->x, cfdata->y);
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   cfdata->preview = ob;

   ob = e_widget_slider_add(evas, 0, 0, _("%1.0f"), 1.0, 12.0, 1.0, 0, NULL,
                            &(cfdata->y), 150);
   e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 0, 1);

   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f"), 1.0, 12.0, 1.0, 0, NULL,
                            &(cfdata->x), 200);
   e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 0);

   e_widget_toolbook_page_append(otb, NULL, _("Desktops"), of, 1, 1, 1, 1,
                                 0.5, 0.0);

   o = e_widget_list_add(evas, 0, 0);

   ob = e_widget_check_add(evas, _("Flip when dragging objects to the screen edge"),
                           &(cfdata->edge_flip_dragging));
   e_widget_list_object_append(o, ob, 1, 1, 0.);

   ob = e_widget_check_add(evas, _("Wrap desktops around when flipping"),
                           &(cfdata->flip_wrap));
   e_widget_list_object_append(o, ob, 1, 0, 0.5);

   of = e_widget_framelist_add(evas, _("Animation"), 0);

   /* FIXME: this could maybe be some sort of demo list like comp config? */
   rg = e_widget_radio_group_new(&(cfdata->flip_mode));
   EINA_LIST_FOREACH(cfdata->comp_effects, l, s)
     {
        char *p;
        const char *pp;
        char buf[PATH_MAX];

        pp = strchr(s, '/');
        pp = pp ? pp + 1 : s;
        p = memcpy(buf, pp, strlen(pp) + 1);
        p[0] = toupper(p[0]);
        ob = e_widget_radio_add(evas, _(p), mode, rg);
        e_widget_framelist_object_append(of, ob);
        mode++;
     }

   e_widget_list_object_append(o, of, 1, 1, 0.);

   e_widget_toolbook_page_append(otb, NULL, _("Flip"), o,
                                 1, 0, 1, 0, 0.5, 0.0);

   e_widget_toolbook_page_show(otb, 0);

   return otb;
}

static void
_cb_slider_change(void *data, Evas_Object *obj EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;

   e_widget_bgpreview_num_desks_set(cfdata->preview, cfdata->x, cfdata->y);
}
