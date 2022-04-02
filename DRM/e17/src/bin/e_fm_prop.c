#include "e.h"
#ifdef HAVE_LIBEXIF
#include <libexif/exif-data.h>
#endif

/* FIXME:
 *
 * basic -
 * + show file
 * + show size
 * + show last access date
 * + show modified date
 * + show mimetype
 * + show permissions (others read, others write)
 * + show preview
 * + show owner
 * + show icon
 * * show symlink/fifo/socket/etc. status
 * + show broken link status
 * + change icon for mime type
 * * change icon for just this file
 * + change permissions
 *
 * advanced (extra) -
 * * show change date
 * * show comment
 * * show generic
 * * show mount status
 * * show setuid bit
 * + show link destination (if symlink or link)
 * * show group
 * * change link destination
 * * change app to open THIS file with (or dir)
 *
 */

/* PROTOTYPES - same all the time */

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
#if 0
static int          _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
#endif

static void         _cb_icon_sel(void *data, void *data2);
static void         _cb_type(void *data, Evas_Object *obj, void *event_info);
static void         _cb_preview_update(void *data, Evas_Object *obj, void *event_info);
static void         _cb_fsel_sel(void *data, Evas_Object *obj);
static void         _cb_fsel_ok(void *data, E_Dialog *dia);
static void         _cb_fsel_cancel(void *data, E_Dialog *dia);

/* Actual config data we will be playing with while the dialog is active */
struct _E_Config_Dialog_Data
{
   E_Fm2_Icon      *ic;
   E_Fm2_Icon_Info *fi;
   struct
   {
      Evas_Object *icon_wid;
      Evas_Object *preview;
      Evas_Object *preview_table;
      Evas_Object *fsel_wid;
      E_Dialog    *fsel;
   } gui;
   /*- BASIC -*/
   char            *file;
   char            *location;
   char            *size;
   char            *mod_date;
   char            *acc_date;
   char            *pms_date;
   char            *exif_date;
   char            *mime;
   char            *owner;
   char            *link;
   char            *plink;
   char            *blocks;
   int              owner_read;
   int              owner_write;
   int              owner_exec;
   int              others_read;
   int              others_write;
   int              others_exec;
   int              group_read;
   int              group_write;
   int              group_exec;
   int              picon_type;
   int              picon_mime;
   int              picon_changed;
   int              icon_type;
   int              icon_mime;
   char            *icon;
   /*- ADVANCED -*/
};

/* a nice easy setup function that does the dirty work */
E_API E_Config_Dialog *
e_fm_prop_file(E_Fm2_Icon *ic)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   v = E_NEW(E_Config_Dialog_View, 1);

   /* methods */
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
#if 0
   v->advanced.apply_cfdata = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;
#endif
   /* create config dialog for NULL object/data */
   cfd = e_config_dialog_new(NULL,
                             _("File Properties"),
                             "E", "_fm_prop",
                             "enlightenment/file_properties", 0, v, ic);
   return cfd;
}

