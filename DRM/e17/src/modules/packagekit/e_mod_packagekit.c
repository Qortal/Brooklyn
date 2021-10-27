#include "e.h"
#include "e_mod_main.h"
#include "e_mod_packagekit.h"

/* GUI */
void
packagekit_icon_update(E_PackageKit_Module_Context *ctxt,
                       Eina_Bool working)
{
   E_PackageKit_Instance *inst;
   E_PackageKit_Package *pkg;
   unsigned count = 0;
   const char *state;
   char buf[16];
   Eina_List *l;

   if (!ctxt->instances)
     return;

   if (working)
     state = "packagekit,state,working";
   else if (ctxt->error)
     state = "packagekit,state,error";
   else
     {
        EINA_LIST_FOREACH(ctxt->packages, l, pkg)
          {
             switch (pkg->info)
               {
                  case PK_INFO_ENUM_LOW:
                  case PK_INFO_ENUM_ENHANCEMENT:
                  case PK_INFO_ENUM_NORMAL:
                  case PK_INFO_ENUM_BUGFIX:
                  case PK_INFO_ENUM_IMPORTANT:
                  case PK_INFO_ENUM_SECURITY:
                  case PK_INFO_ENUM_AVAILABLE:
                     count++;
                     break;
                  default:
                     break;
               }
          }

        if (count > 0)
          state = "packagekit,state,updates";
        else
          state = "packagekit,state,updated";
     }

   //DBG("PKGKIT: IconUpdate, %d updates available (%s)", count, state);

   if (count) snprintf(buf, sizeof(buf), "%d", count);
   EINA_LIST_FOREACH(ctxt->instances, l, inst)
     {
        edje_object_signal_emit(inst->gadget, state, "e");
        edje_object_part_text_set(inst->gadget, "num_updates", count ? buf : "");
     }
}

static void
_update_button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                  void *event EINA_UNUSED)
{
   E_PackageKit_Instance *inst = data;

   packagekit_create_transaction_and_exec(inst->ctxt, packagekit_refresh_cache);
}

static char *
_help_gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   PackageKit_Package_Info info = (PackageKit_Package_Info)data;

   if (strcmp(part, "elm.text"))
      return NULL;

   switch (info)
     {
        case PK_INFO_ENUM_LOW:
          return strdup(_("Low priority update"));
        case PK_INFO_ENUM_ENHANCEMENT:
          return strdup(_("Enhancement update"));
        case PK_INFO_ENUM_NORMAL:
          return strdup(_("Normal update"));
        case PK_INFO_ENUM_BUGFIX:
          return strdup(_("Bugfix update"));
        case PK_INFO_ENUM_IMPORTANT:
          return strdup(_("High priority update"));
        case PK_INFO_ENUM_SECURITY:
          return strdup(_("Security update"));
        default:
          return NULL;
     }
}

static Evas_Object *
_help_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
   PackageKit_Package_Info info = (PackageKit_Package_Info)data;
   const char *emblem_name;
   Evas_Object *icon;

   if (strcmp(part, "elm.swallow.icon"))
      return NULL;

   switch (info)
     {
        case PK_INFO_ENUM_LOW:
          emblem_name = "e/modules/packagekit/icon/low"; break;
        case PK_INFO_ENUM_ENHANCEMENT:
          emblem_name = "e/modules/packagekit/icon/enhancement"; break;
        case PK_INFO_ENUM_NORMAL:
          emblem_name = "e/modules/packagekit/icon/normal"; break;
        case PK_INFO_ENUM_BUGFIX:
          emblem_name = "e/modules/packagekit/icon/bugfix"; break;
        case PK_INFO_ENUM_IMPORTANT:
          emblem_name = "e/modules/packagekit/icon/important"; break;
        case PK_INFO_ENUM_SECURITY:
          emblem_name = "e/modules/packagekit/icon/security"; break;
        default:
          return NULL; break;
     }

   icon = edje_object_add(evas_object_evas_get(obj));
   e_theme_edje_object_set(icon, "base/theme/modules/packagekit", emblem_name);

   return icon;
}

