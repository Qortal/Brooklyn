#include "config.h"

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca(size_t);
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Ipc.h>
#include <Ecore_File.h>
#include <Evas.h>
#include <Eet.h>
#include <Edje.h>
#include <Emotion.h>
#include "e_sha1.h"
#include "e_user.h"

typedef struct _E_Thumb E_Thumb;

struct _E_Thumb
{
   int        objid;
   int        w, h;
   int        desk_x, desk_y, desk_x_count, desk_y_count;
   Eina_List *sigsrc;
   char      *file;
   char      *key;
};

/* local subsystem functions */
static int       _e_ipc_init(void);
static Eina_Bool _e_ipc_cb_server_add(void *data,
                                      int type,
                                      void *event);
static Eina_Bool _e_ipc_cb_server_del(void *data,
                                      int type,
                                      void *event);
static Eina_Bool _e_ipc_cb_server_data(void *data,
                                       int type,
                                       void *event);
static Eina_Bool _e_cb_idle_enterer(void *data);
static void      _e_thumb_generate(E_Thumb *eth);
static char     *_e_thumb_file_id(char *file,
                                  char *key,
                                  int desk_x,
                                  int desk_y,
                                  int desk_x_count,
                                  int desk_y_count,
                                  Eina_List *sigsrc);

/* local subsystem globals */
static Ecore_Idle_Enterer *_idle_enterer = NULL;
static Ecore_Ipc_Server *_e_ipc_server = NULL;
static Eina_List *_thumblist = NULL;
static char _thumbdir[4096] = "";

/* externally accessible functions */
int
main(int argc,
     char **argv)
{
   int i;

   for (i = 1; i < argc; i++)
     {
        if ((!strcmp(argv[i], "-h")) ||
            (!strcmp(argv[i], "-help")) ||
            (!strcmp(argv[i], "--help")))
          {
             printf(
               "This is an internal tool for Enlightenment.\n"
               "do not use it.\n"
               );
             exit(0);
          }
        else if (!strncmp(argv[i], "--nice=", 7))
          {
             const char *val;

             val = argv[i] + 7;
             if (*val)
               {
                  if (nice(atoi(val)) < 0) perror("nice");
               }
          }
     }

   ecore_app_no_system_modules();
   ecore_init();
   ecore_app_args_set(argc, (const char **)argv);
   eet_init();
   evas_init();
   ecore_evas_init();
   edje_init();
   ecore_file_init();
   ecore_ipc_init();
   emotion_init();

   e_user_dir_concat_static(_thumbdir, "fileman/thumbnails");
   ecore_file_mkpath(_thumbdir);

   _idle_enterer = ecore_idle_enterer_add(_e_cb_idle_enterer, NULL);
   if (_idle_enterer)
     {
        if (_e_ipc_init()) ecore_main_loop_begin();
        ecore_idle_enterer_del(_idle_enterer);
        _idle_enterer = NULL;
     }

   if (_e_ipc_server)
     {
        ecore_ipc_server_del(_e_ipc_server);
        _e_ipc_server = NULL;
     }

   emotion_shutdown();
   ecore_ipc_shutdown();
   ecore_file_shutdown();
   ecore_evas_shutdown();
   edje_shutdown();
   evas_shutdown();
   eet_shutdown();
   ecore_shutdown();

   return 0;
}

/* local subsystem functions */
static int
_e_ipc_init(void)
{
   char *sdir;

   sdir = getenv("E_IPC_SOCKET");
   if (!sdir)
     {
        printf("The E_IPC_SOCKET environment variable is not set. This is\n"
               "exported by Enlightenment to all processes it launches.\n"
               "This environment variable must be set and must point to\n"
               "Enlightenment's IPC socket file (minus port number).\n");
        return 0;
     }
   _e_ipc_server = ecore_ipc_server_connect(ECORE_IPC_LOCAL_SYSTEM, sdir, 0, NULL);
   if (!_e_ipc_server) return 0;

   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_ADD, _e_ipc_cb_server_add, NULL);
   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DEL, _e_ipc_cb_server_del, NULL);
   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DATA, _e_ipc_cb_server_data, NULL);

   return 1;
}

