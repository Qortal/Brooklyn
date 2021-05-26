#include "e.h"


typedef struct _Match_Config
{
   E_Comp_Match            match;
   E_Config_Dialog *cfd;
   char            *title, *name, *clas, *role;
   int              borderless, dialog, accepts_focus;
   int              argb, fullscreen, modal, primary_type;
   int effect_type;
} Match_Config;

struct _E_Config_Dialog_Data
{
   Eina_List *popups;    // used for e popups
   Eina_List *borders;    // used for borders
   Eina_List *overrides;    // used for client menus, tooltips etc.
   Eina_List *menus;    // used for e menus
   Eina_List *objects;    // used for e objects
   Eina_List *comp_effects;    // list of visibility effects
   int        changed;

   Evas_Object *edit_il;

   Evas_Object *popups_il;
   Evas_Object *borders_il;
   Evas_Object *overrides_il;
   Evas_Object *menus_il;
   Evas_Object *objects_il;
};

static void
_match_dup(E_Comp_Match *m, Match_Config *m2)
{
   m2->match = *m;
   m2->match.title = eina_stringshare_ref(m2->match.title);
   m2->match.name = eina_stringshare_ref(m2->match.name);
   m2->match.clas = eina_stringshare_ref(m2->match.clas);
   m2->match.role = eina_stringshare_ref(m2->match.role);
   m2->primary_type = m2->match.primary_type;

   m2->match.shadow_style = eina_stringshare_ref(m2->match.shadow_style);
   m2->match.visibility_effect = eina_stringshare_ref(m2->match.visibility_effect);
}

static void
_match_free(Match_Config *m)
{
   eina_stringshare_del(m->match.title);
   eina_stringshare_del(m->match.name);
   eina_stringshare_del(m->match.clas);
   eina_stringshare_del(m->match.role);
   eina_stringshare_del(m->match.shadow_style);
   eina_stringshare_del(m->match.visibility_effect);
   free(m->title);
   free(m->name);
   free(m->clas);
   free(m->role);
   free(m);
}

static void
_match_dup2(Match_Config *m2, E_Comp_Match *m)
{
   *m = m2->match;
   m->title = eina_stringshare_add(m->title);
   m->name = eina_stringshare_add(m->name);
   m->clas = eina_stringshare_add(m->clas);
   m->role = eina_stringshare_add(m->role);
   m->shadow_style = eina_stringshare_add(m->shadow_style);
   m->visibility_effect = eina_stringshare_add(m->visibility_effect);
}

static const char *
_match_type_label_get(int type)
{
   if (E_WINDOW_TYPE_UNKNOWN == type)
     return _("Unused");
   if (E_WINDOW_TYPE_COMBO == type)
     return _("Combo");
   if (E_WINDOW_TYPE_DESKTOP == type)
     return _("Desktop");
   if (E_WINDOW_TYPE_DIALOG == type)
     return _("Dialog");
   if (E_WINDOW_TYPE_DOCK == type)
     return _("Dock");
   if (E_WINDOW_TYPE_DND == type)
     return _("Drag and Drop");
   if (E_WINDOW_TYPE_MENU == type)
     return _("Menu");
   if (E_WINDOW_TYPE_DROPDOWN_MENU == type)
     return _("Menu (Dropdown)");
   if (E_WINDOW_TYPE_POPUP_MENU == type)
     return _("Menu (Popup)");
   if (E_WINDOW_TYPE_NORMAL == type)
     return _("Normal");
   if (E_WINDOW_TYPE_NOTIFICATION == type)
     return _("Notification");
   if (E_WINDOW_TYPE_SPLASH == type)
     return _("Splash");
   if (E_WINDOW_TYPE_TOOLBAR == type)
     return _("Toolbar");
   if (E_WINDOW_TYPE_TOOLTIP == type)
     return _("Tooltip");
   if (E_WINDOW_TYPE_UTILITY == type)
     return _("Utility");
   if (E_WINDOW_TYPE_REAL_UNKNOWN == type)
     return _("Unknown");

   return _("Unused");
}