static void
_help_button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event EINA_UNUSED)
{
   E_PackageKit_Instance *inst = data;
   Elm_Genlist_Item_Class *help_itc;
   char buf[1024];
   long i;

   if (inst->popup_help_mode)
     {
        inst->popup_help_mode = EINA_FALSE;
        packagekit_popup_update(inst, EINA_TRUE);
        return;
     }
   inst->popup_help_mode = EINA_TRUE;

   // special item class for help items
   help_itc = elm_genlist_item_class_new();
   help_itc->item_style = "default";
   help_itc->func.text_get = _help_gl_text_get;
   help_itc->func.content_get = _help_gl_content_get;

   // repopulate the genlist
   elm_genlist_clear(inst->popup_genlist);
   for (i = PK_INFO_ENUM_LOW; i <= PK_INFO_ENUM_SECURITY; i++)
     {
        Elm_Genlist_Item *it;
        it = elm_genlist_item_append(inst->popup_genlist, help_itc, (void*)i,
                                     NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
        elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
     }

   elm_genlist_item_class_free(help_itc);

   // update title label
   if (inst->ctxt->v_maj != -1)
     snprintf(buf, sizeof(buf), "PackageKit %d.%d.%d",
              inst->ctxt->v_maj, inst->ctxt->v_min, inst->ctxt->v_mic);
   else
     snprintf(buf, sizeof(buf), _("Unknown PackageKit"));
   elm_object_text_set(inst->popup_title_entry, buf);
}

static void
_install_button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                   void *event EINA_UNUSED)
{
   E_PackageKit_Instance *inst = data;
   E_PackageKit_Package *pkg;
   const Eina_List *selected;
   Elm_Genlist_Item *item;
   Eina_List *l;

   selected = elm_genlist_selected_items_get(inst->popup_genlist);
   if (!selected)
     {
        // nothing selected, update all packages
        EINA_LIST_FOREACH(inst->ctxt->packages, l, pkg)
          pkg->to_be_installed = EINA_TRUE;
     }
   else
     {
        // only updated selected packages
        EINA_LIST_FOREACH(inst->ctxt->packages, l, pkg)
          pkg->to_be_installed = EINA_FALSE;
        EINA_LIST_FOREACH((Eina_List*)selected, l, item)
          {
             pkg = elm_object_item_data_get(item);
             pkg->to_be_installed = EINA_TRUE;
          }
     }

   packagekit_create_transaction_and_exec(inst->ctxt, packagekit_update_packages);
}

static void
_run_button_cb(void *data, Evas_Object *obj EINA_UNUSED,
               void *event EINA_UNUSED)
{
   E_PackageKit_Instance *inst = data;
   packagekit_popup_del(inst);

   e_exec(e_zone_current_get(), NULL,
          inst->ctxt->config->manager_command,
          NULL, NULL);
}

void
packagekit_popup_update(E_PackageKit_Instance *inst, Eina_Bool rebuild_list)
{
   E_PackageKit_Module_Context *ctxt = inst->ctxt;
   E_PackageKit_Package *pkg;
   const Eina_List *selected;
   unsigned num_updates = 0;
   char buf[1024];
   Eina_List *l;

   if (!inst->popup_genlist) return;

   if (inst->popup_help_mode)
     inst->popup_help_mode = EINA_FALSE;

   if (rebuild_list)
     elm_genlist_clear(inst->popup_genlist);

   if (ctxt->error)
     {
        elm_object_text_set(inst->popup_title_entry, _("No information available"));
        elm_object_text_set(inst->popup_error_label, ctxt->error);
        if ((ctxt->v_maj != -1) && (ctxt->v_min != -1) && (ctxt->v_mic != -1))
          {
             snprintf(buf, sizeof(buf), "<ps/>PackageKit version: %d.%d.%d",
                      ctxt->v_maj, ctxt->v_min, ctxt->v_mic);
             elm_entry_entry_append(inst->popup_error_label, buf);
          }
        return;
     }

   EINA_LIST_FOREACH(ctxt->packages, l, pkg)
     {
        if (rebuild_list)
          elm_genlist_item_append(inst->popup_genlist, inst->popup_genlist_itc,
                                  pkg, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
        num_updates++;
     }

   // show the progress bar if an operation is in progress
   if (ctxt->transaction)
     {
        elm_genlist_clear(inst->popup_genlist);
        elm_progressbar_value_set(inst->popup_progressbar,
                                  ctxt->transaction_progress);
        evas_object_show(inst->popup_progressbar_frame);
     }
   else
     evas_object_hide(inst->popup_progressbar_frame);

   // update title and error lables
   if (ctxt->transaction)
      snprintf(buf, sizeof(buf), _("Operation in progress"));
   else if (num_updates >= 1)
     snprintf(buf, sizeof(buf), P_("One update available", "%d updates available", num_updates), num_updates);
   else
     snprintf(buf, sizeof(buf), _("Your system is updated"));
   elm_object_text_set(inst->popup_title_entry, buf);
   elm_object_text_set(inst->popup_error_label, "");

   // update the status of the install button
   selected = elm_genlist_selected_items_get(inst->popup_genlist);
   if (ctxt->transaction)
     {
        elm_object_text_set(inst->popup_install_button, _("Please wait"));
        elm_object_disabled_set(inst->popup_install_button, EINA_TRUE);
     }
   else if (num_updates < 1)
     {
        elm_object_text_set(inst->popup_install_button, _("Nothing to do"));
        elm_object_disabled_set(inst->popup_install_button, EINA_TRUE);
     }
   else if ((selected == NULL) || (eina_list_count(selected) == 0))
     {
        elm_object_text_set(inst->popup_install_button,
                           _("Install updates"));
        elm_object_disabled_set(inst->popup_install_button, EINA_FALSE);
     }
   else if (eina_list_count(selected) > 0)
     {
        snprintf(buf, sizeof(buf), P_("Install update",
                                      "Install %d updates",
                                      eina_list_count(selected)),
                                      eina_list_count(selected));
        elm_object_text_set(inst->popup_install_button, buf);
        elm_object_disabled_set(inst->popup_install_button, EINA_FALSE);
     }
}

void
packagekit_all_popups_update(E_PackageKit_Module_Context *ctxt,
                             Eina_Bool rebuild_list)
{
   E_PackageKit_Instance *inst;
   Eina_List *l;

   EINA_LIST_FOREACH(ctxt->instances, l, inst)
     packagekit_popup_update(inst, rebuild_list);
}

void
packagekit_progress_percentage_update(E_PackageKit_Module_Context *ctxt,
                                      int percent)
{
   E_PackageKit_Instance *inst;
   Eina_List *l;
   double val = (double)percent / 100.0;

   ctxt->transaction_progress = val;
   EINA_LIST_FOREACH(ctxt->instances, l, inst)
     if (inst->popup_progressbar)
       elm_progressbar_value_set(inst->popup_progressbar, val);
}

static void
_popup_del_cb(void *obj)
{
   packagekit_popup_del(e_object_data_get(obj));
}

static char *
_gl_item_single_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   E_PackageKit_Package *pkg = data;

   if (!strcmp(part, "elm.text"))
     {
        char *s = malloc(strlen(pkg->name) + strlen(pkg->version) + 2);
        sprintf(s, "%s %s", pkg->name, pkg->version);
        return s;
     }

   return NULL;
}

