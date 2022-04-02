#include "e.h"

/* PROTOTYPES - same all the time */

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

/* Actual config data we will be playing with whil the dialog is active */
struct _E_Config_Dialog_Data
{
   int load_xrdb;
   int load_xmodmap;
   int load_gnome;
   int load_kde;
   int exe_always_single_instance;
   const char *desktop_environment;
   Eina_List *desktop_environments;
   int desktop_environment_id;
};

/* a nice easy setup function that does the dirty work */
E_Config_Dialog *
e_int_config_deskenv(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "windows/desktop_environments"))
     return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   /* methods */
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.create_widgets = _basic_create;
   v->basic.check_changed = _basic_check_changed;

   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(NULL, _("Desktop Environments"), "E",
                             "windows/desktop_environments",
                             "preferences-desktop-environments", 0, v, NULL);
   return cfd;
}

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->load_xrdb = e_config->deskenv.load_xrdb;
   cfdata->load_xmodmap = e_config->deskenv.load_xmodmap;
   cfdata->load_gnome = e_config->deskenv.load_gnome;
   cfdata->load_kde = e_config->deskenv.load_kde;
   cfdata->exe_always_single_instance = e_config->exe_always_single_instance;
// can ask efreet, but don't - have a fixed "sensible" list
//   cfdata->desktop_environments = efreet_util_desktop_environments_list();
   cfdata->desktop_environments = eina_list_append(cfdata->desktop_environments, "Enlightenment");
   cfdata->desktop_environments = eina_list_append(cfdata->desktop_environments, "GNOME");
   cfdata->desktop_environments = eina_list_append(cfdata->desktop_environments, "KDE");
   cfdata->desktop_environments = eina_list_append(cfdata->desktop_environments, "XFCE");
   eina_stringshare_replace(&(cfdata->desktop_environment), e_config->desktop_environment);
   if (e_config->desktop_environment)
     cfdata->desktop_environment_id = eina_list_count(cfdata->desktop_environments) + 1;
   else
     cfdata->desktop_environment_id = 0;
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
   eina_list_free(cfdata->desktop_environments);
   E_FREE(cfdata);
}

static int
_basic_check_changed(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   if (cfdata->desktop_environment_id > 0)
     {
        const char *de;

        de = eina_list_nth(cfdata->desktop_environments, (cfdata->desktop_environment_id - 1));
        eina_stringshare_replace(&(cfdata->desktop_environment), de);
     }
   else
     {
        eina_stringshare_replace(&(cfdata->desktop_environment), NULL);
     }

   return (e_config->deskenv.load_xrdb != cfdata->load_xrdb) ||
          (e_config->deskenv.load_xmodmap != cfdata->load_xmodmap) ||
          (e_config->deskenv.load_gnome != cfdata->load_gnome) ||
          (e_config->deskenv.load_kde != cfdata->load_kde) ||
          (e_config->exe_always_single_instance != cfdata->exe_always_single_instance) ||
          (e_util_strcmp(e_config->desktop_environment, cfdata->desktop_environment));
}

/**--APPLY--**/
static int
_basic_apply(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   e_config->deskenv.load_xrdb = cfdata->load_xrdb;
   e_config->deskenv.load_xmodmap = cfdata->load_xmodmap;
   e_config->deskenv.load_gnome = cfdata->load_gnome;
   e_config->deskenv.load_kde = cfdata->load_kde;
   e_config->exe_always_single_instance = cfdata->exe_always_single_instance;
   eina_stringshare_replace(&(e_config->desktop_environment), cfdata->desktop_environment);
   e_config_save_queue();
   efreet_desktop_environment_set(e_config->desktop_environment);
   return 1; /* Apply was OK */
}

/**--GUI--**/
static Evas_Object *
_basic_create(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *fr, *ob;
   Eina_List *l;
   E_Radio_Group *rg;
   const char *de;
   int cde = 0;

   o = e_widget_list_add(evas, 0, 0);

   fr = e_widget_framelist_add(evas, _("Execution"), 0);
   ob = e_widget_check_add(evas, _("Only launch single instances"),
                           &(cfdata->exe_always_single_instance));
   e_widget_framelist_object_append(fr, ob);
   e_widget_list_object_append(o, fr, 1, 0, 0.0);

#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        fr = e_widget_framelist_add(evas, _("X11 Basics"), 0);
        ob = e_widget_check_add(evas, _("Load X Resources"),
                                &(cfdata->load_xrdb));
        e_widget_framelist_object_append(fr, ob);
        ob = e_widget_check_add(evas, _("Load X Modifier Map"),
                                &(cfdata->load_xmodmap));
        e_widget_framelist_object_append(fr, ob);
        e_widget_list_object_append(o, fr, 1, 0, 0.0);
     }
#endif

   fr = e_widget_framelist_add(evas, _("Major Desktops"), 0);
   ob = e_widget_check_add(evas, _("Start GNOME services on login"),
                           &(cfdata->load_gnome));
   e_widget_framelist_object_append(fr, ob);
   ob = e_widget_check_add(evas, _("Start KDE services on login"),
                           &(cfdata->load_kde));
   e_widget_framelist_object_append(fr, ob);
   e_widget_list_object_append(o, fr, 1, 0, 0.0);

   fr = e_widget_framelist_add(evas, _("Show applications only for Environment"), 0);
   rg = e_widget_radio_group_new(&(cfdata->desktop_environment_id));
   ob = e_widget_radio_add(evas, _("Any"), cde, rg);
   e_widget_framelist_object_append(fr, ob);
   EINA_LIST_FOREACH(cfdata->desktop_environments, l, de)
     {
        if (!e_util_strcmp(e_config->desktop_environment, de))
          cfdata->desktop_environment_id = (cde + 1);
        ob = e_widget_radio_add(evas, de, ++cde, rg);
        e_widget_framelist_object_append(fr, ob);
     }

   e_widget_list_object_append(o, fr, 1, 0, 0.0);

   return o;
}