static char *
_match_label_get(Match_Config *m)
{
   char *label;
   Eina_Strbuf *buf = eina_strbuf_new();

   if (m->match.title)
     {
        eina_strbuf_append(buf, _("Title:"));
        eina_strbuf_append(buf, m->match.title);
        eina_strbuf_append(buf, _(" / "));
     }
   if (m->match.primary_type)
     {
        eina_strbuf_append(buf, _("Type:"));
        eina_strbuf_append(buf, _match_type_label_get(m->match.primary_type));
        eina_strbuf_append(buf, _(" / "));
     }
   if (m->match.name)
     {
        eina_strbuf_append(buf, _("Name:"));
        eina_strbuf_append(buf, m->match.name);
        eina_strbuf_append(buf, _(" / "));
     }
   if (m->match.clas)
     {
        eina_strbuf_append(buf, _("Class:"));
        eina_strbuf_append(buf, m->match.clas);
        eina_strbuf_append(buf, _(" / "));
     }
   if (m->match.role)
     {
        eina_strbuf_append(buf, _("Role:"));
        eina_strbuf_append(buf, m->match.role);
        eina_strbuf_append(buf, _(" / "));
     }
   if (m->match.shadow_style)
     {
        eina_strbuf_append(buf, _("Style:"));
        eina_strbuf_append(buf, m->match.shadow_style);
     }
   if (m->match.visibility_effect)
     {
        eina_strbuf_append(buf, _(" / "));
        eina_strbuf_append(buf, _("Effect:"));
        eina_strbuf_append(buf, m->match.visibility_effect);
     }

   if (!eina_strbuf_length_get(buf))
     return _("Unknown");

   label = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);

   return label;
}

static void
_match_ilist_append(Evas_Object *il, Match_Config *m, int pos, int pre)
{
   char *name = _match_label_get(m);

   if (pos == -1)
     e_widget_ilist_append(il, NULL, name, NULL, m, NULL);
   else
     {
        if (pre)
          e_widget_ilist_prepend_relative(il, NULL, name, NULL, m, NULL, pos);
        else
          e_widget_ilist_append_relative(il, NULL, name, NULL, m, NULL, pos);
     }
   free(name);
}

static void
_match_list_up(Eina_List **list, Match_Config *m)
{
   Eina_List *l, *lp;

   l = eina_list_data_find_list(*list, m);
   if (!l) return;
   lp = l->prev;
   *list = eina_list_remove_list(*list, l);
   if (lp) *list = eina_list_prepend_relative_list(*list, m, lp);
   else *list = eina_list_prepend(*list, m);
}

static void
_match_list_down(Eina_List **list, Match_Config *m)
{
   Eina_List *l, *lp;

   l = eina_list_data_find_list(*list, m);
   if (!l) return;
   lp = l->next;
   *list = eina_list_remove_list(*list, l);
   if (lp) *list = eina_list_append_relative_list(*list, m, lp);
   else *list = eina_list_append(*list, m);
}

static Eina_Bool
_match_list_del(Eina_List **list, Match_Config *m)
{
   Eina_List *l;

   l = eina_list_data_find_list(*list, m);
   if (!l) return EINA_FALSE;
   *list = eina_list_remove_list(*list, l);
   _match_free(m);
   return EINA_TRUE;
}


static void
_cb_dialog_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *bg, *of;
   int x, y, w, h;

   of = data;
   bg = evas_object_data_get(of, "bg");
   evas_object_geometry_get(obj, &x, &y, &w, &h);

   evas_object_move(bg, x, y);
   evas_object_resize(bg, w, h);
   evas_object_move(of, x, y);
   evas_object_resize(of, w, h);
}