static char *
_gl_item_double_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   E_PackageKit_Package *pkg = data;

   if (!strcmp(part, "elm.text"))
     {
        char *s = malloc(strlen(pkg->name) + strlen(pkg->version) + 2);
        sprintf(s, "%s %s", pkg->name, pkg->version);
        return s;
     }
   else
     {
        return strdup(pkg->summary);
     }

   return NULL;
}

static Evas_Object *
_gl_item_content_get(void *data, Evas_Object *obj, const char *part)
{
   E_PackageKit_Package *pkg = data;
   Efreet_Desktop *desktop;
   Evas_Object *icon;

   if (!strcmp(part, "elm.swallow.icon"))
     {
        // get the priority icon from the theme
        const char *emblem_name;

        switch (pkg->info)
          {
             case PK_INFO_ENUM_LOW:
               emblem_name = "e/modules/packagekit/icon/low"; break;
             case PK_INFO_ENUM_ENHANCEMENT:
               emblem_name = "e/modules/packagekit/icon/enhancement"; break;
             case PK_INFO_ENUM_NORMAL:
               emblem_name = "e/modules/packagekit/icon/normal"; break;
             case PK_INFO_ENUM_BUGFIX:
               emblem_name = "e/modules/packagekit/icon/bugfix"; break;
             case PK_INFO_ENUM_IMPORTANT:
               emblem_name = "e/modules/packagekit/icon/important"; break;
             case PK_INFO_ENUM_SECURITY:
               emblem_name = "e/modules/packagekit/icon/security"; break;
             default:
               emblem_name = NULL; break;
          }
        if (emblem_name)
          {
            icon = edje_object_add(evas_object_evas_get(obj));
            e_theme_edje_object_set(icon, "base/theme/modules/packagekit", emblem_name);
            return icon;
          }
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        // try to find a desktop file that match the executable or the name
        desktop = efreet_util_desktop_exec_find(pkg->name);
        if (!desktop)
          desktop = efreet_util_desktop_name_find(pkg->name);

        if (desktop && desktop->icon)
          {
             icon = elm_icon_add(obj);
             elm_icon_standard_set(icon, desktop->icon);
             efreet_desktop_free(desktop);
             return icon;
          }
     }

   return NULL;
}

static void
_genlist_selunsel_cb(void *data, Evas_Object *obj EINA_UNUSED,
                     void *event EINA_UNUSED)
{
   packagekit_popup_update(data, EINA_FALSE);
}

static void
_popup_autoclose_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   packagekit_popup_del(data);
}

