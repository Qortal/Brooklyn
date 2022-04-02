/*
 * XML parsing abstraction interface header.
 * Contains public structs and lists externs which are further used.
 */

#ifndef E_MOD_PARSE_H
#define E_MOD_PARSE_H

typedef struct _E_XKB_Model
{
   const char *name;
   const char *description;
} E_XKB_Model;

typedef struct _E_XKB_Variant
{
   const char *name;
   const char *description;
} E_XKB_Variant;

typedef struct _E_XKB_Layout
{
   const char *name;
   const char *description;

   Eina_List  *variants;
} E_XKB_Layout;

typedef struct _E_XKB_Option
{
   const char *name;
   const char *description;
} E_XKB_Option;

int  parse_rules(void);
void clear_rules(void);
void find_rules(void);

int  layout_sort_by_name_cb(const void *data1, const void *data2);

extern Eina_List *models;
extern Eina_List *layouts;

extern Eina_List *optled;
extern Eina_List *optswitch;
extern Eina_List *optlv3;
extern Eina_List *optctrl;
extern Eina_List *optkeypad;
extern Eina_List *optdelkeypad;
extern Eina_List *optcapslock;
extern Eina_List *optaltwin;
extern Eina_List *optcompose;
extern Eina_List *optcurrency;
extern Eina_List *optlv5;
extern Eina_List *optspacebar;
extern Eina_List *optjapan;
extern Eina_List *optkorean;
extern Eina_List *optesperanto;
extern Eina_List *optsolaris;
extern Eina_List *optterminate;
extern Eina_List *optmisc;


#endif