static void
_edit_ok(void *d1, void *d2)
{
   Match_Config *m = d1;
   Evas_Object *dia, *bg, *of = d2;
   Evas_Object *il;

   if (m->title || m->name || m->clas || m->role || (m->primary_type != m->match.primary_type) ||
       (eina_list_nth(m->cfd->cfdata->comp_effects, m->effect_type) != m->match.visibility_effect))
     {
        m->cfd->cfdata->changed = 1;
        e_config_dialog_changed_set(m->cfd, 1);
        eina_stringshare_replace(&m->match.title, NULL);
        if (m->title)
          {
             if (m->title[0]) m->match.title = eina_stringshare_add(m->title);
             E_FREE(m->title);
          }
        eina_stringshare_replace(&m->match.name, NULL);
        if (m->name)
          {
             if (m->name[0]) m->match.name = eina_stringshare_add(m->name);
             E_FREE(m->name);
          }
        eina_stringshare_replace(&m->match.clas, NULL);
        if (m->clas)
          {
             if (m->clas[0]) m->match.clas = eina_stringshare_add(m->clas);
             E_FREE(m->clas);
          }
        eina_stringshare_replace(&m->match.role, NULL);
        if (m->role)
          {
             if (m->role[0]) m->match.role = eina_stringshare_add(m->role);
             E_FREE(m->role);
          }
        m->match.borderless = m->borderless;
        m->match.dialog = m->dialog;
        m->match.accepts_focus = m->accepts_focus;
        m->match.argb = m->argb;
        m->match.fullscreen = m->fullscreen;
        m->match.modal = m->modal;
        m->match.primary_type = m->primary_type;
        eina_stringshare_refplace(&m->match.visibility_effect, eina_list_nth(m->cfd->cfdata->comp_effects, m->effect_type));
        if (eina_streq(m->match.visibility_effect, "none"))
          eina_stringshare_replace(&m->match.visibility_effect, NULL);
        il = m->cfd->cfdata->edit_il;
        {
           const Eina_List *l;
           E_Ilist_Item *ili;
           Eina_Bool found = EINA_FALSE;

           EINA_LIST_FOREACH(e_widget_ilist_items_get(il), l, ili)
             {
                char *txt;

                if (e_widget_ilist_item_data_get(ili) != m) continue;
                txt = _match_label_get(m);
                e_ilist_item_label_set(ili, txt);
                free(txt);
                found = EINA_TRUE;
                break;
             }
           if (!found)
             {
                unsigned int n;

                if (il == m->cfd->cfdata->popups_il)
                  m->cfd->cfdata->popups = eina_list_append(m->cfd->cfdata->popups, m);
                else if (il == m->cfd->cfdata->borders_il)
                  m->cfd->cfdata->borders = eina_list_append(m->cfd->cfdata->borders, m);
                else if (il == m->cfd->cfdata->overrides_il)
                  m->cfd->cfdata->overrides = eina_list_append(m->cfd->cfdata->overrides, m);
                else if (il == m->cfd->cfdata->menus_il)
                  m->cfd->cfdata->menus = eina_list_append(m->cfd->cfdata->menus, m);
                else if (il == m->cfd->cfdata->objects_il)
                  m->cfd->cfdata->objects = eina_list_append(m->cfd->cfdata->objects, m);
                _match_ilist_append(il, m, -1, 0);
                n = e_widget_ilist_count(il);
                e_widget_ilist_nth_show(il, n - 1, 0);
                e_widget_ilist_selected_set(il, n - 1);
             }
        }
     }
   bg = evas_object_data_get(of, "bg");
   dia = evas_object_data_get(of, "dia");

   evas_object_event_callback_del(dia, EVAS_CALLBACK_RESIZE, _cb_dialog_resize);
   evas_object_del(bg);
   evas_object_del(of);
}