void
packagekit_popup_new(E_PackageKit_Instance *inst)
{
   Evas_Object *table, *bt, *ic, *lb, *li, *pb, *fr, *bx, *size_rect;
   const char *p;

   inst->popup = e_gadcon_popup_new(inst->gcc, EINA_FALSE);
   table = elm_table_add(e_comp->elm);
   evas_object_show(table);

   fr = elm_frame_add(table);
   elm_object_style_set(fr, "pad_large");
   evas_object_size_hint_expand_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(fr, EVAS_HINT_FILL, 0.0);
   elm_table_pack(table, fr, 0, 0, 1, 1);
   evas_object_show(fr);

   // horiz box for title and buttons
   bx = elm_box_add(table);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_expand_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bx, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   // title label
   lb = inst->popup_title_entry = elm_entry_add(table);
   elm_entry_editable_set(lb, 0);
   evas_object_size_hint_expand_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
   elm_entry_text_style_user_push(lb, "DEFAULT='font_weight=Bold'");
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   // refresh button
   ic = elm_icon_add(table);
   evas_object_size_hint_min_set(ic, 20 * elm_config_scale_get(),
                                     20 * elm_config_scale_get());
   elm_icon_standard_set(ic, "view-refresh");
   bt = elm_button_add(table);
   elm_object_content_set(bt, ic);
   evas_object_smart_callback_add(bt, "clicked", _update_button_cb, inst);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   // help button
   ic = elm_icon_add(table);
   evas_object_size_hint_min_set(ic, 20 * elm_config_scale_get(),
                                     20 * elm_config_scale_get());
   elm_icon_standard_set(ic, "help-contents");
   bt = elm_button_add(table);
   elm_object_content_set(bt, ic);
   evas_object_smart_callback_add(bt, "clicked", _help_button_cb, inst);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   // central area (sizer)
   size_rect = evas_object_rectangle_add(evas_object_evas_get(table));
   evas_object_size_hint_min_set(size_rect, 240 * elm_config_scale_get(),
                                            240 * elm_config_scale_get());
   elm_table_pack(table, size_rect, 0, 1, 1, 1);

   // central area (error label)
   lb = inst->popup_error_label = elm_entry_add(table);
   elm_entry_editable_set(lb, EINA_FALSE);
   E_EXPAND(lb);
   E_FILL(lb);
   elm_table_pack(table, lb, 0, 1, 1, 1);
   evas_object_show(lb);

   // central area (genlist)
   inst->popup_genlist_itc = elm_genlist_item_class_new();
   if (inst->ctxt->config->show_description)
     {
        inst->popup_genlist_itc->item_style = "double_label";
        inst->popup_genlist_itc->func.text_get = _gl_item_double_text_get;
     }
   else
     {
        inst->popup_genlist_itc->item_style = "default";
        inst->popup_genlist_itc->func.text_get = _gl_item_single_text_get;
     }
   inst->popup_genlist_itc->func.content_get = _gl_item_content_get;

   li = inst->popup_genlist = elm_genlist_add(table);
   elm_genlist_homogeneous_set(li, EINA_TRUE);
   elm_genlist_mode_set(li, ELM_LIST_COMPRESS);
   elm_genlist_multi_select_set(li, EINA_TRUE);
   E_EXPAND(li);
   E_FILL(li);
   evas_object_smart_callback_add(li, "selected", _genlist_selunsel_cb, inst);
   evas_object_smart_callback_add(li, "unselected", _genlist_selunsel_cb, inst);
   elm_table_pack(table, li, 0, 1, 1, 1);
   evas_object_show(li);

   // central area (progress bar) (inside a padding frame)
   fr = inst->popup_progressbar_frame = elm_frame_add(table);
   elm_object_style_set(fr, "pad_large");
   E_EXPAND(fr);
   E_FILL(fr);
   elm_table_pack(table, fr, 0, 1, 1, 1);
   evas_object_show(fr);

   pb = inst->popup_progressbar = elm_progressbar_add(table);
   E_EXPAND(pb);
   E_FILL(pb);
   elm_object_content_set(fr, pb);

   fr = elm_frame_add(table);
   elm_object_style_set(fr, "pad_large");
   evas_object_size_hint_expand_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(fr, EVAS_HINT_FILL, 0.0);
   elm_table_pack(table, fr, 0, 2, 1, 1);
   evas_object_show(fr);

   // horiz box for title and buttons
   bx = elm_box_add(table);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_box_homogeneous_set(bx, EINA_TRUE);
   evas_object_size_hint_expand_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bx, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   // install button
   bt = inst->popup_install_button = elm_button_add(table);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _install_button_cb, inst);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   // run package manager button (only if configured)
   p = inst->ctxt->config->manager_command;
   if (p && p[0])
     {
        bt = elm_button_add(table);
        evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, 0.0);
        elm_object_text_set(bt, _("Manage packages"));
        evas_object_smart_callback_add(bt, "clicked", _run_button_cb, inst);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);
     }

   // setup and show the popup
   e_gadcon_popup_content_set(inst->popup, table);
   e_object_data_set(E_OBJECT(inst->popup), inst);
   E_OBJECT_DEL_SET(inst->popup, _popup_del_cb);
   e_comp_object_util_autoclose(inst->popup->comp_object,
                                _popup_autoclose_cb, NULL, inst);
   e_gadcon_popup_show(inst->popup);

   // update the popup state and contents
   packagekit_popup_update(inst, EINA_TRUE);
}

