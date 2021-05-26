#ifndef E_WIZARD_PRIV_H

EINTERN int wizard_init(void);
EINTERN int wizard_shutdown(void);
EINTERN void wizard_go(void);
EINTERN void wizard_apply(void);
EINTERN void wizard_next(void);
EINTERN void wizard_page_show(Evas_Object *obj);
EINTERN E_Wizard_Page *wizard_page_add(void *handle, const char *name,
                                      int (*init)     (E_Wizard_Page *pg, Eina_Bool *need_xdg_desktops, Eina_Bool *need_xdg_icons),
                                      int (*shutdown) (E_Wizard_Page *pg),
                                      int (*show)     (E_Wizard_Page *pg),
                                      int (*hide)     (E_Wizard_Page *pg),
                                      int (*apply)    (E_Wizard_Page *pg)
                                     );
EINTERN void wizard_page_del(E_Wizard_Page *pg);
EINTERN void wizard_button_next_enable_set(int enable);
EINTERN void wizard_title_set(const char *title);
EINTERN void wizard_labels_update(void);
EINTERN const char *wizard_dir_get(void);
EINTERN void wizard_xdg_desktops_reset(void);

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