static void
_create_edit_frame(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata, Match_Config *m)
{
   Evas_Object *of, *oi, *lb, *en, *bt, *tb, *tab2, *o, *sf, *li;
   E_Radio_Group *rg;
   Eina_List *l;
   Eina_Stringshare *s;
   int row, mode = 0;
   int x, y, w, h, mw, mh;

   o = edje_object_add(evas);
   e_theme_edje_object_set(o, "base/theme/dialog", "e/widgets/dialog/main");
   evas_object_geometry_get(cfd->dia->bg_object, &x, &y, &w, &h);
   evas_object_move(o, x, y);
   evas_object_resize(o, w, h);
   evas_object_show(o);

   of = e_widget_frametable_add(evas, _("Edit E_Comp_Match"), 0);
   evas_object_data_set(of, "bg", o);
   evas_object_data_set(of, "dia", cfd->dia->bg_object);
   evas_object_move(of, x, y);
   evas_object_resize(of, w, h);
   evas_object_show(of);

   evas_object_event_callback_add(cfd->dia->bg_object, EVAS_CALLBACK_RESIZE,
                                  _cb_dialog_resize, of);

   tb = e_widget_toolbook_add(evas, 48 * e_scale, 48 * e_scale);

   tab2 = e_widget_table_add(e_win_evas_win_get(evas), 0);
   if (cfdata->edit_il == cfdata->borders_il)
     {
        if (m->match.title) m->title = strdup(m->match.title);
        else m->title = NULL;
        lb = e_widget_label_add(evas, _("Title"));
        e_widget_table_object_append(tab2, lb, 0, 0, 1, 1, 1, 0, 0, 0);
        en = e_widget_entry_add(cfd->dia->win, &(m->title), NULL, NULL, NULL);
        e_widget_table_object_append(tab2, en, 1, 0, 1, 1, 1, 0, 1, 0);
     }
   if ((cfdata->edit_il == cfdata->borders_il) ||
       (cfdata->edit_il == cfdata->overrides_il) ||
       (cfdata->edit_il == cfdata->popups_il))
     {
        if (m->match.name) m->name = strdup(m->match.name);
        else m->name = NULL;
        lb = e_widget_label_add(evas, _("Name"));
        e_widget_table_object_append(tab2, lb, 0, 1, 1, 1, 1, 0, 0, 0);
        en = e_widget_entry_add(cfd->dia->win, &(m->name), NULL, NULL, NULL);
        e_widget_table_object_append(tab2, en, 1, 1, 1, 1, 1, 0, 1, 0);
     }
   if ((cfdata->edit_il == cfdata->borders_il) ||
       (cfdata->edit_il == cfdata->overrides_il))
     {
        if (m->match.clas) m->clas = strdup(m->match.clas);
        else m->clas = NULL;
        lb = e_widget_label_add(evas, _("Class"));
        e_widget_table_object_append(tab2, lb, 0, 2, 1, 1, 1, 0, 0, 0);
        en = e_widget_entry_add(cfd->dia->win, &(m->clas), NULL, NULL, NULL);
        e_widget_table_object_append(tab2, en, 1, 2, 1, 1, 1, 0, 1, 0);
     }
   if (cfdata->edit_il == cfdata->borders_il)
     {
        if (m->match.role) m->role = strdup(m->match.role);
        else m->role = NULL;
        lb = e_widget_label_add(evas, _("Role"));
        e_widget_table_object_append(tab2, lb, 0, 3, 1, 1, 1, 0, 0, 0);
        en = e_widget_entry_add(cfd->dia->win, &(m->role), NULL, NULL, NULL);
        e_widget_table_object_append(tab2, en, 1, 3, 1, 1, 1, 0, 1, 0);
     }
   e_widget_toolbook_page_append(tb, NULL, _("Names"), tab2, 1, 1, 1, 1, 0.5, 0.0);

   if ((cfdata->edit_il == cfdata->borders_il) ||
       (cfdata->edit_il == cfdata->overrides_il))
     {
        rg = e_widget_radio_group_new(&m->primary_type);

        li = e_widget_list_add(evas, 1, 0);

        o = e_widget_radio_add(evas, _("Unused"), E_WINDOW_TYPE_UNKNOWN, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);

        o = e_widget_radio_add(evas, _("Combo"), E_WINDOW_TYPE_COMBO, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Desktop"), E_WINDOW_TYPE_DESKTOP, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Dialog"), E_WINDOW_TYPE_DIALOG, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Dock"), E_WINDOW_TYPE_DOCK, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Drag and Drop"), E_WINDOW_TYPE_DND, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Menu"), E_WINDOW_TYPE_MENU, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Menu (Dropdown)"), E_WINDOW_TYPE_DROPDOWN_MENU, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Menu (Popup)"), E_WINDOW_TYPE_POPUP_MENU, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Normal"), E_WINDOW_TYPE_NORMAL, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Notification"), E_WINDOW_TYPE_NOTIFICATION, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Splash"), E_WINDOW_TYPE_SPLASH, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Toolbar"), E_WINDOW_TYPE_TOOLBAR, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Tooltip"), E_WINDOW_TYPE_TOOLTIP, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Utility"), E_WINDOW_TYPE_UTILITY, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);
        o = e_widget_radio_add(evas, _("Unknown"), E_WINDOW_TYPE_REAL_UNKNOWN, rg);
        e_widget_list_object_append(li, o, 1, 0, 0.0);

        e_widget_size_min_get(li, &mw, &mh);
        evas_object_resize(li, mw, mh);

        sf = e_widget_scrollframe_simple_add(evas, li);
        e_widget_toolbook_page_append(tb, NULL, _("Types"), sf,
                                      1, 1, 1, 1, 0.5, 0.0);
     }

   m->borderless = m->match.borderless;
   m->dialog = m->match.dialog;
   m->accepts_focus = m->match.accepts_focus;
   m->argb = m->match.argb;
   m->fullscreen = m->match.fullscreen;
   m->modal = m->match.modal;

   row = 0;
   tab2 = e_widget_table_add(e_win_evas_win_get(evas), 0);
   lb = e_widget_label_add(evas, _("Unused"));
   e_widget_table_object_append(tab2, lb, 1, row, 1, 1, 0, 0, 0, 0);
   lb = e_widget_label_add(evas, _("On"));
   e_widget_table_object_append(tab2, lb, 2, row, 1, 1, 0, 0, 0, 0);
   lb = e_widget_label_add(evas, _("Off"));
   e_widget_table_object_append(tab2, lb, 3, row, 1, 1, 0, 0, 0, 0);
   row++;

   if (cfdata->edit_il == cfdata->borders_il)
     {
        lb = e_widget_label_add(evas, _("Borderless"));
        e_widget_table_object_append(tab2, lb, 0, row, 1, 1, 1, 0, 1, 0);
        rg = e_widget_radio_group_new(&m->borderless);
        o = e_widget_radio_add(evas, NULL, 0, rg);
        e_widget_table_object_append(tab2, o, 1, row, 1, 1, 0, 0, 0, 0);
        o = e_widget_radio_add(evas, NULL, 1, rg);
        e_widget_table_object_append(tab2, o, 2, row, 1, 1, 0, 0, 0, 0);
        o = e_widget_radio_add(evas, NULL, -1, rg);
        e_widget_table_object_append(tab2, o, 3, row, 1, 1, 0, 0, 0, 0);
        row++;
     }
   if (cfdata->edit_il == cfdata->borders_il)
     {
        lb = e_widget_label_add(evas, _("Dialog"));
        e_widget_table_object_append(tab2, lb, 0, row, 1, 1, 1, 0, 1, 0);
        rg = e_widget_radio_group_new(&m->dialog);
        o = e_widget_radio_add(evas, NULL, 0, rg);
        e_widget_table_object_append(tab2, o, 1, row, 1, 1, 0, 0, 0, 0);
        o = e_widget_radio_add(evas, NULL, 1, rg);
        e_widget_table_object_append(tab2, o, 2, row, 1, 1, 0, 0, 0, 0);
        o = e_widget_radio_add(evas, NULL, -1, rg);
        e_widget_table_object_append(tab2, o, 3, row, 1, 1, 0, 0, 0, 0);
        row++;
     }
   if (cfdata->edit_il == cfdata->borders_il)
     {
        lb = e_widget_label_add(evas, _("Accepts Focus"));
        e_widget_table_object_append(tab2, lb, 0, row, 1, 1, 1, 0, 1, 0);
        rg = e_widget_radio_group_new(&m->accepts_focus);
        o = e_widget_radio_add(evas, NULL, 0, rg);
        e_widget_table_object_append(tab2, o, 1, row, 1, 1, 0, 0, 0, 0);
        o = e_widget_radio_add(evas, NULL, 1, rg);
        e_widget_table_object_append(tab2, o, 2, row, 1, 1, 0, 0, 0, 0);
        o = e_widget_radio_add(evas, NULL, -1, rg);
        e_widget_table_object_append(tab2, o, 3, row, 1, 1, 0, 0, 0, 0);
        row++;
     }
   lb = e_widget_label_add(evas, _("ARGB"));
   e_widget_table_object_append(tab2, lb, 0, row, 1, 1, 1, 0, 1, 0);
   rg = e_widget_radio_group_new(&m->argb);
   o = e_widget_radio_add(evas, NULL, 0, rg);
   e_widget_table_object_append(tab2, o, 1, row, 1, 1, 0, 0, 0, 0);
   o = e_widget_radio_add(evas, NULL, 1, rg);
   e_widget_table_object_append(tab2, o, 2, row, 1, 1, 0, 0, 0, 0);
   o = e_widget_radio_add(evas, NULL, -1, rg);
   e_widget_table_object_append(tab2, o, 3, row, 1, 1, 0, 0, 0, 0);
   row++;
   if (cfdata->edit_il == cfdata->borders_il)
     {
        lb = e_widget_label_add(evas, _("Fullscreen"));
        e_widget_table_object_append(tab2, lb, 0, row, 1, 1, 1, 0, 1, 0);
        rg = e_widget_radio_group_new(&m->fullscreen);
        o = e_widget_radio_add(evas, NULL, 0, rg);
        e_widget_table_object_append(tab2, o, 1, row, 1, 1, 0, 0, 0, 0);
        o = e_widget_radio_add(evas, NULL, 1, rg);
        e_widget_table_object_append(tab2, o, 2, row, 1, 1, 0, 0, 0, 0);
        o = e_widget_radio_add(evas, NULL, -1, rg);
        e_widget_table_object_append(tab2, o, 3, row, 1, 1, 0, 0, 0, 0);
        row++;
     }
   if (cfdata->edit_il == cfdata->borders_il)
     {
        lb = e_widget_label_add(evas, _("Modal"));
        e_widget_table_object_append(tab2, lb, 0, row, 1, 1, 1, 0, 1, 0);
        rg = e_widget_radio_group_new(&m->modal);
        o = e_widget_radio_add(evas, NULL, 0, rg);
        e_widget_table_object_append(tab2, o, 1, row, 1, 1, 0, 0, 0, 0);
        o = e_widget_radio_add(evas, NULL, 1, rg);
        e_widget_table_object_append(tab2, o, 2, row, 1, 1, 0, 0, 0, 0);
        o = e_widget_radio_add(evas, NULL, -1, rg);
        e_widget_table_object_append(tab2, o, 3, row, 1, 1, 0, 0, 0, 0);
        row++;
     }
   e_widget_toolbook_page_append(tb, NULL, _("Flags"), tab2,
                                 1, 1, 1, 1, 0.5, 0.0);

   oi = e_comp_style_selector_create(evas, &(m->match.shadow_style));
   e_widget_toolbook_page_append(tb, NULL, _("Style"), oi,
                                 1, 1, 1, 1, 0.5, 0.0);

   oi = e_widget_list_add(evas, 1, 0);
   EINA_LIST_FOREACH(cfdata->comp_effects->next, l, s)
     {
        m->effect_type++;
        if (s == m->match.visibility_effect)
          break;
     }
   if (!s) m->effect_type = 0;
   rg = e_widget_radio_group_new(&(m->effect_type));
   EINA_LIST_FOREACH(cfdata->comp_effects, l, s)
     {
        char *p;
        const char *pp;
        char buf[PATH_MAX];

        pp = strchr(s, '/');
        pp = pp ? pp + 1 : s;
        p = memcpy(buf, pp, strlen(pp) + 1);
        p[0] = toupper(p[0]);
        o = e_widget_radio_add(evas, _(p), mode, rg);
        e_widget_list_object_append(oi, o, 1, 0, 0.5);
        mode++;
     }
   e_widget_toolbook_page_append(tb, NULL, _("Effect"), oi,
                                 1, 1, 1, 1, 0.5, 0.0);

   e_widget_frametable_object_append(of, tb, 0, 0, 1, 1, 1, 1, 1, 1);
   e_widget_toolbook_page_show(tb, 0);

   bt = e_widget_button_add(evas, _("OK"), NULL, _edit_ok, m, of);
   e_widget_frametable_object_append(of, bt, 0, 1, 1, 1, 0, 0, 0, 0);

   e_widget_size_min_get(of, &mw, &mh);
   {
      int ww, wh;

      evas_object_geometry_get(cfd->dia->win, NULL, NULL, &ww, &wh);
      evas_object_resize(cfd->dia->win, MAX(mw, ww), MAX(mh, wh));
   }
}