void
packagekit_popup_del(E_PackageKit_Instance *inst)
{
   if (inst->popup)
     E_FREE_FUNC(inst->popup, e_object_del);

   inst->popup_genlist = inst->popup_title_entry = NULL;
   inst->popup_progressbar = inst->popup_progressbar_frame = NULL;
   if (inst->popup_genlist_itc)
     {
        elm_genlist_item_class_free(inst->popup_genlist_itc);
        inst->popup_genlist_itc = NULL;
     }
}

static void
_store_error(E_PackageKit_Module_Context *ctxt, const char *err)
{
   ERR("PKGKIT: ERROR: %s", err);
   if (ctxt->error)
      eina_stringshare_replace(&ctxt->error, err);
   else
      ctxt->error = eina_stringshare_add(err);
   packagekit_icon_update(ctxt, EINA_FALSE);
   packagekit_all_popups_update(ctxt, EINA_FALSE);
}

/* DBus PackageKit method calls */

static void
null_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   E_PackageKit_Module_Context *ctxt = data;
   const char *error, *error_msg;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     _store_error(ctxt, error_msg);
}


/* RefreshCache(in'b'force) */
static void
_signal_repo_detail_cb(void *data, const Eldbus_Message *msg)
{ /* RepoDetail ('s'repo_id, 's'description, 'b'enabled) */
   E_PackageKit_Module_Context *ctxt = data;
   const char *error, *error_msg, *repo_id, *desc;
   Eina_Bool enabled;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        _store_error(ctxt, error_msg);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "ssb", &repo_id, &desc, &enabled))
     {
        _store_error(ctxt, "could not get arguments (ssb)");
        return;
     }
   //DBG("PKGKIT: RepoDetail: (%d) %s [ %s ]", enabled, repo_id, desc);
}

static void
_signal_cache_finished_cb(void *data, const Eldbus_Message *msg)
{  /* Finished ('u'exit, 'u'runtime) */
   E_PackageKit_Module_Context *ctxt = data;
   const char *error, *error_msg;

   //DBG("PKGKIT: Cache Finished CB");

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        _store_error(ctxt, error_msg);
        return;
     }

   Eldbus_Object *obj = eldbus_proxy_object_get(ctxt->transaction);
   E_FREE_FUNC(ctxt->transaction, eldbus_proxy_unref);
   E_FREE_FUNC(obj, eldbus_object_unref);

   packagekit_create_transaction_and_exec(ctxt, packagekit_get_updates);
}

void
packagekit_refresh_cache(E_PackageKit_Module_Context *ctxt, const char *transaction)
{
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;
   Eldbus_Pending *pending;

   ctxt->config->last_update = ecore_time_unix_get();

   obj = eldbus_object_get(ctxt->conn, "org.freedesktop.PackageKit", transaction);
   proxy = eldbus_proxy_get(obj, "org.freedesktop.PackageKit.Transaction");
   pending = eldbus_proxy_call(proxy, "RefreshCache", null_cb, ctxt, -1, "b", 1);
   if (!pending)
     {
        _store_error(ctxt, "could not call RefreshCache()");
        return;
     }
   eldbus_proxy_signal_handler_add(proxy, "Finished",
                                   _signal_cache_finished_cb, ctxt);
   eldbus_proxy_signal_handler_add(proxy, "RepoDetail",
                                   _signal_repo_detail_cb, ctxt);
   ctxt->transaction = proxy;
}


