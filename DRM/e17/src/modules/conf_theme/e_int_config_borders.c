#include "e.h"

static E_Config_Dialog_View *_config_view_new(void);

static void                 *_create_data(E_Config_Dialog *cfd);
static void                  _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int                   _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object          *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int                   _basic_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void                  _fill_data(E_Config_Dialog_Data *cfdata);
static void                  _basic_apply_border(E_Config_Dialog_Data *cfdata);

struct _E_Config_Dialog_Data
{
   E_Client *client;
   const char  *bordername;
   int          remember_border;
};

E_Config_Dialog *
e_int_config_borders(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "appearance/borders")) return NULL;
   v = _config_view_new();
   if (!v) return NULL;
   cfd = e_config_dialog_new(NULL, _("Default Border Style"),
                             "E", "appearance/borders",
                             "preferences-system-windows", 0, v, NULL);
   return cfd;
}

E_Config_Dialog *
e_int_config_borders_border(E_Comp *comp EINA_UNUSED, const char *params)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   E_Client *ec;

   if (!params) return NULL;
   ec = NULL;
   sscanf(params, "%p", &ec);
   if (!ec) return NULL;
   v = _config_view_new();
   if (!v) return NULL;
   cfd = e_config_dialog_new(NULL,
                             _("Window Border Selection"),
                             "E", "_config_border_border_style_dialog",
                             "preferences-system-windows", 0, v, ec);
   ec->border_border_dialog = cfd;
   evas_object_layer_set(e_win_client_get(cfd->dia->win)->frame, ec->layer);
   return cfd;
}

static E_Config_Dialog_View *
_config_view_new(void)
{
   E_Config_Dialog_View *v;

   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return NULL;
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.check_changed = _basic_check_changed;
   v->override_auto_apply = 1;
   return v;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->client = NULL;
   if (cfd->data)
     cfdata->client = cfd->data;

   _fill_data(cfdata);
   return cfdata;
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   if (cfdata->client)
     {
        if ((cfdata->client->remember) &&
            (cfdata->client->remember->apply & E_REMEMBER_APPLY_BORDER))
          {
             cfdata->remember_border = 1;
          }
        cfdata->bordername = eina_stringshare_add(cfdata->client->border.name);
     }
   else
     cfdata->bordername = eina_stringshare_add(e_config->theme_default_border_style);
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   if (cfdata->client)
     cfdata->client->border_border_dialog = NULL;

   eina_stringshare_del(cfdata->bordername);
   E_FREE(cfdata);
}

static int
_basic_check_changed(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   Eina_Bool remch = ((cfdata->remember_border &&
                       !((cfdata->client->remember) &&
                         (cfdata->client->remember->apply & E_REMEMBER_APPLY_BORDER))) ||
                      (!cfdata->remember_border && cfdata->client &&
                          ((cfdata->client->remember) &&
                              (cfdata->client->remember->apply & E_REMEMBER_APPLY_BORDER))));
   if (cfdata->client)
     return (cfdata->bordername != cfdata->client->border.name) || (remch);
   else
     return (cfdata->bordername != e_config->theme_default_border_style) || (remch);
}

static int
_basic_apply(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   if (cfdata->client)
     _basic_apply_border(cfdata);
   else
     {
        Eina_List *l;
        E_Client *ec;
        eina_stringshare_replace(&e_config->theme_default_border_style, cfdata->bordername);
        EINA_LIST_FOREACH(e_comp->clients, l, ec)
          {
             if (e_client_util_ignored_get(ec)) continue;
             EC_CHANGED(ec);
             ec->border.changed = 1;
          }
     }
   e_config_save_queue();
   return 1;
}

static void
_basic_apply_border(E_Config_Dialog_Data *cfdata)
{
   if ((!cfdata->client->lock_border) && (!cfdata->client->shaded))
     {
        eina_stringshare_replace(&cfdata->client->bordername, cfdata->bordername);
        cfdata->client->border.changed = 1;
        EC_CHANGED(cfdata->client);
     }
   if (cfdata->remember_border)
     {
        E_Remember *rem = cfdata->client->remember;

        if (!rem)
          {
             rem = e_remember_new();
             if (rem) e_remember_use(rem);
          }
        if (rem)
          {
             rem->apply |= E_REMEMBER_APPLY_BORDER;
             e_remember_default_match_set(rem, cfdata->client);
             eina_stringshare_replace(&rem->prop.border, cfdata->client->bordername);
             cfdata->client->remember = rem;
             e_remember_update(cfdata->client);
          }
     }
   else
     {
        if (cfdata->client->remember)
          {
             cfdata->client->remember->apply &= ~E_REMEMBER_APPLY_BORDER;
             if (cfdata->client->remember->apply == 0)
               {
                  e_remember_unuse(cfdata->client->remember);
                  e_remember_del(cfdata->client->remember);
                  cfdata->client->remember = NULL;
               }
          }
     }
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *ol, *ob, *oj, *orect, *of;
   Evas_Coord w, h;
   Eina_List *borders;
   int n = 1, sel = 0;
   const char *str, *tmp;

   e_dialog_resizable_set(cfd->dia, 1);
   if (cfdata->client)
     tmp = cfdata->client->border.name;
   else
     tmp = e_config->theme_default_border_style;

   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Default Border Style"), 0);
   e_widget_framelist_content_align_set(of, 0.0, 0.0);
   ol = e_widget_ilist_add(evas, 96, 96, &(cfdata->bordername));
   borders = e_theme_border_list();
   orect = evas_object_rectangle_add(evas);
   evas_object_color_set(orect, 0, 0, 0, 128);

   evas_event_freeze(evas_object_evas_get(ol));
   edje_freeze();
   e_widget_ilist_freeze(ol);
   e_widget_ilist_append(ol, orect, "borderless", NULL, NULL, "borderless");

   EINA_LIST_FREE(borders, str)
     {
        char buf[4096];

        ob = e_livethumb_add(evas);
        e_livethumb_vsize_set(ob, 96, 96);
        oj = edje_object_add(e_livethumb_evas_get(ob));
        snprintf(buf, sizeof(buf), "e/widgets/border/%s/border", str);
        e_theme_edje_object_set(oj, "base/theme/borders", buf);
        e_livethumb_thumb_set(ob, oj);
        orect = evas_object_rectangle_add(e_livethumb_evas_get(ob));
        evas_object_color_set(orect, 0, 0, 0, 128);
        evas_object_show(orect);
        edje_object_part_swallow(oj, "e.swallow.client", orect);
        e_widget_ilist_append(ol, ob, (char *)str, NULL, NULL, str);
        if (tmp == str) sel = n;
        n++;
        eina_stringshare_del(str);
     }

   e_widget_size_min_get(ol, &w, &h);
   e_widget_size_min_set(ol, w > 200 ? w : 200, 100);

   e_widget_ilist_go(ol);
   e_widget_ilist_selected_set(ol, sel);
   e_widget_ilist_thaw(ol);
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(ol));

   e_widget_framelist_object_append(of, ol);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   if (cfdata->client)
     {
        ob = e_widget_check_add(evas, _("Remember this Border for this window next time it appears"),
                                &(cfdata->remember_border));
        e_widget_list_object_append(o, ob, 1, 0, 0.0);
     }

   return o;
}