static void
_but_up(void *d1, void *d2)
{
   E_Config_Dialog *cfd = d1;
   Evas_Object *il = d2;
   Match_Config *m;
   int n;

   e_widget_ilist_freeze(il);
   n = e_widget_ilist_selected_get(il);
   if (n < 1) return;
   m = e_widget_ilist_nth_data_get(il, n);
   if (!m)
     {
        e_widget_ilist_thaw(il);
        return;
     }
   e_widget_ilist_remove_num(il, n);
   n--;
   _match_ilist_append(il, m, n, 1);
   e_widget_ilist_nth_show(il, n, 0);
   e_widget_ilist_selected_set(il, n);
   e_widget_ilist_thaw(il);
   e_widget_ilist_go(il);
   _match_list_up(&(cfd->cfdata->popups), m);
   _match_list_up(&(cfd->cfdata->borders), m);
   _match_list_up(&(cfd->cfdata->overrides), m);
   _match_list_up(&(cfd->cfdata->menus), m);
   _match_list_up(&(cfd->cfdata->objects), m);
   cfd->cfdata->changed = 1;
   e_config_dialog_changed_set(cfd, 1);
}

static void
_but_down(void *d1, void *d2)
{
   E_Config_Dialog *cfd = d1;
   Evas_Object *il = d2;
   Match_Config *m;
   int n;

   e_widget_ilist_freeze(il);
   n = e_widget_ilist_selected_get(il);
   if (n >= (e_widget_ilist_count(il) - 1)) return;
   m = e_widget_ilist_nth_data_get(il, n);
   if (!m)
     {
        e_widget_ilist_thaw(il);
        return;
     }
   e_widget_ilist_remove_num(il, n);
   _match_ilist_append(il, m, n, 0);
   e_widget_ilist_nth_show(il, n + 1, 0);
   e_widget_ilist_selected_set(il, n + 1);
   e_widget_ilist_thaw(il);
   e_widget_ilist_go(il);
   _match_list_down(&(cfd->cfdata->popups), m);
   _match_list_down(&(cfd->cfdata->borders), m);
   _match_list_down(&(cfd->cfdata->overrides), m);
   _match_list_down(&(cfd->cfdata->menus), m);
   _match_list_down(&(cfd->cfdata->objects), m);
   cfd->cfdata->changed = 1;
   e_config_dialog_changed_set(cfd, 1);
}