/* GetUpdates(in't'filter) */
static void
_signal_package_cb(void *data, const Eldbus_Message *msg)
{  /* Package ('u'info, 's'package_id, 's'summary) */
   const char *error, *error_msg, *pkg_id, *summary, *info_str;
   E_PackageKit_Module_Context *ctxt = data;
   PackageKit_Package_Info info;
   unsigned num_elements = 0;
   char **splitted;
   Eina_Bool ret;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        _store_error(ctxt, error_msg);
        return;
     }
   if (PKITV07)
     ret = eldbus_message_arguments_get(msg, "sss", &info_str, &pkg_id, &summary);
   else
     ret = eldbus_message_arguments_get(msg, "uss", &info, &pkg_id, &summary);
   if (!ret)
     {
        _store_error(ctxt, "could not get package arguments");
        return;
     }
   //if (PKITV07)
     //{ DBG("PKGKIT: Package: (%s) %s [ %s ]", info_str, pkg_id, summary); }
   //else
     //{ DBG("PKGKIT: Package: (%d) %s [ %s ]", info, pkg_id, summary); }

   splitted = eina_str_split_full(pkg_id, ";", 3, &num_elements);
   if (num_elements >= 2)
     {
        E_PackageKit_Package *pkg = E_NEW(E_PackageKit_Package, 1);
        pkg->pkg_id = eina_stringshare_add(pkg_id);
        pkg->name = eina_stringshare_add(splitted[0]);
        pkg->version = eina_stringshare_add(splitted[1]);
        pkg->summary = eina_stringshare_add(summary);
        if (PKITV07)
          {
             if (!info_str) pkg->info = PK_INFO_ENUM_NORMAL;
             else if (!strcmp(info_str, "normal"))      pkg->info = PK_INFO_ENUM_NORMAL;
             else if (!strcmp(info_str, "security"))    pkg->info = PK_INFO_ENUM_SECURITY;
             else if (!strcmp(info_str, "blocked"))     pkg->info = PK_INFO_ENUM_BLOCKED;
             else if (!strcmp(info_str, "low"))         pkg->info = PK_INFO_ENUM_LOW;
             else if (!strcmp(info_str, "enhancement")) pkg->info = PK_INFO_ENUM_ENHANCEMENT;
             else if (!strcmp(info_str, "bugfix"))      pkg->info = PK_INFO_ENUM_BUGFIX;
             else if (!strcmp(info_str, "important"))   pkg->info = PK_INFO_ENUM_IMPORTANT;
             else pkg->info = PK_INFO_ENUM_UNKNOWN;
          }
        else
          pkg->info = info;
        ctxt->packages = eina_list_append(ctxt->packages, pkg);
     }
   if (splitted)
     {
        free(splitted[0]);
        free(splitted);
     }
}

static void
_signal_finished_cb(void *data, const Eldbus_Message *msg)
{  /* Finished ('u'exit, 'u'runtime) */
   const char *error, *error_msg;
   E_PackageKit_Module_Context *ctxt = data;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        _store_error(ctxt, error_msg);
        return;
     }

   Eldbus_Object *obj = eldbus_proxy_object_get(ctxt->transaction);
   E_FREE_FUNC(ctxt->transaction, eldbus_proxy_unref);
   E_FREE_FUNC(obj, eldbus_object_unref);
   E_FREE_FUNC(ctxt->error, eina_stringshare_del);

   //DBG("PKGKIT: PackageFinished");
   packagekit_icon_update(ctxt, EINA_FALSE);
   packagekit_all_popups_update(ctxt, EINA_TRUE);
}

void
packagekit_get_updates(E_PackageKit_Module_Context *ctxt, const char *transaction)
{
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;
   Eldbus_Pending *pending;
   E_PackageKit_Package *pkg;

   obj = eldbus_object_get(ctxt->conn, "org.freedesktop.PackageKit", transaction);
   proxy = eldbus_proxy_get(obj, "org.freedesktop.PackageKit.Transaction");
   if (PKITV07)
     pending = eldbus_proxy_call(proxy, "GetUpdates", null_cb, ctxt, -1, "s", "none");
   else
     pending = eldbus_proxy_call(proxy, "GetUpdates", null_cb, ctxt, -1, "t", 1);
   if (!pending)
     {
        _store_error(ctxt, "could not call GetUpdates()");
        return;
     }

   eldbus_proxy_signal_handler_add(proxy, "Package", _signal_package_cb, ctxt);
   eldbus_proxy_signal_handler_add(proxy, "Finished", _signal_finished_cb, ctxt);
   ctxt->transaction = proxy;

   EINA_LIST_FREE(ctxt->packages, pkg)
     {
        E_FREE_FUNC(pkg->pkg_id, eina_stringshare_del);
        E_FREE_FUNC(pkg->name, eina_stringshare_del);
        E_FREE_FUNC(pkg->version, eina_stringshare_del);
        E_FREE_FUNC(pkg->summary, eina_stringshare_del);
     }
}