#ifdef HAVE_LIBEXIF
static ExifEntry *
_exif_entry_find(ExifData *ed, ExifTag tag)
{
   ExifEntry *entry;

   if ((entry = exif_content_get_entry(ed->ifd[EXIF_IFD_0], tag))) return entry;
   if ((entry = exif_content_get_entry(ed->ifd[EXIF_IFD_1], tag))) return entry;
   if ((entry = exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF], tag))) return entry;
   if ((entry = exif_content_get_entry(ed->ifd[EXIF_IFD_GPS], tag))) return entry;
   if ((entry = exif_content_get_entry(ed->ifd[EXIF_IFD_INTEROPERABILITY], tag))) return entry;
   return NULL;
}
#endif

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata, E_Fm2_Icon *ic)
{
   char loc[PATH_MAX];
   char blks[256];
   struct passwd *pw;

   cfdata->ic = ic;
   cfdata->fi = e_fm2_icon_file_info_get(ic);
   if (cfdata->fi->file) cfdata->file = strdup(cfdata->fi->file);
   cfdata->size = e_util_size_string_get(cfdata->fi->statinfo.st_size);
   cfdata->mod_date = e_util_file_time_get(cfdata->fi->statinfo.st_mtime);
   cfdata->acc_date = e_util_file_time_get(cfdata->fi->statinfo.st_atime);
   cfdata->pms_date = e_util_file_time_get(cfdata->fi->statinfo.st_ctime);
#ifdef HAVE_LIBEXIF
   snprintf(loc, sizeof(loc), "%s/%s", e_fm2_real_path_get(cfdata->fi->fm), cfdata->fi->file);
   ExifData *ed = exif_data_new_from_file(loc);
   if (ed)
     {
        ExifEntry *entry = _exif_entry_find(ed, EXIF_TAG_DATE_TIME_ORIGINAL);
        if (!entry) entry = _exif_entry_find(ed, EXIF_TAG_DATE_TIME_DIGITIZED);
        if (!entry) entry = _exif_entry_find(ed, EXIF_TAG_DATE_TIME);
        if (entry)
          {
             char tbuf[128];
             struct tm tm;
             time_t tim;

             exif_entry_get_value(entry, tbuf, sizeof(tbuf));
             tbuf[127] = 0;
             memset(&tm, 0, sizeof(tm));
             tm.tm_isdst = -1;
             // "YYYY:MM:DD HH:MM:SS"
             if (sscanf(tbuf, "%i:%i:%i %i:%i:%i",
                        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                        &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6)
               {
                  tm.tm_year -= 1900;
                  tm.tm_mon -= 1;
                  tim = mktime(&tm);
                  if (tim != (time_t)-1)
                    cfdata->exif_date = e_util_file_time_get(tim);
               }
          }
        exif_data_unref(ed);
     }
#endif
   if (cfdata->fi->mime) cfdata->mime = strdup(cfdata->fi->mime);

   snprintf(blks, sizeof(blks), "%lu", (unsigned long)cfdata->fi->statinfo.st_blocks);
   cfdata->blocks = strdup(blks);

   snprintf(loc, sizeof(loc), "%s", e_fm2_real_path_get(cfdata->fi->fm));
   cfdata->location = strdup(loc);

   pw = getpwuid(cfdata->fi->statinfo.st_uid);
   if (pw) cfdata->owner = strdup(pw->pw_name);
   if (cfdata->fi->link) cfdata->link = strdup(cfdata->fi->link);
   if (cfdata->fi->link) cfdata->plink = strdup(cfdata->fi->link);
   if (cfdata->fi->statinfo.st_mode & S_IRUSR) cfdata->owner_read = 1;
   if (cfdata->fi->statinfo.st_mode & S_IWUSR) cfdata->owner_write = 1;
   if (cfdata->fi->statinfo.st_mode & S_IEXEC) cfdata->owner_exec = 1;
   if (cfdata->fi->statinfo.st_mode & S_IROTH) cfdata->others_read = 1;
   if (cfdata->fi->statinfo.st_mode & S_IWOTH) cfdata->others_write = 1;
   if (cfdata->fi->statinfo.st_mode & S_IXOTH) cfdata->others_exec = 1;
   if (cfdata->fi->statinfo.st_mode & S_IRGRP) cfdata->group_read = 1;
   if (cfdata->fi->statinfo.st_mode & S_IWGRP) cfdata->group_write = 1;
   if (cfdata->fi->statinfo.st_mode & S_IXGRP) cfdata->group_exec = 1;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   /* Create cfdata - cfdata is a temporary block of config data that this
    * dialog will be dealing with while configuring. it will be applied to
    * the running systems/config in the apply methods
    */
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata, cfd->data);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   if (cfdata->gui.fsel)
     e_object_del(E_OBJECT(cfdata->gui.fsel));
   E_FREE(cfdata->file);
   E_FREE(cfdata->location);
   E_FREE(cfdata->size);
   E_FREE(cfdata->blocks);
   E_FREE(cfdata->mod_date);
   E_FREE(cfdata->acc_date);
   E_FREE(cfdata->pms_date);
   E_FREE(cfdata->exif_date);
   E_FREE(cfdata->mime);
   E_FREE(cfdata->owner);
   E_FREE(cfdata->link);
   E_FREE(cfdata->plink);
   E_FREE(cfdata->icon);
   free(cfdata);
}