static void
_but_add(void *d1, void *d2)
{
   E_Config_Dialog *cfd = d1;
   Evas_Object *il = d2;
   Match_Config *m;

   m = E_NEW(Match_Config, 1);
   m->cfd = cfd;
   m->match.shadow_style = eina_stringshare_add("default");

   cfd->cfdata->edit_il = il;
   _create_edit_frame(cfd, evas_object_evas_get(il), cfd->cfdata, m);
}

static void
_but_del(void *d1, void *d2)
{
   E_Config_Dialog *cfd = d1;
   Evas_Object *il = d2;
   Match_Config *m;
   int n;

   e_widget_ilist_freeze(il);
   n = e_widget_ilist_selected_get(il);
   m = e_widget_ilist_nth_data_get(il, n);
   if (!m)
     {
        e_widget_ilist_thaw(il);
        return;
     }
   e_widget_ilist_remove_num(il, n);
   e_widget_ilist_thaw(il);
   e_widget_ilist_go(il);
   if (!_match_list_del(&(cfd->cfdata->popups), m))
     if (!_match_list_del(&(cfd->cfdata->borders), m))
       if (!_match_list_del(&(cfd->cfdata->overrides), m))
         if (!_match_list_del(&(cfd->cfdata->menus), m))
           _match_list_del(&(cfd->cfdata->objects), m);
   cfd->cfdata->changed = 1;
   e_config_dialog_changed_set(cfd, 1);
}