/* UpdatePackages(in't'transaction_flags, in'as'package_ids) */
static void
_signal_update_error_code_cb(void *data, const Eldbus_Message *msg)
{  /* ErrorCode ('u'code, 's'details) */
   const char *error, *error_msg, *details;
   E_PackageKit_Module_Context *ctxt = data;
   Eina_Bool ret;
   int err_code;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        _store_error(ctxt, error_msg);
        return;
     }

   ret = eldbus_message_arguments_get(msg, "us", &err_code, &details);
   if (!ret)
     {
        _store_error(ctxt, "could not get error code arguments");
        return;
     }

   if (details)
     _store_error(ctxt, details);
}

static void
_signal_update_finished_cb(void *data, const Eldbus_Message *msg)
{  /* Finished ('u'exit, 'u'runtime) */
   const char *error, *error_msg;
   E_PackageKit_Module_Context *ctxt = data;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     _store_error(ctxt, error_msg);
   else
     E_FREE_FUNC(ctxt->error, eina_stringshare_del);

   Eldbus_Object *obj = eldbus_proxy_object_get(ctxt->transaction);
   E_FREE_FUNC(ctxt->transaction, eldbus_proxy_unref);
   E_FREE_FUNC(obj, eldbus_object_unref);
}

void
packagekit_update_packages(E_PackageKit_Module_Context *ctxt, const char *transaction)
{
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *array_of_string;
   Eldbus_Pending *pending;
   E_PackageKit_Package *pkg;
   Eina_List *l;

   fprintf(stderr, "PKIT: UpdatePackages (t:%s)\n", transaction);

   obj = eldbus_object_get(ctxt->conn, "org.freedesktop.PackageKit", transaction);
   proxy = eldbus_proxy_get(obj, "org.freedesktop.PackageKit.Transaction");
   msg = eldbus_proxy_method_call_new(proxy, "UpdatePackages");
   iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_arguments_append(iter, "tas",
                                        PK_TRANSACTION_FLAG_ENUM_ONLY_TRUSTED,
                                        &array_of_string);
   EINA_LIST_FOREACH(ctxt->packages, l, pkg)
     {
        if (pkg->to_be_installed)
          {
             DBG("Install: %s %s", pkg->pkg_id, pkg->version);
             eldbus_message_iter_arguments_append(array_of_string, "s", pkg->pkg_id);
          }
     }
   eldbus_message_iter_container_close(iter, array_of_string);

   pending = eldbus_proxy_send(proxy, msg, null_cb, ctxt, -1);
   if (!pending)
     {
        _store_error(ctxt, "could not call UpdatePackages()");
        return;
     }
   eldbus_proxy_signal_handler_add(proxy, "ErrorCode",
                                   _signal_update_error_code_cb, ctxt);
   eldbus_proxy_signal_handler_add(proxy, "Finished",
                                   _signal_update_finished_cb, ctxt);
   ctxt->transaction = proxy;
}

/* CreateTransaction() */
static void
_transaction_changed_props_iter(void *data, const void *key,
                                Eldbus_Message_Iter *val)
{
   E_PackageKit_Module_Context *ctxt = data;

   if (!strcmp(key, "Percentage"))
     {
        int val_int;
        eldbus_message_iter_basic_get(val, &val_int);
        packagekit_progress_percentage_update(ctxt, val_int);
     }
}

static void
_signal_transaction_props_changed_cb(void *data, const Eldbus_Message *msg)
{ /* PropertiesChanged('s'interface_name, 'a{sv}'changed_properties,
                       'as' invalidated_properties) */
   E_PackageKit_Module_Context *ctxt = data;
   const char *error, *error_msg, *interface_name;
   Eldbus_Message_Iter *changed_properties, *invalidated_properties;
   Eina_Bool ret;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        _store_error(ctxt, error_msg);
        return;
     }

   ret = eldbus_message_arguments_get(msg, "sa{sv}as", &interface_name,
                                      &changed_properties,
                                      &invalidated_properties);
   if (!ret)
     {
        _store_error(ctxt, "could not get signal arguments");
        return;
     }

   eldbus_message_iter_dict_iterate(changed_properties, "sv",
                                    _transaction_changed_props_iter, ctxt);
}

static void
_transaction_created_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   const char *error, *error_msg, *trans_path;
   E_PackageKit_Module_Context *ctxt = data;
   E_PackageKit_Transaction_Func func;
   Eldbus_Proxy *proxy;
   Eldbus_Object *obj;
   Eina_Bool ret;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        _store_error(ctxt, error_msg);
        return;
     }

   if (PKITV07)
     ret = eldbus_message_arguments_get(msg, "s", &trans_path);
   else
     ret = eldbus_message_arguments_get(msg, "o", &trans_path);
   if (!ret)
     {
        _store_error(ctxt, "could not get transaction path");
        return;
     }

   // monitor the "Percentage" property to update the progress bar
   obj = eldbus_object_get(ctxt->conn, "org.freedesktop.PackageKit", trans_path);
   proxy = eldbus_proxy_get(obj, "org.freedesktop.DBus.Properties");
   eldbus_proxy_signal_handler_add(proxy, "PropertiesChanged",
                                   _signal_transaction_props_changed_cb, ctxt);

   // call the operation function
   func = eldbus_pending_data_get(pending, "func");
   func(ctxt, trans_path);

   // update all the faces
   packagekit_icon_update(ctxt, EINA_TRUE);
   packagekit_all_popups_update(ctxt, EINA_FALSE);
}