/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   char buf[PATH_MAX];
   Eina_Bool fperm = EINA_FALSE;

   snprintf(buf, sizeof(buf), "%s/%s",
            cfdata->location, cfdata->fi->file);
   if (((cfdata->fi->statinfo.st_mode & S_IRUSR) && (cfdata->owner_read)) ||
       ((!(cfdata->fi->statinfo.st_mode & S_IRUSR)) && (!cfdata->owner_read)))
     fperm = EINA_TRUE;
   if (((cfdata->fi->statinfo.st_mode & S_IWUSR) && (cfdata->owner_write)) ||
       ((!(cfdata->fi->statinfo.st_mode & S_IWUSR)) && (!cfdata->owner_write)))
     fperm = EINA_TRUE;
   if (((cfdata->fi->statinfo.st_mode & S_IROTH) && (cfdata->others_read)) ||
       ((!(cfdata->fi->statinfo.st_mode & S_IROTH)) && (!cfdata->others_read)))
     fperm = EINA_TRUE;
   if (((cfdata->fi->statinfo.st_mode & S_IEXEC) && (cfdata->owner_exec)) ||
       ((!(cfdata->fi->statinfo.st_mode & S_IEXEC)) && (!cfdata->owner_exec)))
     fperm = EINA_TRUE;
   if (((cfdata->fi->statinfo.st_mode & S_IWOTH) && (cfdata->others_write)) ||
       ((!(cfdata->fi->statinfo.st_mode & S_IWOTH)) && (!cfdata->others_write)))
     fperm = EINA_TRUE;
   if (((cfdata->fi->statinfo.st_mode & S_IXOTH) && (cfdata->others_exec)) ||
       ((!(cfdata->fi->statinfo.st_mode & S_IXOTH)) && (!cfdata->others_exec)))
     fperm = EINA_TRUE;
   if (((cfdata->fi->statinfo.st_mode & S_IRGRP) && (cfdata->group_read)) ||
       ((!(cfdata->fi->statinfo.st_mode & S_IRGRP)) && (!cfdata->group_read)))
     fperm = EINA_TRUE;
   if (((cfdata->fi->statinfo.st_mode & S_IWGRP) && (cfdata->group_write)) ||
       ((!(cfdata->fi->statinfo.st_mode & S_IWGRP)) && (!cfdata->group_write)))
     fperm = EINA_TRUE;
   if (((cfdata->fi->statinfo.st_mode & S_IXGRP) && (cfdata->group_exec)) ||
       ((!(cfdata->fi->statinfo.st_mode & S_IXGRP)) && (!cfdata->group_exec)))
     fperm = EINA_TRUE;
   if (fperm == EINA_TRUE)
     {
        mode_t pmode;

        pmode = cfdata->fi->statinfo.st_mode;
        if (cfdata->owner_read) cfdata->fi->statinfo.st_mode |= S_IRUSR;
        else cfdata->fi->statinfo.st_mode &= ~S_IRUSR;
        if (cfdata->owner_write) cfdata->fi->statinfo.st_mode |= S_IWUSR;
        else cfdata->fi->statinfo.st_mode &= ~S_IWUSR;
        if (cfdata->owner_exec) cfdata->fi->statinfo.st_mode |= S_IEXEC;
        else cfdata->fi->statinfo.st_mode &= ~S_IEXEC;
        if (cfdata->others_read) cfdata->fi->statinfo.st_mode |= S_IROTH;
        else cfdata->fi->statinfo.st_mode &= ~S_IROTH;
        if (cfdata->others_write) cfdata->fi->statinfo.st_mode |= S_IWOTH;
        else cfdata->fi->statinfo.st_mode &= ~S_IWOTH;
        if (cfdata->others_exec) cfdata->fi->statinfo.st_mode |= S_IXOTH;
        else cfdata->fi->statinfo.st_mode &= ~S_IXOTH;
        if (cfdata->group_read) cfdata->fi->statinfo.st_mode |= S_IRGRP;
        else cfdata->fi->statinfo.st_mode &= ~S_IRGRP;
        if (cfdata->group_write) cfdata->fi->statinfo.st_mode |= S_IWGRP;
        else cfdata->fi->statinfo.st_mode &= ~S_IWGRP;
        if (cfdata->group_exec) cfdata->fi->statinfo.st_mode |= S_IXGRP;
        else cfdata->fi->statinfo.st_mode &= ~S_IXGRP;
        if (chmod(buf, cfdata->fi->statinfo.st_mode) == -1)
          {
             e_util_dialog_show(_("Error"),
                                _("Cannot change permissions: %s"),
                                strerror(errno));

             cfdata->fi->statinfo.st_mode = pmode;
          }
     }
   if ((cfdata->link) && ((cfdata->fi->real_link) || (cfdata->fi->broken_link)))
     {
        if ((cfdata->link[0]) && (strcmp(cfdata->plink, cfdata->link)))
          {
             ecore_file_unlink(buf);
             ecore_file_symlink(cfdata->link, buf);
             free(cfdata->plink);
             cfdata->plink = strdup(cfdata->link);
          }
     }
   if ((cfdata->picon_type != cfdata->icon_type) ||
       (cfdata->picon_mime != cfdata->icon_mime) ||
       (cfdata->picon_changed))
     {
        if ((cfdata->icon_mime) && (cfdata->mime)) /* modify mimetype */
          {
             Eina_List *l;
             E_Config_Mime_Icon *mi = NULL;
             Eina_Bool found = EINA_FALSE;

             if (!cfdata->picon_mime) /* remove previous custom icon info */
               e_fm2_custom_file_del(buf);
             EINA_LIST_FOREACH(e_config->mime_icons, l, mi)
               {
                  if (!mi) continue;
                  if (strcmp(mi->mime, cfdata->mime)) continue;
                  if (mi->icon)
                    {
                       eina_stringshare_del(mi->icon);
                       mi->icon = NULL;
                    }
                  found = EINA_TRUE;
                  break;
               }
             if ((found == EINA_FALSE) && (cfdata->icon_type != 0))
               {
                  mi = E_NEW(E_Config_Mime_Icon, 1);
                  mi->mime = eina_stringshare_add(cfdata->mime);
                  e_config->mime_icons = eina_list_append(e_config->mime_icons, mi);
               }
             /* FIXME: modify mime info */
             if (cfdata->icon_type == 0)
               {
                  if (found == EINA_TRUE)
                    {
                       e_config->mime_icons = eina_list_remove(e_config->mime_icons, mi);
                       if (mi->mime) eina_stringshare_del(mi->mime);
                       if (mi->icon) eina_stringshare_del(mi->icon);
                       free(mi);
                    }
               }
             else if (cfdata->icon_type == 1)
               {
                  mi->icon = eina_stringshare_add("THUMB");
               }
             else if (cfdata->icon_type == 2)
               {
                  mi->icon = eina_stringshare_add(cfdata->icon);
               }
             e_config_save_queue();
             e_fm_mime_icon_cache_flush();
          }
        else /* custom for this file */
          {
             E_Fm2_Custom_File *cf, cf0;

             cf = e_fm2_custom_file_get(buf);
             if (cf)
               {
                  cf->icon.type = cfdata->icon_type;
                  if (cf->icon.icon)
                    eina_stringshare_del(cf->icon.icon);
                  cf->icon.icon = NULL;
                  if (cfdata->icon_type == 2)
                    cf->icon.icon = eina_stringshare_add(cfdata->icon);
                  if (cfdata->icon_type == 0)
                    cf->icon.valid = 0;
                  else
                    cf->icon.valid = 1;
               }
             else
               {
                  memset(&cf0, 0, sizeof(E_Fm2_Custom_File));
                  cf = &cf0;
                  cf->icon.type = cfdata->icon_type;
                  if (cfdata->icon_type == 2)
                    cf->icon.icon = cfdata->icon;
                  if (cfdata->icon_type == 0)
                    cf->icon.valid = 0;
                  else
                    cf->icon.valid = 1;
               }
             e_fm2_custom_file_set(buf, cf);
             e_fm2_custom_file_flush();
          }
        cfdata->picon_type = cfdata->icon_type;
        cfdata->picon_mime = cfdata->icon_mime;

        e_fm2_all_icons_update();
     }

   return 1; /* Apply was OK */
}