static void
_but_edit(void *d1, void *d2)
{
   E_Config_Dialog *cfd = d1;
   Evas_Object *il = d2;
   int n;
   Match_Config *m;

   n = e_widget_ilist_selected_get(il);
   m = e_widget_ilist_nth_data_get(il, n);
   if (!m) return;

   cfd->cfdata->edit_il = il;
   _create_edit_frame(cfd, evas_object_evas_get(il), cfd->cfdata, m);
   cfd->cfdata->changed = 1;
   e_config_dialog_changed_set(cfd, 1);
}

static Evas_Object *
_create_match_editor(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata EINA_UNUSED, Eina_List **matches, Evas_Object **il_ret)
{
   Evas_Object *tab, *il, *bt;
   Match_Config *m;
   Eina_List *l;

   tab = e_widget_table_add(e_win_evas_win_get(evas), 0);

   il = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_size_min_set(il, 160, 100);

   EINA_LIST_FOREACH(*matches, l, m)
     {
        _match_ilist_append(il, m, -1, 0);
     }

   e_widget_ilist_go(il);
   e_widget_table_object_append(tab, il, 0, 0, 1, 5, 1, 1, 1, 1);

   bt = e_widget_button_add(evas, _("Up"), NULL, _but_up, cfd, il);
   e_widget_table_object_append(tab, bt, 1, 0, 1, 1, 1, 1, 0, 0);
   bt = e_widget_button_add(evas, _("Down"), NULL, _but_down, cfd, il);
   e_widget_table_object_append(tab, bt, 1, 1, 1, 1, 1, 1, 0, 0);
   bt = e_widget_button_add(evas, _("Add"), NULL, _but_add, cfd, il);
   e_widget_table_object_append(tab, bt, 1, 2, 1, 1, 1, 1, 0, 0);
   bt = e_widget_button_add(evas, _("Del"), NULL, _but_del, cfd, il);
   e_widget_table_object_append(tab, bt, 1, 3, 1, 1, 1, 1, 0, 0);
   bt = e_widget_button_add(evas, _("Edit"), NULL, _but_edit, cfd, il);
   e_widget_table_object_append(tab, bt, 1, 4, 1, 1, 1, 1, 0, 0);

   *il_ret = il;

   return tab;
}

static Evas_Object *
_create_styles_toolbook(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *tb, *oi, *il;

   tb = e_widget_toolbook_add(evas, 48 * e_scale, 48 * e_scale);

   oi = _create_match_editor(cfd, evas, cfdata, &(cfdata->borders), &il);
   cfdata->borders_il = il;
   e_widget_toolbook_page_append(tb, NULL, _("Apps"), oi, 1, 1, 1, 1, 0.5, 0.0);

   oi = _create_match_editor(cfd, evas, cfdata, &(cfdata->popups), &il);
   cfdata->popups_il = il;
   e_widget_toolbook_page_append(tb, NULL, _("Popups"), oi, 1, 1, 1, 1, 0.5, 0.0);

   oi = _create_match_editor(cfd, evas, cfdata, &(cfdata->overrides), &il);
   cfdata->overrides_il = il;
   e_widget_toolbook_page_append(tb, NULL, _("Overrides"), oi, 1, 1, 1, 1, 0.5, 0.0);

   oi = _create_match_editor(cfd, evas, cfdata, &(cfdata->menus), &il);
   cfdata->menus_il = il;
   e_widget_toolbook_page_append(tb, NULL, _("Menus"), oi, 1, 1, 1, 1, 0.5, 0.0);

   oi = _create_match_editor(cfd, evas, cfdata, &(cfdata->objects), &il);
   cfdata->objects_il = il;
   e_widget_toolbook_page_append(tb, NULL, _("Objects"), oi, 1, 1, 1, 1, 0.5, 0.0);

   e_widget_toolbook_page_show(tb, 0);

   return tb;
}

