#include "e_mod_main.h"

//TODO min input for items not in history

#define MAX_ITEMS  100
#define MAX_PLUGIN 25

typedef struct _Plugin Plugin;

struct _Plugin
{
   Evry_Plugin base;

   Evry_Item  *warning;
};

static int
_fetch(Evry_Plugin *plugin, const char *input)
{
   Plugin *p = (Plugin *)plugin;
   Evry_Plugin *pp;
   Eina_List *l, *ll, *lp = NULL;
   Evry_Item *it, *it2 = NULL;
   int top_level = 0, subj_sel = 0, cnt = 0;
   size_t inp_len = 0;
   Eina_List *items = NULL;
   const char *context = NULL;
   char buf[128];
   Evry_State *s = plugin->state;
   Evry_Selector *sel = s->selector;
   Evry_Selector **sels = sel->win->selectors;

   if (input && input[0])
     inp_len = strlen(input);
   else
     input = NULL;

   if ((eina_list_count(sel->states) == 1))
     top_level = 1;

   /* get current items' context, which is
    * the previous selectors selected item */
   if (sel == sels[0])
     {
        subj_sel = 1;
     }
   else if (sel == sels[1])
     {
        it = sels[0]->state->cur_item;
        if (it) context = it->context;
     }
   else if (sel == sels[2])
     {
        it = sels[1]->state->cur_item;
        if (it) context = it->context;
     }

   EVRY_PLUGIN_ITEMS_FREE(p);

   /* collect plugins to be shown in aggregator */
   EINA_LIST_FOREACH (s->cur_plugins, l, pp)
     {
        /* dont show in aggregator */
        if (!pp->config->aggregate)
          continue;
        /* do not recurse */
        if (pp == plugin)
          continue;
        /* dont show plugin in top-level */
        if (top_level && !pp->config->top_level)
          continue;
        lp = eina_list_append(lp, pp);
     }

   /* show non-top-level plugins as item */
   if ((top_level) && (!s->trigger_active))
     {
        EINA_LIST_FOREACH (s->plugins, l, pp)
          {
             int min_fuzz = 0, n;
             double max_usage = 0.0;

             if (pp->config->top_level)
               continue;

             if (pp == plugin)
               continue;

             if (!pp->items)
               continue;

             /* give plugin item the highest priority of its items */
             EINA_LIST_FOREACH (pp->items, ll, it)
               {
                  if (it->usage > -1.0)
                    evry_history_item_usage_set(it, input, context);

                  if (it->usage > max_usage)
                    max_usage = it->usage;

                  if (it->fuzzy_match == 0)
                    it->fuzzy_match = evry_fuzzy_match(it->label, input);

                  if ((!min_fuzz) || ((it->fuzzy_match > 0) &&
                                      (it->fuzzy_match < min_fuzz)))
                    min_fuzz = it->fuzzy_match;
               }

             GET_ITEM(itp, pp);

             itp->hi = NULL;
             /* TODO get better usage estimate */
             evry_history_item_usage_set(itp, NULL, NULL);
             itp->usage /= 100.0;

             if ((!eina_dbl_exact(itp->usage, 0)) && (!eina_dbl_exact(max_usage, 0)) && (itp->usage < max_usage))
               itp->usage = max_usage;
             itp->fuzzy_match = min_fuzz;

             IF_RELEASE(itp->detail);
             n = eina_list_count(pp->items);
             snprintf(buf, sizeof(buf), P_("%d item", "%d items", n), n);
             itp->detail = eina_stringshare_add(buf);

             items = eina_list_append(items, itp);
          }

        /* only one plugin: show items */
        if ((eina_list_count(s->cur_plugins)) == 1 && items &&
            (pp = eina_list_data_get(items)) && (pp->config->aggregate))
          {
             eina_list_free(items);
             items = NULL;
             EINA_LIST_FOREACH (pp->items, l, it)
               {
                  if (it->usage >= 0)
                    evry_history_item_usage_set(it, input, context);
                  if (it->fuzzy_match == 0)
                    it->fuzzy_match = evry_fuzzy_match(it->label, input);

                  items = eina_list_append(items, it);
               }
          }

#if 0
        /* append all plugins as items (which were not added above) */
        if (inp_len >= plugin->config->min_query)
          {
             EINA_LIST_FOREACH (s->plugins, l, pp)
               {
                  if (!strcmp(pp->name, "Actions"))
                    continue;

                  /* items MUST only contain plugins here ! */
                  EINA_LIST_FOREACH (items, ll, pp2)
                    if (pp2->name == pp->name) break;
                  if (pp2)
                    continue;

                  GET_ITEM(it, pp);

                  if ((!input) ||
                      (it->fuzzy_match = evry_fuzzy_match(it->label, input)))
                    {
                       evry_history_item_usage_set(it, input, NULL);
                       it->usage /= 100.0;

                       EVRY_ITEM(pp)->plugin->state = s;
                       items = eina_list_append(items, pp);
                    }
               }
          }
#endif
        if (!lp && (eina_list_count(items) < 2))
          {
             if (items) eina_list_free(items);
             return 0;
          }
     }

   if (!lp && !items)
     return 0;

   /* if there is input append all items that match */
   if (input)
     {
        EINA_LIST_FOREACH (lp, l, pp)
          {
             EINA_LIST_FOREACH (pp->items, ll, it)
               {
                  if (it->fuzzy_match == 0)
                    it->fuzzy_match = evry_fuzzy_match(it->label, input);

                  if (it->usage >= 0)
                    evry_history_item_usage_set(it, input, context);

                  if ((subj_sel) && (top_level) &&
                      eina_dbl_exact(it->usage, 0) && ((int) inp_len < plugin->config->min_query))
                    continue;

                  items = eina_list_append(items, it);
               }
          }
     }
   /* no input: append all items that are in history */
   else
     {
        EINA_LIST_FOREACH (lp, l, pp)
          {
             cnt = 1;
             EINA_LIST_FOREACH (pp->items, ll, it)
               {
                  if ((!subj_sel) || (it->usage < 0) ||
                      (evry_history_item_usage_set(it, input, context)))
                    {
                       it->fuzzy_match = 0;
                       items = eina_list_append(items, it);
                       if (++cnt > MAX_PLUGIN) break;
                    }
               }
          }
     }

   if ((!top_level) && (eina_list_count(items) < MAX_ITEMS))
     {
        EINA_LIST_FOREACH (lp, l, pp)
          {
             EINA_LIST_FOREACH (pp->items, ll, it)
               {
                  if (eina_list_data_find_list(items, it))
                    continue;

                  items = eina_list_append(items, it);
               }
          }
     }

   evry_util_items_sort(&items, 0 /* !input */);

   EINA_LIST_FOREACH (items, l, it)
     {
        /* remove duplicates provided by different plugins. e.g.
           files / places and tracker can find the same files */
        if (it->id)
          {
             EINA_LIST_FOREACH (p->base.items, ll, it2)
               {
                  if ((it->plugin->name != it2->plugin->name) &&
                      (it->type == it2->type) &&
                      (it->id == it2->id))
                    break;
               }
          }

        if (!it->id || !it2)
          {
             evry_item_ref(it);
             EVRY_PLUGIN_ITEM_APPEND(p, it);
          }

        if (cnt++ > MAX_ITEMS)
          break;
     }

   if (lp) eina_list_free(lp);

   if (items)
     {
        eina_list_free(items);
        return EVRY_PLUGIN_HAS_ITEMS(p);
     }
   /* 'text' and 'actions' are always loaded */
   else if ((subj_sel) && (eina_list_count(s->plugins) == 2))
     {
        evry_item_ref(p->warning);
        EVRY_PLUGIN_ITEM_APPEND(p, p->warning);
     }

   return 0;
}

static void
_finish(Evry_Plugin *plugin)
{
   GET_PLUGIN(p, plugin);

   EVRY_PLUGIN_ITEMS_FREE(p);

   evry_item_free(p->warning);

   E_FREE(p);
}

static Evry_Plugin *
_begin(Evry_Plugin *plugin, const Evry_Item *it EINA_UNUSED)
{
   Plugin *p;

   /* GET_PLUGIN(base, plugin); */
   EVRY_PLUGIN_INSTANCE(p, plugin);

   p->warning = evry_item_new(NULL, EVRY_PLUGIN(p), N_("No plugins loaded"), NULL, NULL);
   p->warning->type = EVRY_TYPE_NONE;

   return EVRY_PLUGIN(p);
}

Evry_Plugin *
evry_aggregator_new(int type)
{
   Evry_Plugin *p;

   p = EVRY_PLUGIN_BASE(N_("All"), NULL, 0, _begin, _finish, _fetch);

   if (evry_plugin_register(p, type, -1))
     {
        p->config->view_mode = VIEW_MODE_THUMB;
     }

   return p;
}

