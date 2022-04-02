#ifndef E_COMP_X_RANDR_H
# define E_COMP_X_RANDR_H

E_API void e_comp_x_randr_init(void);
E_API void e_comp_x_randr_shutdown(void);
E_API void e_comp_x_randr_config_apply(void);
E_API Eina_Bool e_comp_x_randr_available(void);
E_API E_Randr2 *e_comp_x_randr_create(void);

E_API void e_comp_x_randr_screen_iface_set(void);
E_API Eina_Bool e_comp_x_randr_canvas_new(Ecore_Window parent, int w, int h);
#endif