void
packagekit_create_transaction_and_exec(E_PackageKit_Module_Context *ctxt,
                                       E_PackageKit_Transaction_Func func)
{
   Eldbus_Pending *pending;

   //DBG("PKGKIT: Version: %d.%d.%d", ctxt->v_maj, ctxt->v_min, ctxt->v_mic);

   if (ctxt->transaction)
     {
        WRN("PKGKIT: Another transaction in progress...");
        return;
     }

   pending = eldbus_proxy_call(ctxt->packagekit,
                               PKITV07 ? "GetTid" : "CreateTransaction",
                               _transaction_created_cb, ctxt, -1, "");
   if (!pending)
     {
        _store_error(ctxt, "could not call CreateTransaction()");
        return;
     }
   eldbus_pending_data_set(pending, "func", func);
}


/* PackageKit DBus */
static void
_signal_updates_changed_cb(void *data, const Eldbus_Message *msg EINA_UNUSED)
{
   E_PackageKit_Module_Context *ctxt = data;

   packagekit_create_transaction_and_exec(ctxt, packagekit_get_updates);
}

static void
_iterate_dict(void *data, const void *key, Eldbus_Message_Iter *var)
{
   E_PackageKit_Module_Context *ctxt = data;

   if (!strcmp(key, "VersionMajor"))
     eldbus_message_iter_arguments_get(var, "u", &(ctxt->v_maj));
   else if (!strcmp(key, "VersionMinor"))
     eldbus_message_iter_arguments_get(var, "u", &(ctxt->v_min));
   else if (!strcmp(key, "VersionMicro"))
     eldbus_message_iter_arguments_get(var, "u", &(ctxt->v_mic));
   else return;

   if ((ctxt->v_maj != -1) && (ctxt->v_min != -1) && (ctxt->v_mic != -1))
     packagekit_create_transaction_and_exec(ctxt, packagekit_get_updates);
}

static void
_prop_get_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   E_PackageKit_Module_Context *ctxt = data;
   Eldbus_Message_Iter *array;
   const char *error, *error_msg;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        _store_error(ctxt, error_msg);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "a{sv}", &array))
     {
        _store_error(ctxt, "could not get arguments (a{sv})");
        return;
     }
   eldbus_message_iter_dict_iterate(array, "sv", _iterate_dict, ctxt);
}

Eina_Bool
packagekit_dbus_connect(E_PackageKit_Module_Context *ctxt)
{
   Eldbus_Object *obj;

   //DBG("PKGKIT: dbus_init()");

   ctxt->conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   if (!ctxt->conn)
     {
        _store_error(ctxt, "could not connect to system bus");
        return EINA_FALSE;
     }

   obj = eldbus_object_get(ctxt->conn, "org.freedesktop.PackageKit",
                                       "/org/freedesktop/PackageKit");
   ctxt->packagekit = eldbus_proxy_get(obj, "org.freedesktop.PackageKit");
   if (!ctxt->packagekit)
   {
      _store_error(ctxt, "could not connect to PackageKit");
      return EINA_FALSE;
   }

   eldbus_proxy_property_get_all(ctxt->packagekit, _prop_get_cb, ctxt);
   eldbus_proxy_signal_handler_add(ctxt->packagekit, "UpdatesChanged",
                                   _signal_updates_changed_cb, ctxt);

   return EINA_TRUE;
}

void
packagekit_dbus_disconnect(E_PackageKit_Module_Context *ctxt)
{
   Eldbus_Object *obj;

   //DBG("PKGKIT: dbus_shutdown()");

   if (ctxt->transaction)
     {
        // TODO abort the transaction ???
        obj = eldbus_proxy_object_get(ctxt->transaction);
        E_FREE_FUNC(ctxt->transaction, eldbus_proxy_unref);
        E_FREE_FUNC(obj, eldbus_object_unref);
     }

   obj = eldbus_proxy_object_get(ctxt->packagekit);
   E_FREE_FUNC(ctxt->packagekit, eldbus_proxy_unref);
   E_FREE_FUNC(obj, eldbus_object_unref);

   eldbus_connection_unref(ctxt->conn);
}
