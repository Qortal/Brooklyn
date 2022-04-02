#ifndef E_MOD_TILING_H
# define E_MOD_TILING_H

# include <e.h>

# include <stdbool.h>

# include <assert.h>

# include "window_tree.h"

typedef struct _Config      Config;
typedef struct _Tiling_Info Tiling_Info;

struct tiling_g
{
   E_Module  *module;
   Config    *config;
   int        log_domain;

   Eina_List *gadget_instances;
   int        gadget_number;
};
extern struct tiling_g tiling_g;

# undef ERR
# undef DBG
# define ERR(...) EINA_LOG_DOM_ERR(tiling_g.log_domain, __VA_ARGS__)
# define DBG(...) EINA_LOG_DOM_DBG(tiling_g.log_domain, __VA_ARGS__)

# define TILING_MAX_PADDING 50

struct _Config_vdesk
{
   int          x, y;
   unsigned int zone_num;
   int          nb_stacks;
};

struct _Config
{
   int        window_padding;
   int        tile_dialogs;
   int        show_titles;
   int        have_floating_mode;
   Eina_List *vdesks;
};

struct _Tiling_Info
{
   /* The desk for which this _Tiling_Info is used. Needed because (for
    * example) on e restart all desks are shown on all zones but no change
    * events are triggered */
   const E_Desk         *desk;

   struct _Config_vdesk *conf;

   Window_Tree          *tree;
};

struct _E_Config_Dialog_Data
{
   struct _Config config;
   Evas_Object   *o_zonelist;
   Evas_Object   *o_desklist;
   Evas_Object   *osf;
   Evas          *evas;
};

E_Config_Dialog      *e_int_config_tiling_module(Evas_Object *parent, const char *params);

E_API extern E_Module_Api e_modapi;

E_API void            *e_modapi_init(E_Module *m);
E_API int              e_modapi_shutdown(E_Module *m);
E_API int              e_modapi_save(E_Module *m);

void                  change_desk_conf(struct _Config_vdesk *newconf);

void                  e_tiling_update_conf(void);

struct _Config_vdesk *get_vdesk(Eina_List *vdesks, int x, int y,
                                unsigned int zone_num);

void                  tiling_e_client_move_resize_extra(E_Client *ec, int x, int y, int w,
                                                        int h);
void                  tiling_e_client_does_not_fit(E_Client *ec);
# define EINA_LIST_IS_IN(_list, _el) \
  (eina_list_data_find(_list, _el) == _el)
# define EINA_LIST_APPEND(_list, _el) \
  _list = eina_list_append(_list, _el)
# define EINA_LIST_REMOVE(_list, _el) \
  _list = eina_list_remove(_list, _el)

# define _TILE_MIN(a, b) (((a) < (b)) ? (a) : (b))
# define _TILE_MAX(a, b) (((a) > (b)) ? (a) : (b))

#endif