static Eina_Bool
_e_ipc_cb_server_add(void *data EINA_UNUSED,
                     int type   EINA_UNUSED,
                     void *event)
{
   Ecore_Ipc_Event_Server_Add *e;

   e = event;
   ecore_ipc_server_send(e->server,
                         5 /*E_IPC_DOMAIN_THUMB*/,
                         1 /*hello*/,
                         0, 0, 0, NULL, 0); /* send hello */
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_ipc_cb_server_del(void *data  EINA_UNUSED,
                     int type    EINA_UNUSED,
                     void *event EINA_UNUSED)
{
   /* quit now */
   ecore_main_loop_quit();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_ipc_cb_server_data(void *data EINA_UNUSED,
                      int type   EINA_UNUSED,
                      void *event)
{
   Ecore_Ipc_Event_Server_Data *e;
   E_Thumb *eth;
   Eina_List *l;
   const char *file = NULL;
   const char *key = NULL;

   e = event;
   if (e->major != 5 /*E_IPC_DOMAIN_THUMB*/) return ECORE_CALLBACK_PASS_ON;
   switch (e->minor)
     {
      case 1:
        if (e->data)
          {
             const char *s, *start;
             const int *desk;
             Eina_List *sigsrc = NULL;

             /* begin thumb */
             /* don't check stuff. since this connects TO E it is connecting */
             /* TO a trusted process that WILL send this message properly */
             /* formatted. if the thumbnailer dies anyway - it's not a big loss */
             /* but it is a sign of a bug in e formatting messages maybe */
             s = start = e->data;
             desk = (const int *)(void *)(s);
             s += (4 * sizeof(int));
             file = s;
             s += strlen(s) + 1;
             key = s;
             s += strlen(s) + 1;
             if (!key[0]) key = NULL;
             while ((s - start) < e->size)
               {
                  sigsrc = eina_list_append(sigsrc, eina_stringshare_add(s));
                  s += strlen(s) + 1;
               }
             eth = calloc(1, sizeof(E_Thumb));
             if (eth)
               {
                  eth->objid = e->ref;
                  eth->w = e->ref_to;
                  eth->h = e->response;
                  eth->file = strdup(file);
                  eth->desk_x = desk[0];
                  eth->desk_y = desk[1];
                  eth->desk_x_count = desk[2];
                  eth->desk_y_count = desk[3];
                  eth->sigsrc = sigsrc;
                  if (key) eth->key = strdup(key);
                  _thumblist = eina_list_append(_thumblist, eth);
               }
          }
        break;

      case 2:
        /* end thumb */
        EINA_LIST_FOREACH(_thumblist, l, eth)
          {
             if (eth->objid == e->ref)
               {
                  _thumblist = eina_list_remove_list(_thumblist, l);
                  free(eth->file);
                  free(eth->key);
                  free(eth);
                  break;
               }
          }
        break;

      case 3:
        /* quit now */
        ecore_main_loop_quit();
        break;

      default:
        break;
     }
   return ECORE_CALLBACK_PASS_ON;
}

static void
_cb_wakeup(void *data EINA_UNUSED)
{
}

static Eina_Bool
_e_cb_idle_enterer(void *data EINA_UNUSED)
{
   E_Thumb *eth;

   /* take thumb at head of list */
   if (_thumblist)
     {
        const char *s;

        eth = eina_list_data_get(_thumblist);
        _thumblist = eina_list_remove_list(_thumblist, _thumblist);
        _e_thumb_generate(eth);
        EINA_LIST_FREE(eth->sigsrc, s) eina_stringshare_del(s);
        free(eth->file);
        free(eth->key);
        free(eth);
        if (_thumblist) ecore_job_add(_cb_wakeup, NULL);
     }
   return ECORE_CALLBACK_RENEW;
}

typedef struct _Color Color;

struct _Color
{
   Color        *closest;
   int           closest_dist;
   int           use;
   unsigned char r, g, b;
};

static void
_e_thumb_generate(E_Thumb *eth)
{
   char buf[PATH_MAX + 200], dbuf[PATH_MAX + 2], *id, *td, *ext = NULL;
   Evas *evas = NULL, *evas_im = NULL;
   Ecore_Evas *ee = NULL, *ee_im = NULL;
   Evas_Object *im = NULL, *edje = NULL;
   Eet_File *ef = NULL;
   int iw, ih, alpha, ww, hh;
   const unsigned int *data = NULL;
   time_t mtime_orig, mtime_thumb;

   id = _e_thumb_file_id(eth->file, eth->key,
                         0, 0, 1, 1,
//                         eth->desk_x, eth->desk_y,
//                         eth->desk_x_count, eth->desk_y_count,
                         eth->sigsrc);
   if (!id) return;

   td = strdup(id);
   if (!td)
     {
        free(id);
        return;
     }
   td[2] = 0;

   snprintf(dbuf, sizeof(dbuf), "%s/%s", _thumbdir, td);
   snprintf(buf, sizeof(buf), "%s/%s/%s-%ix%i.thm",
            _thumbdir, td, id + 2, eth->w, eth->h);
   free(id);
   free(td);

   mtime_orig = ecore_file_mod_time(eth->file);
   mtime_thumb = ecore_file_mod_time(buf);
   while (mtime_thumb <= mtime_orig)
     {
        unsigned int *data1;
        Eina_Bool sortkey;
        Evas_Object *im2, *bg;

        im = NULL;
        im2 = NULL;
        bg = NULL;

        ecore_file_mkdir(dbuf);

        edje_file_cache_set(0);
        edje_collection_cache_set(0);
        ee = ecore_evas_buffer_new(1, 1);
        evas = ecore_evas_get(ee);
        evas_image_cache_set(evas, 0);
        evas_font_cache_set(evas, 0);
        ww = 0;
        hh = 0;
        alpha = 1;
        ext = strrchr(eth->file, '.');

        sortkey = EINA_FALSE;

        if (emotion_object_extension_may_play_get(eth->file))
          {
             Evas_Object *em;

             em = emotion_object_add(evas);
             emotion_object_init(em, NULL);
             emotion_object_file_set(em, eth->file);

             im = emotion_file_meta_artwork_get(em, eth->file, EMOTION_ARTWORK_PREVIEW_IMAGE);
             if (!im) im = emotion_file_meta_artwork_get(em, eth->file, EMOTION_ARTWORK_IMAGE);
             if (im)
               {
                  evas_object_image_size_get(im, &ww, &hh);
                  evas_object_image_fill_set(im, 0, 0, ww, hh);
                  evas_object_move(im, 0, 0);
                  evas_object_resize(im, ww, hh);
               }
             evas_object_del(em);
             if (im) goto process;
          }

        if ((ext) && (eth->key) &&
            ((!strcasecmp(ext, ".edj")) ||
             (!strcasecmp(ext, ".eap"))))
          {
             Eina_List *l;

             ww = eth->w;
             hh = eth->h;
             im = ecore_evas_object_image_new(ee);
             ee_im = evas_object_data_get(im, "Ecore_Evas");
             evas_im = ecore_evas_get(ee_im);
             evas_image_cache_set(evas_im, 0);
             evas_font_cache_set(evas_im, 0);
             evas_object_image_size_set(im, ww * 4, hh * 4);
             evas_object_image_fill_set(im, 0, 0, ww, hh);
             edje = edje_object_add(evas_im);
             if ((eth->key) &&
                 ((!strcmp(eth->key, "e/desktop/background")) ||
                  (!strcmp(eth->key, "e/init/splash"))))
               alpha = 0;
             if (edje_object_file_set(edje, eth->file, eth->key))
               {
                  evas_object_move(edje, 0, 0);
                  evas_object_resize(edje, ww * 4, hh * 4);
                  evas_object_show(edje);
               }
             if ((eth->desk_x_count > 0) &&
                 (eth->desk_y_count > 0))
               {
                  Edje_Message_Float_Set *msg;

                  msg = alloca(sizeof(Edje_Message_Float_Set) +
                               (4 * sizeof(double)));
                  msg->count = 5;
                  msg->val[0] = 0.0;
                  msg->val[1] = eth->desk_x;
                  msg->val[2] = eth->desk_x_count;
                  msg->val[3] = eth->desk_y;
                  msg->val[4] = eth->desk_y_count;
                  edje_object_message_send(edje, EDJE_MESSAGE_FLOAT_SET,
                                           0, msg);
               }
             l = eth->sigsrc;
             while (l)
               {
                  const char *sig, *src;

                  sig = l->data;
                  l = l->next;
                  if (l)
                    {
                       src = l->data;
                       l = l->next;
                       edje_object_signal_emit(edje, sig, src);
                    }
               }
             edje_object_message_signal_process(edje);
             evas_object_move(im, 0, 0);
             evas_object_resize(im, ww, hh);
             sortkey = EINA_TRUE;
          }
        else if ((ext) &&
                 ((!strcasecmp(ext, ".ttf")) ||
                  (!strcasecmp(ext, ".pcf")) ||
                  (!strcasecmp(ext, ".bdf")) ||
                  (!strcasecmp(ext, ".ttx")) ||
                  (!strcasecmp(ext, ".pfa")) ||
                  (!strcasecmp(ext, ".pfb")) ||
                  (!strcasecmp(ext, ".afm")) ||
                  (!strcasecmp(ext, ".sfd")) ||
                  (!strcasecmp(ext, ".snf")) ||
                  (!strcasecmp(ext, ".otf")) ||
                  (!strcasecmp(ext, ".psf")) ||
                  (!strcasecmp(ext, ".ttc")) ||
                  (!strcasecmp(ext, ".ttx")) ||
                  (!strcasecmp(ext, ".gsf")) ||
                  (!strcasecmp(ext, ".spd"))
                 ))
          {
             Evas_Coord tx = 0, ty = 0, tw = 0, th = 0;
             ww = eth->w;
             hh = eth->h;
             alpha = 0;

             bg = evas_object_rectangle_add(evas);
             evas_object_color_set(bg, 96, 96, 96, 255);
             evas_object_move(bg, 0, 0);
             evas_object_resize(bg, ww, hh);
             evas_object_show(bg);

             im = evas_object_text_add(evas);
             evas_object_text_font_set(im, eth->file, hh / 4);
             evas_object_color_set(im, 192, 192, 192, 255);
             evas_object_text_ellipsis_set(im, 0.0);
             evas_object_text_text_set(im, "ABCabc");
             evas_object_geometry_get(im, NULL, NULL, &tw, &th);
             if (tw > ww) tw = ww;
             tx = 0 + ((ww - tw) / 2);
             ty = 0 + (((hh / 2) - th) / 2);
             evas_object_move(im, tx, ty);
             evas_object_resize(im, tw, th);
             evas_object_show(im);

             im2 = evas_object_text_add(evas);
             evas_object_text_font_set(im2, eth->file, hh / 4);
             evas_object_color_set(im2, 255, 255, 255, 255);
             evas_object_text_ellipsis_set(im2, 0.0);
             evas_object_text_text_set(im2, "123!@?");
             evas_object_geometry_get(im2, NULL, NULL, &tw, &th);
             if (tw > ww) tw = ww;
             tx = 0 + ((ww - tw) / 2);
             ty = (hh / 2) + (((hh / 2) - th) / 2);
             evas_object_move(im2, tx, ty);
             evas_object_resize(im2, tw, th);
             evas_object_show(im2);
          }
        else if (evas_object_image_extension_can_load_get(ext))
          {
             im = evas_object_image_add(evas);
             evas_object_image_load_orientation_set(im, EINA_TRUE);
             evas_object_image_load_size_set(im, eth->w, eth->h);
             evas_object_image_file_set(im, eth->file, NULL);
             iw = 0; ih = 0;
             evas_object_image_size_get(im, &iw, &ih);
             alpha = evas_object_image_alpha_get(im);
             if ((iw > 0) && (ih > 0))
               {
                  ww = eth->w;
                  hh = (eth->w * ih) / iw;
                  if (hh > eth->h)
                    {
                       hh = eth->h;
                       ww = (eth->h * iw) / ih;
                    }
                  evas_object_image_fill_set(im, 0, 0, ww, hh);
               }
             evas_object_move(im, 0, 0);
             evas_object_resize(im, ww, hh);
             sortkey = EINA_TRUE;
          }
        else
          goto end;

process:
        ecore_evas_alpha_set(ee, alpha);
        ecore_evas_resize(ee, ww, hh);
        evas_object_show(im);
        if (ww <= 0) goto end;
        data = ecore_evas_buffer_pixels_get(ee);
        if (!data) goto end;
        ef = eet_open(buf, EET_FILE_MODE_WRITE);
        if (!ef) goto end;
        eet_write(ef, "/thumbnail/orig_file",
                  eth->file, strlen(eth->file), 1);
        if (eth->key)
          eet_write(ef, "/thumbnail/orig_key",
                    eth->key, strlen(eth->key), 1);
        eet_data_image_write(ef, "/thumbnail/data",
                             (void *)data, ww, hh, alpha,
                             0, 91, 1);
        if (sortkey)
          {
             ww = 4; hh = 4;
             evas_object_image_fill_set(im, 0, 0, ww, hh);
             evas_object_resize(im, ww, hh);
             ecore_evas_resize(ee, ww, hh);
             data = ecore_evas_buffer_pixels_get(ee);
             if (!data) goto end;

             data1 = malloc(ww * hh * sizeof(unsigned int));
             memcpy(data1, data, ww * hh * sizeof(unsigned int));
             ww = 2; hh = 2;
             evas_object_image_fill_set(im, 0, 0, ww, hh);
             evas_object_resize(im, ww, hh);
             ecore_evas_resize(ee, ww, hh);
             data = ecore_evas_buffer_pixels_get(ee);
             if (data)
               {
                  unsigned int *data2;

                  data2 = malloc(ww * hh * sizeof(unsigned int));
                  memcpy(data2, data, ww * hh * sizeof(unsigned int));
                  ww = 1; hh = 1;
                  evas_object_image_fill_set(im, 0, 0, ww, hh);
                  evas_object_resize(im, ww, hh);
                  ecore_evas_resize(ee, ww, hh);
                  data = ecore_evas_buffer_pixels_get(ee);
                  if (data)
                    {
                       unsigned int *data3;
                       unsigned char id2[(21 * 4) + 1];
                       int n, i;
                       int hi, si, vi;
                       float h, s, v;
                       const int pat2[4] =
                         {
                            0, 3, 1, 2
                         };
                       const int pat1[16] =
                         {
                            5, 10, 6, 9,
                            0, 15, 3, 12,
                            1, 14, 7, 8,
                            4, 11, 2, 13
                         };

                       /* ww = hh = 1 here */
                       data3 = malloc(sizeof(unsigned int));
                       memcpy(data3, data, sizeof(unsigned int));
                       // sort_id
                       n = 0;
#define A(v) (((v) >> 24) & 0xff)
#define R(v) (((v) >> 16) & 0xff)
#define G(v) (((v) >> 8) & 0xff)
#define B(v) (((v)) & 0xff)
#define HSV(p)                                         \
  evas_color_rgb_to_hsv(R(p), G(p), B(p), &h, &s, &v); \
  hi = 20 * (h / 360.0);                               \
  si = 20 * s;                                         \
  vi = 20 * v;                                         \
  if (si < 2) hi = 25;
#define SAVEHSV(h, s, v) \
  id2[n++] = 'a' + h;    \
  id2[n++] = 'a' + v;    \
  id2[n++] = 'a' + s;
#define SAVEX(x) \
  id2[n++] = 'a' + x;
#if 0
                       HSV(data3[0]);
                       SAVEHSV(hi, si, vi);
                       for (i = 0; i < 4; i++)
                         {
                            HSV(data2[pat2[i]]);
                            SAVEHSV(hi, si, vi);
                         }
                       for (i = 0; i < 16; i++)
                         {
                            HSV(data1[pat1[i]]);
                            SAVEHSV(hi, si, vi);
                         }
#else
                       HSV(data3[0]);
                       SAVEX(hi);
                       for (i = 0; i < 4; i++)
                         {
                            HSV(data2[pat2[i]]);
                            SAVEX(hi);
                         }
                       for (i = 0; i < 16; i++)
                         {
                            HSV(data1[pat1[i]]);
                            SAVEX(hi);
                         }
                       HSV(data3[0]);
                       SAVEX(vi);
                       for (i = 0; i < 4; i++)
                         {
                            HSV(data2[pat2[i]]);
                            SAVEX(vi);
                         }
                       for (i = 0; i < 16; i++)
                         {
                            HSV(data1[pat1[i]]);
                            SAVEX(vi);
                         }
                       HSV(data3[0]);
                       SAVEX(si);
                       for (i = 0; i < 4; i++)
                         {
                            HSV(data2[pat2[i]]);
                            SAVEX(si);
                         }
                       for (i = 0; i < 16; i++)
                         {
                            HSV(data1[pat1[i]]);
                            SAVEX(si);
                         }
#endif
                       id2[n++] = 0;
                       eet_write(ef, "/thumbnail/sort_id", id2, n, 1);
                       free(data3);
                    }
                  free(data2);
               }
             free(data1);
          }
end:
        if (ef) eet_close(ef);

        /* will free all */
        if (edje) evas_object_del(edje);
        if (ee_im) ecore_evas_free(ee_im);
        else if (im) evas_object_del(im);
        if (im2) evas_object_del(im2);
        if (bg) evas_object_del(bg);
        ecore_evas_free(ee);
        eet_clearcache();
        break;
     }
   /* send back path to thumb */
   ecore_ipc_server_send(_e_ipc_server, 5, 2, eth->objid, 0, 0, buf, strlen(buf) + 1);
}

static char *
_e_thumb_file_id(char *file,
                 char *key,
                 int desk_x,
                 int desk_y,
                 int desk_x_count,
                 int desk_y_count,
                 Eina_List *sigsrc)
{
   char s[64];
   const char *chmap = "0123456789abcdef", *str;
   unsigned char id[20], *st;
   Eina_Strbuf *sbuf;
   int i;
   Eina_List *l;

   sbuf = eina_strbuf_new();
   EINA_LIST_FOREACH(sigsrc, l, str)
     {
        eina_strbuf_append_printf(sbuf, "<<%s>>", str);
     }
   eina_strbuf_append_printf(sbuf, "|%i.%i.%i.%i|",
                             desk_x, desk_y, desk_x_count, desk_y_count);
   eina_strbuf_append_printf(sbuf, "///%s", file);
   if (key) eina_strbuf_append_printf(sbuf, "/%s", key);

   st = (unsigned char *)eina_strbuf_string_get(sbuf);
   e_sha1_sum(st, eina_strbuf_length_get(sbuf), id);

   for (i = 0; i < 20; i++)
     {
        s[(i * 2) + 0] = chmap[(id[i] >> 4) & 0xf];
        s[(i * 2) + 1] = chmap[(id[i]) & 0xf];
     }
   s[(i * 2)] = 0;
   eina_strbuf_free(sbuf);

   return strdup(s);
}

