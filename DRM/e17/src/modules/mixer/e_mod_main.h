#ifndef _E_MOD_MAIN_H_
#define _E_MOD_MAIN_H_

#define CONFIG_VERSION 1

extern int _e_emix_log_domain;

#undef DBG
#undef INF
#undef WRN
#undef ERR
#undef CRIT
#define DBG(...) EINA_LOG_DOM_DBG(_e_emix_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INF(_e_emix_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_e_emix_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_e_emix_log_domain, __VA_ARGS__)
#define CRIT(...) EINA_LOG_DOM_CRIT(_e_emix_log_domain, __VA_ARGS__)

E_API extern E_Module_Api e_modapi;

E_API void *e_modapi_init(E_Module *m);
E_API int   e_modapi_shutdown(E_Module *m);
E_API int   e_modapi_save(E_Module *m);

#endif /* _E_MOD_MAIN_H_ */
