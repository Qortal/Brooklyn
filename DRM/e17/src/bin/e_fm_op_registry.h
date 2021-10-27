#ifdef E_TYPEDEFS

typedef enum _E_Fm2_Op_Status
{
  E_FM2_OP_STATUS_UNKNOWN = 0,
  E_FM2_OP_STATUS_IN_PROGRESS,
  E_FM2_OP_STATUS_SUCCESSFUL,
  E_FM2_OP_STATUS_ABORTED,
  E_FM2_OP_STATUS_ERROR
} E_Fm2_Op_Status;

typedef struct _E_Fm2_Op_Registry_Entry E_Fm2_Op_Registry_Entry;

#else
#ifndef E_FM_OP_REGISTRY_H
#define E_FM_OP_REGISTRY_H

typedef void (*E_Fm2_Op_Registry_Abort_Func)(E_Fm2_Op_Registry_Entry *entry);

struct _E_Fm2_Op_Registry_Entry
{
   int id;
   int percent; /* XXX use char? */
   off_t done;
   off_t total;
   Evas_Object *e_fm;
   const char *src; /* stringshared */
   const char *dst; /* stringshared */
   double start_time;
   int eta; /* XXX use double? */
   E_Fm_Op_Type op;
   E_Fm2_Op_Status status;
   Eina_Bool needs_attention E_BITFIELD;
   E_Dialog *dialog;
   Eina_Bool finished E_BITFIELD;

   // service callbacks
   struct
     {
        E_Fm2_Op_Registry_Abort_Func abort;
     } func;
};

extern E_API int E_EVENT_FM_OP_REGISTRY_ADD;
extern E_API int E_EVENT_FM_OP_REGISTRY_DEL;
extern E_API int E_EVENT_FM_OP_REGISTRY_CHANGED;

E_API int e_fm2_op_registry_entry_ref(E_Fm2_Op_Registry_Entry *entry);
E_API int e_fm2_op_registry_entry_unref(E_Fm2_Op_Registry_Entry *entry);

E_API Ecore_X_Window e_fm2_op_registry_entry_xwin_get(const E_Fm2_Op_Registry_Entry *entry);

E_API E_Fm2_Op_Registry_Entry *e_fm2_op_registry_entry_get(int id);

E_API void e_fm2_op_registry_entry_listener_add(E_Fm2_Op_Registry_Entry *entry, void (*cb)(void *data, const E_Fm2_Op_Registry_Entry *entry), const void *data, void (*free_data)(void *data));
E_API void e_fm2_op_registry_entry_listener_del(E_Fm2_Op_Registry_Entry *entry, void (*cb)(void *data, const E_Fm2_Op_Registry_Entry *entry), const void *data);

E_API Eina_Iterator *e_fm2_op_registry_iterator_new(void);
E_API Eina_List     *e_fm2_op_registry_get_all(void);
E_API void           e_fm2_op_registry_get_all_free(Eina_List *list);
E_API Eina_Bool      e_fm2_op_registry_is_empty(void);
E_API int            e_fm2_op_registry_count(void);

E_API void           e_fm2_op_registry_entry_abort(E_Fm2_Op_Registry_Entry *entry);

EINTERN unsigned int e_fm2_op_registry_init(void);
EINTERN unsigned int e_fm2_op_registry_shutdown(void);

/* E internal/private functions, symbols not exported outside e binary (e_fm.c mainly) */
Eina_Bool e_fm2_op_registry_entry_add(int id, Evas_Object *e_fm, E_Fm_Op_Type op, E_Fm2_Op_Registry_Abort_Func abort);
Eina_Bool e_fm2_op_registry_entry_del(int id);
void      e_fm2_op_registry_entry_changed(const E_Fm2_Op_Registry_Entry *entry);
void      e_fm2_op_registry_entry_e_fm_set(E_Fm2_Op_Registry_Entry *entry, Evas_Object *e_fm);
void      e_fm2_op_registry_entry_files_set(E_Fm2_Op_Registry_Entry *entry, const char *src, const char *dst);

#endif
#endif