#if 0
static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   return 1; /* Apply was OK */
}

#endif

/**--GUI--**/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *ot, *ob, *of, *oi, *win;
   E_Radio_Group *rg;
   char buf[PATH_MAX];
   const char *itype = NULL;
   int mh, row = 0;

   win = cfd->dia->win;
   snprintf(buf, sizeof(buf), "%s/%s",
            cfdata->location, cfdata->fi->file);
   o = e_widget_table_add(e_win_evas_win_get(evas), 0);

   ot = e_widget_table_add(e_win_evas_win_get(evas), 0);

   of = e_widget_frametable_add(evas, _("General"), 1);

   ob = e_widget_label_add(evas, _("Name:"));
   e_widget_frametable_object_append(of, ob, 0, row, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(win, &(cfdata->file), NULL, NULL, NULL);
   e_widget_size_min_get(ob, NULL, &mh);
   e_widget_size_min_set(ob, 140, mh);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);
   row++;

   ob = e_widget_label_add(evas, _("Location:"));
   e_widget_frametable_object_append(of, ob, 0, row, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(win, &(cfdata->location), NULL, NULL, NULL);
   e_widget_size_min_get(ob, NULL, &mh);
   e_widget_size_min_set(ob, 140, mh);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);
   row++;

   ob = e_widget_label_add(evas, _("Size:"));
   e_widget_frametable_object_append(of, ob, 0, row, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(win, &(cfdata->size), NULL, NULL, NULL);
   e_widget_size_min_get(ob, NULL, &mh);
   e_widget_size_min_set(ob, 140, mh);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);
   row++;

   ob = e_widget_label_add(evas, _("Occupied blocks on disk:"));
   e_widget_frametable_object_append(of, ob, 0, row, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(win, &(cfdata->blocks), NULL, NULL, NULL);
   e_widget_size_min_get(ob, NULL, &mh);
   e_widget_size_min_set(ob, 140, mh);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);
   row++;

   if (cfdata->exif_date)
     {
        ob = e_widget_label_add(evas, _("Taken on:"));
        e_widget_frametable_object_append(of, ob, 0, row, 1, 1, 1, 0, 1, 0);
        ob = e_widget_entry_add(win, &(cfdata->exif_date), NULL, NULL, NULL);
        e_widget_size_min_get(ob, NULL, &mh);
        e_widget_size_min_set(ob, 140, mh);
        e_widget_entry_readonly_set(ob, 1);
        e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);
        row++;
     }

   ob = e_widget_label_add(evas, _("Last Accessed:"));
   e_widget_frametable_object_append(of, ob, 0, row, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(win, &(cfdata->acc_date), NULL, NULL, NULL);
   e_widget_size_min_get(ob, NULL, &mh);
   e_widget_size_min_set(ob, 140, mh);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);
   row++;

   ob = e_widget_label_add(evas, _("Last Modified:"));
   e_widget_frametable_object_append(of, ob, 0, row, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(win, &(cfdata->mod_date), NULL, NULL, NULL);
   e_widget_size_min_get(ob, NULL, &mh);
   e_widget_size_min_set(ob, 140, mh);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);
   row++;

   ob = e_widget_label_add(evas, _("Last Modified Permissions:"));
   e_widget_frametable_object_append(of, ob, 0, row, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(win, &(cfdata->pms_date), NULL, NULL, NULL);
   e_widget_size_min_get(ob, NULL, &mh);
   e_widget_size_min_set(ob, 140, mh);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);
   row++;

   ob = e_widget_label_add(evas, _("File Type:"));
   e_widget_frametable_object_append(of, ob, 0, row, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(win, &(cfdata->mime), NULL, NULL, NULL);
   e_widget_size_min_get(ob, NULL, &mh);
   e_widget_size_min_set(ob, 140, mh);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);
   row++;

   e_widget_table_object_append(ot, of, 0, 0, 1, 1, 1, 1, 1, 1);
   e_widget_table_object_append(o, ot, 1, 0, 0, 1, 1, 1, 1, 1);


   of = e_widget_frametable_add(evas, _("Permissions"), 0);
   ob = e_widget_entry_add(win, &(cfdata->owner), NULL, NULL, NULL);

   e_widget_entry_readonly_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("read"), &(cfdata->owner_read));
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("write"), &(cfdata->owner_write));
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("execute"), &(cfdata->owner_exec));
   e_widget_frametable_object_append(of, ob, 0, 3, 1, 1, 1, 1, 1, 1);

   ob = e_widget_label_add(evas, _("Group:"));
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("read"), &(cfdata->group_read));
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("write"), &(cfdata->group_write));
   e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("execute"), &(cfdata->group_exec));
   e_widget_frametable_object_append(of, ob, 1, 3, 1, 1, 1, 1, 1, 1);

   ob = e_widget_label_add(evas, _("Others:"));
   e_widget_frametable_object_append(of, ob, 2, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("read"), &(cfdata->others_read));
   e_widget_frametable_object_append(of, ob, 2, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("write"), &(cfdata->others_write));
   e_widget_frametable_object_append(of, ob, 2, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("execute"), &(cfdata->others_exec));
   e_widget_frametable_object_append(of, ob, 2, 3, 1, 1, 1, 1, 1, 1);

   e_widget_table_object_append(ot, of, 0, 1, 1, 1, 1, 1, 1, 1);
   e_widget_table_object_append(o, ot, 0, 0, 1, 1, 1, 1, 1, 1);


   of = e_widget_frametable_add(evas, _("Preview"), 0);

   ot = e_widget_table_add(e_win_evas_win_get(evas), 0);
   ob = e_widget_preview_add(evas, 128, 128);
   cfdata->gui.preview = ob;
   cfdata->gui.preview_table = ot;
   evas_object_smart_callback_add(ob, "preview_update",
                                  _cb_preview_update, cfdata);
   e_widget_table_object_append(ot, ob, 0, 0, 1, 1, 0, 0, 1, 1);
   e_widget_preview_thumb_set(ob, buf,
                              "e/desktop/background", 128, 128);
   e_widget_frametable_object_append(of, ot, 0, 0, 1, 1, 1, 1, 1, 1);

   e_widget_table_object_append(o, of, 1, 0, 1, 1, 1, 1, 1, 1);

   ot = e_widget_frametable_add(evas, _("Icon"), 0);

   ob = e_widget_button_add(evas, "", NULL, _cb_icon_sel, cfdata, cfd);
   cfdata->gui.icon_wid = ob;
   oi = e_fm2_icon_get(evas,
                       cfdata->ic,
                       NULL, NULL, 0, &itype);
   e_widget_button_icon_set(ob, oi);
   e_widget_frametable_object_append(ot, ob, 0, 0, 1, 3, 1, 1, 1, 1);

   if (itype)
     {
        if ((!strcmp(itype, "THEME_ICON")) ||
            (!strcmp(itype, "DESKTOP")) ||
            (!strcmp(itype, "FILE_TYPE")))
          {
             e_widget_disabled_set(ob, 1);
             cfdata->icon_type = 0;
          }
        else if (!strcmp(itype, "THUMB"))
          {
             cfdata->icon_type = 1;
             e_widget_disabled_set(ob, 1);
          }
        else if (!strcmp(itype, "CUSTOM"))
          cfdata->icon_type = 2;
     }
   else
     cfdata->icon_type = 0;
   cfdata->picon_type = cfdata->icon_type;

   rg = e_widget_radio_group_new(&cfdata->icon_type);
   ob = e_widget_radio_add(evas, _("Default"), 0, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(ot, ob, 1, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, _("Thumbnail"), 1, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(ot, ob, 1, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, _("Custom"), 2, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(ot, ob, 1, 2, 1, 1, 1, 1, 1, 1);

   cfdata->icon_mime = 1;
   if ((cfdata->fi->icon) || ((itype) && (!strcmp(itype, "DESKTOP"))))
     cfdata->icon_mime = 0;
   cfdata->picon_mime = cfdata->icon_mime;
   if (cfdata->mime)
     {
        ob = e_widget_check_add(evas, _("Use this icon for all files of this type"), &(cfdata->icon_mime));
        e_widget_frametable_object_append(ot, ob, 0, 3, 2, 1, 1, 1, 1, 1);
     }

   e_widget_table_object_append(o, ot, 0, 1, 1, 1, 1, 1, 1, 1);

   if ((cfdata->link) && ((cfdata->fi->real_link) || (cfdata->fi->broken_link)))
     {
        ot = e_widget_frametable_add(evas, _("Link Information"), 0);

        ob = e_widget_entry_add(win, &(cfdata->link), NULL, NULL, NULL);
        e_widget_frametable_object_append(ot, ob, 0, 0, 1, 1, 1, 0, 1, 0);

        if (cfdata->fi->broken_link)
          {
             ob = e_widget_label_add(evas, _("This link is broken."));
             e_widget_frametable_object_append(ot, ob, 0, 1, 1, 1, 1, 0, 1, 0);
          }

        e_widget_table_object_append(o, ot, 1, 1, 2, 1, 1, 1, 1, 1);
     }
   return o;
}

#if 0
static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for an advanced dialog */
   Evas_Object *o;

   o = e_widget_table_add(e_win_evas_win_get(evas), 0);
   return o;
}

#endif

static void
_dia_del(void *data)
{
   E_Dialog *dia = data;
   E_Config_Dialog_Data *cfdata;

   cfdata = dia->data;
   if (!cfdata) return;
   cfdata->gui.fsel = NULL;
   cfdata->gui.fsel_wid = NULL;
}

static void
_cb_icon_sel(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   E_Config_Dialog *cfd;
   E_Dialog *dia;
   Evas_Object *o;
   Evas_Coord w, h;

   cfdata = data;
   if (!cfdata) return;
   if (cfdata->gui.fsel) return;

   cfd = data2;
   if (!cfd) return;

   dia = e_dialog_new(NULL, "E", "_fm2_file_properties_icon_select_dialog");
   if (!dia) return;
//   if (cfdata->type == EDJ)
//     e_dialog_title_set(dia, _("Select an Edj File"));
//   else if (cfdata->type == IMG)
   e_dialog_title_set(dia, _("Select an Image"));

   dia->data = cfdata;
   o = e_widget_fsel_add(evas_object_evas_get(dia->win), "~/", "/", NULL, NULL,
                         _cb_fsel_sel, cfdata, NULL, cfdata, 1);

   cfdata->gui.fsel_wid = o;
   evas_object_show(o);
   e_widget_size_min_get(o, &w, &h);
   e_dialog_content_set(dia, o, w, h);

   e_dialog_button_add(dia, _("OK"), NULL, _cb_fsel_ok, cfdata);
   e_dialog_button_add(dia, _("Cancel"), NULL, _cb_fsel_cancel, cfdata);
   e_util_win_auto_resize_fill(dia->win);
   elm_win_center(dia->win, 1, 1);
   e_dialog_show(dia);
   e_object_del_attach_func_set(E_OBJECT(dia), _dia_del);

   cfdata->gui.fsel = dia;
}

static void
_cb_type(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   if (cfdata->icon_type == 2)
     e_widget_disabled_set(cfdata->gui.icon_wid, 0);
   else
     e_widget_disabled_set(cfdata->gui.icon_wid, 1);
}

static void
_cb_preview_update(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   e_widget_table_object_repack(cfdata->gui.preview_table,
                                cfdata->gui.preview,
                                0, 0, 1, 1, 0, 0, 1, 1);
}

static void
_cb_fsel_sel(void *data, Evas_Object *obj EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   if (!cfdata) return;
}

static void
_cb_fsel_ok(void *data, E_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata;
   const char *file, *ext;
   Evas_Object *icon = NULL;

   cfdata = data;
   if (!cfdata) return;

   file = e_widget_fsel_selection_path_get(cfdata->gui.fsel_wid);
   E_FREE(cfdata->icon);
   if (file) cfdata->icon = strdup(file);
   _cb_fsel_cancel(data, dia);
   ext = strrchr(cfdata->icon, '.');
   if (ext)
     {
        if (!strcasecmp(ext, ".edj"))
          {
             icon = edje_object_add(evas_object_evas_get(cfdata->gui.icon_wid));
             edje_object_file_set(icon, cfdata->file, "icon");
          }
        else
          {
             icon = e_widget_image_add_from_file(evas_object_evas_get(cfdata->gui.icon_wid), cfdata->icon, 48, 48);
          }
     }
   else
     {
        icon = e_widget_image_add_from_file(evas_object_evas_get(cfdata->gui.icon_wid), cfdata->icon, 48, 48);
     }
   if (icon) e_widget_button_icon_set(cfdata->gui.icon_wid, icon);
}

static void
_cb_fsel_cancel(void *data, E_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   e_object_del(E_OBJECT(dia));
   cfdata->gui.fsel = NULL;
}

