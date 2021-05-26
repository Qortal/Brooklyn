#include "e.h"

static void _e_xinerama_clean(void);
static void _e_xinerama_update(void);
static int  _e_xinerama_cb_screen_sort(const void *data1, const void *data2);

static Eina_List *all_screens = NULL;
static Eina_List *chosen_screens = NULL;
static Eina_List *fake_screens = NULL;

static void
_screen_free(E_Screen *scr)
{
   free(scr->id);
   free(scr);
}

EINTERN int
e_xinerama_init(void)
{
   return 1;
}

EINTERN int
e_xinerama_shutdown(void)
{
   _e_xinerama_clean();
   return 1;
}

E_API void
e_xinerama_update(void)
{
   _e_xinerama_clean();
   _e_xinerama_update();
}

E_API const Eina_List *
e_xinerama_screens_get(void)
{
   if (fake_screens) return fake_screens;
   return chosen_screens;
}

E_API const Eina_List *
e_xinerama_screens_all_get(void)
{
   if (fake_screens) return fake_screens;
   return all_screens;
}

E_API void
e_xinerama_screens_set(Eina_List *screens)
{
   E_FREE_LIST(all_screens, _screen_free);
   chosen_screens = eina_list_free(chosen_screens);
   all_screens = screens;
   _e_xinerama_update();
}

EINTERN void
e_xinerama_fake_screen_del(int num)
{
   Eina_List *l;
   E_Screen *scr, *lscr;
   int n = 0, x = 0;

   /* this assumes X number of identical horizontal screens */
   EINA_SAFETY_ON_NULL_RETURN(fake_screens);
   l = eina_list_nth_list(fake_screens, num);
   EINA_SAFETY_ON_NULL_RETURN(l);
   scr = eina_list_data_get(l);
   free(scr);
   fake_screens = eina_list_remove_list(fake_screens, l);
   EINA_LIST_FOREACH(fake_screens, l, lscr)
     {
        lscr->screen = n++;
        lscr->x = x;
        lscr->w = e_comp->w / eina_list_count(fake_screens);
        x = lscr->x + lscr->w;
     }
   e_comp_canvas_update();
   ecore_event_add(E_EVENT_RANDR_CHANGE, NULL, NULL, NULL);
}

EINTERN void
e_xinerama_fake_screen_append(void)
{
   Eina_List *l;
   E_Screen *lscr;
   int n, x = 0;

   /* this assumes X number of identical horizontal screens */
   n = eina_list_count(fake_screens) + 1;
   EINA_LIST_FOREACH(fake_screens, l, lscr)
     {
        lscr->x = x;
        lscr->w = e_comp->w / n;
        x = lscr->x + lscr->w;
     }
   e_xinerama_fake_screen_add(e_comp->w - (e_comp->w / n), 0, e_comp->w / n, e_comp->h);
   e_comp_canvas_update();
   ecore_event_add(E_EVENT_RANDR_CHANGE, NULL, NULL, NULL);
}

E_API void
e_xinerama_fake_screen_add(int x, int y, int w, int h)
{
   E_Screen *scr;

   scr = E_NEW(E_Screen, 1);
   scr->screen = eina_list_count(fake_screens);
   scr->escreen = scr->screen;
   scr->x = x;
   scr->y = y;
   scr->w = w;
   scr->h = h;
   fake_screens = eina_list_append(fake_screens, scr);
}

E_API Eina_Bool
e_xinerama_fake_screens_exist(void)
{
   return !!fake_screens;
}

/* local subsystem functions */
static void
_e_xinerama_clean(void)
{
   E_FREE_LIST(all_screens, _screen_free);
   chosen_screens = eina_list_free(chosen_screens);
   E_FREE_LIST(fake_screens, _screen_free);
}

static void
_e_xinerama_update(void)
{
   Eina_List *l;
   E_Screen *scr;
   unsigned int n = 0;

   /* now go through all_screens... and build a list of chosen screens */
   EINA_LIST_FOREACH(all_screens, l, scr)
     {
        Eina_List *ll;
        E_Screen *scr2;
        int add = 1;
        Eina_List *removes;

        removes = NULL;
        /* does this screen intersect with any we have chosen? */
        EINA_LIST_FOREACH(chosen_screens, ll, scr2)
          {
             /* if they intersect */
             if (E_INTERSECTS(scr->x, scr->y, scr->w, scr->h,
                              scr2->x, scr2->y, scr2->w, scr2->h))
               {
                  int sz, sz2;

                  /* calculate pixel area */
                  sz = scr->w * scr->h;
                  sz2 = scr2->w * scr2->h;
                  /* if the one we already have is bigger, DON'T add the new */
                  if (sz > sz2)
                    removes = eina_list_append(removes, scr2);
                  /* add the old to a list to remove */
                  else
                    add = 0;
               }
          }
        /* if there are screens to remove - remove them */
        EINA_LIST_FREE(removes, scr2)
          {
             chosen_screens = eina_list_remove(chosen_screens, scr2);
          }
        /* if this screen is to be added, add it */
        if (add)
          chosen_screens = eina_list_append(chosen_screens, scr);
     }
   chosen_screens = eina_list_sort(chosen_screens,
                                   eina_list_count(chosen_screens),
                                   _e_xinerama_cb_screen_sort);
   INF("======================= screens:");
   EINA_LIST_FOREACH(chosen_screens, l, scr)
     {
        INF("E INIT: XINERAMA CHOSEN: [%i][%i], %ix%i+%i+%i",
            scr->screen, scr->escreen, scr->w, scr->h, scr->x, scr->y);
        n++;
     }
}

static int
_e_xinerama_cb_screen_sort(const void *data1, const void *data2)
{
   const E_Screen *scr, *scr2;
   int dif;

   scr = data1;
   scr2 = data2;
   if (scr->x != scr2->x)
     return scr->x - scr2->x;
   else if (scr->y != scr2->y)
     return scr->y - scr2->y;
   else
     {
        dif = (scr2->w * scr2->h) - (scr->w * scr->h);
        if (dif == 0) return scr->screen - scr2->screen;
     }
   return dif;
}

