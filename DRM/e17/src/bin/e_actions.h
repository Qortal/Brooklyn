#ifdef E_TYPEDEFS

typedef struct _E_Action             E_Action;
typedef struct _E_Action_Description E_Action_Description;
typedef struct _E_Action_Group       E_Action_Group;

#else
#ifndef E_ACTIONS_H
#define E_ACTIONS_H

#define E_ACTION_TYPE 0xE0b01010

struct _E_Action
{
   E_Object    e_obj_inherit;

   const char *name;
   struct
   {
      void (*go)(E_Object *obj, const char *params);
      Eina_Bool (*go_mouse)(E_Object *obj, const char *params, E_Binding_Event_Mouse_Button *ev);
      Eina_Bool (*go_wheel)(E_Object *obj, const char *params, E_Binding_Event_Wheel *ev);
      void (*go_edge)(E_Object *obj, const char *params, E_Event_Zone_Edge *ev);
      void (*go_key)(E_Object *obj, const char *params, Ecore_Event_Key *ev);
      void (*go_signal)(E_Object *obj, const char *params, const char *sig, const char *src);
      void (*go_acpi)(E_Object *obj, const char *params, E_Event_Acpi *ev);
      void (*end)(E_Object *obj, const char *params);
      Eina_Bool (*end_mouse)(E_Object *obj, const char *params, E_Binding_Event_Mouse_Button *ev);
      void (*end_key)(E_Object *obj, const char *params, Ecore_Event_Key *ev);
   } func;
};

struct _E_Action_Description
{
   const char *act_name;
   const char *act_cmd;
   const char *act_params;
   const char *param_example;
   int         editable;
};

struct _E_Action_Group
{
   const char *act_grp;
   Eina_List  *acts;
};

EINTERN int      e_actions_init(void);
EINTERN int      e_actions_shutdown(void);

E_API Eina_List  *e_action_name_list(void);
E_API E_Action   *e_action_add(const char *name);
/* e_action_del allows, for example, modules to define their own actions dynamically. */
E_API void        e_action_del(const char *name);
E_API E_Action   *e_action_find(const char *name);

E_API const char *e_action_predef_label_get(const char *action, const char *params);
E_API void        e_action_predef_name_set(const char *act_grp, const char *act_name, const char *act_cmd, const char *act_params, const char *param_example, int editable);
E_API void        e_action_predef_name_del(const char *act_grp, const char *act_name);
E_API void        e_action_predef_name_all_del(void);
E_API Eina_List  *e_action_groups_get(void);

#endif
#endif