static int
_basic_apply_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   Eina_List *l;
   E_Comp_Match *m;
   Match_Config *m2;
   E_Comp_Config *conf = e_comp_config_get();

   if (!cfdata->changed) return 0;

   E_FREE_LIST(conf->match.popups, e_comp_cfdata_match_free);
   E_FREE_LIST(conf->match.borders, e_comp_cfdata_match_free);
   E_FREE_LIST(conf->match.overrides, e_comp_cfdata_match_free);
   E_FREE_LIST(conf->match.menus, e_comp_cfdata_match_free);
   E_FREE_LIST(conf->match.objects, e_comp_cfdata_match_free);

   EINA_LIST_FOREACH(cfdata->popups, l, m2)
     {
        m = E_NEW(E_Comp_Match, 1);
        _match_dup2(m2, m);
        conf->match.popups = eina_list_append(conf->match.popups, m);
     }
   EINA_LIST_FOREACH(cfdata->borders, l, m2)
     {
        m = E_NEW(E_Comp_Match, 1);
        _match_dup2(m2, m);
        conf->match.borders = eina_list_append(conf->match.borders, m);
     }
   EINA_LIST_FOREACH(cfdata->overrides, l, m2)
     {
        m = E_NEW(E_Comp_Match, 1);
        _match_dup2(m2, m);
        conf->match.overrides = eina_list_append(conf->match.overrides, m);
     }
   EINA_LIST_FOREACH(cfdata->menus, l, m2)
     {
        m = E_NEW(E_Comp_Match, 1);
        _match_dup2(m2, m);
        conf->match.menus = eina_list_append(conf->match.menus, m);
     }
   EINA_LIST_FOREACH(cfdata->objects, l, m2)
     {
        m = E_NEW(E_Comp_Match, 1);
        _match_dup2(m2, m);
        conf->match.objects = eina_list_append(conf->match.objects, m);
     }
   cfdata->changed = 0;
   e_comp_shadows_reset();
   e_config_save_queue();
   return 1;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *orec0;

   orec0 = evas_object_rectangle_add(evas);
   evas_object_name_set(orec0, "style_shadows");
   elm_win_center(cfd->dia->win, 1, 1);

   return _create_styles_toolbook(cfd, evas, cfdata);
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   E_FREE_LIST(cfdata->popups, _match_free);
   E_FREE_LIST(cfdata->borders, _match_free);
   E_FREE_LIST(cfdata->menus, _match_free);
   E_FREE_LIST(cfdata->objects, _match_free);
   E_FREE_LIST(cfdata->overrides, _match_free);
   free(cfdata);
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *l;
   Eina_Stringshare *grp;
   E_Comp_Match *m;
   Match_Config *m2;
   E_Comp_Config *conf = e_comp_config_get();

   cfdata = E_NEW(E_Config_Dialog_Data, 1);

   EINA_LIST_FOREACH(conf->match.popups, l, m)
     {
        m2 = E_NEW(Match_Config, 1);
        _match_dup(m, m2);
        m2->cfd = cfd;
        cfdata->popups = eina_list_append(cfdata->popups, m2);
     }

   EINA_LIST_FOREACH(conf->match.borders, l, m)
     {
        m2 = E_NEW(Match_Config, 1);
        _match_dup(m, m2);
        m2->cfd = cfd;
        cfdata->borders = eina_list_append(cfdata->borders, m2);
     }

   EINA_LIST_FOREACH(conf->match.overrides, l, m)
     {
        m2 = E_NEW(Match_Config, 1);
        _match_dup(m, m2);
        m2->cfd = cfd;
        cfdata->overrides = eina_list_append(cfdata->overrides, m2);
     }

   EINA_LIST_FOREACH(conf->match.menus, l, m)
     {
        m2 = E_NEW(Match_Config, 1);
        _match_dup(m, m2);
        m2->cfd = cfd;
        cfdata->menus = eina_list_append(cfdata->menus, m2);
     }

   EINA_LIST_FOREACH(conf->match.objects, l, m)
     {
        m2 = E_NEW(Match_Config, 1);
        _match_dup(m, m2);
        m2->cfd = cfd;
        cfdata->objects = eina_list_append(cfdata->objects, m2);
     }
   cfdata->comp_effects = e_theme_collection_items_find("base/theme/borders", "e/comp/effects/visibility");
   /* comp effects must be prefixed with "visibility" or they won't work and things will break! */
   EINA_LIST_FOREACH(cfdata->comp_effects, l, grp)
     {
        Eina_Stringshare *g;
        char buf[1024];

        snprintf(buf, sizeof(buf), "visibility/%s", grp);
        g = eina_stringshare_add(buf);
        eina_stringshare_del(grp);
        eina_list_data_set(l, g);
     }
   cfdata->comp_effects = eina_list_prepend(cfdata->comp_effects, eina_stringshare_add("none"));
   return cfdata;
}

E_API E_Config_Dialog *
e_int_config_comp_match(Evas_Object *parent, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "internal/comp_matches")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;

   cfd = e_config_dialog_new(parent, _("Composite Match Settings"),
                             "E", "_comp_matches", "preferences-composite", 0, v, NULL);
   e_dialog_resizable_set(cfd->dia, 1);
   return cfd;
}
