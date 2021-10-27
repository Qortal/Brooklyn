#ifndef E_WIZARD_H

#include "e.h"

extern E_Module *wiz_module;

typedef struct _E_Wizard_Page E_Wizard_Page;
typedef struct _E_Wizard_Api E_Wizard_Api;

typedef enum
{
   E_WIZARD_PAGE_STATE_INIT,
   E_WIZARD_PAGE_STATE_SHOW,
   E_WIZARD_PAGE_STATE_HIDE,
   E_WIZARD_PAGE_STATE_SHUTDOWN
} E_Wizard_Page_State;

struct _E_Wizard_Page
{
   EINA_INLIST;
   void *handle;
   Evas *evas;
   Eina_Stringshare *name;
   int (*init)     (E_Wizard_Page *pg, Eina_Bool *need_xdg_desktops, Eina_Bool *need_xdg_icons);
   int (*shutdown) (E_Wizard_Page *pg);
   int (*show)     (E_Wizard_Page *pg);
   int (*hide)     (E_Wizard_Page *pg);
   int (*apply)    (E_Wizard_Page *pg);
   E_Wizard_Page_State state;
};

struct _E_Wizard_Api
{
   void (*wizard_go) (void);
   void (*wizard_apply) (void);
   void (*wizard_next) (void);
   void (*wizard_page_show) (Evas_Object *obj);
   E_Wizard_Page *(*wizard_page_add) (void *handle, const char *name,
                                      int (*init)     (E_Wizard_Page *pg, Eina_Bool *need_xdg_desktops, Eina_Bool *need_xdg_icons),
                                      int (*shutdown) (E_Wizard_Page *pg),
                                      int (*show)     (E_Wizard_Page *pg),
                                      int (*hide)     (E_Wizard_Page *pg),
                                      int (*apply)    (E_Wizard_Page *pg)
                                     );
   void (*wizard_page_del) (E_Wizard_Page *pg);
   void (*wizard_button_next_enable_set) (int enable);
   void (*wizard_title_set) (const char *title);
   void (*wizard_labels_update) (void);
   const char *(*wizard_dir_get) (void);
   void (*wizard_xdg_desktops_reset) (void);
};

/**
 * @addtogroup Optional_Conf
 * @{
 *
 * @defgroup Module_Wizard Wizard
 *
 * Configures the whole Enlightenment in a nice walk-through wizard.
 *
 * Usually is presented on the first run of Enlightenment. The wizard
 * pages are configurable and should be extended by distributions that
 * want to ship Enlightenment as the default window manager.
 *
 * @}
 */
#endif
