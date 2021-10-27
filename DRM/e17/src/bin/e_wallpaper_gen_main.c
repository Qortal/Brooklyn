#include <Elementary.h>
#include "config.h"

// a set of commonly found resolution buckets to try size the image down
// to to match width OR height so we can avoid decoding a larger image from
// disk making startup time faster by picking the right res that's encoded
// already in the file... the more resolutions we encode the bigger
// the file so it's a tradeoff, thus commenting out a lot of the resolutions
// because they are so close or can just miildly crop another res...
// only encode resolutiosn less than the original image a well as throw in
// the original as the highest res option
// pick onne of each y resolution and scale accordingly so e.g. Nx480, Nx600,
// Nx720, Nx788, Nx800, Nx900, ...
static const int resolutions[] =
{
   // not bothering below 640x480
//    640,  480,
//    800,  480,
    854,  480,
//    768,  576,
//   1024,  576,
//    800,  600,
   1024,  600,
   1280,  720,
//   1024,  768,
//   1152,  768
//   1280,  768
   1366,  768,
   1280,  800,
//   1280,  854,
//   1152,  864,
//   1152,  900,
//   1440,  900,
   1600,  900,
//   1280,  960,
   1440,  960,
//   1280, 1024,
//   1400, 1050,
//   1680, 1050,
//   1440, 1080,
//   1920, 1080,
//   2048, 1080,
//   2880, 1080,
   3840, 1080,
//   1600, 1200,
   1920, 1200,
//   1920, 1280,
   2560, 1440,
//   3840, 1440,
//   5120, 1440,
//   2048, 1536,
   2560, 1600,
//   2880, 1620,
//   2880, 1800,
   3200, 1800,
//   2560, 2048,
//   3840, 2160,
   4096, 2160,
//   5120, 2160,
   5120, 2880,
   6016, 3384,
//   7680, 4320,
   8192, 4320,

// one day... we may have to add more resolutions here. when that day comes...
// we'll do that. until then... I think 8k is "good enough"
   0, 0 // sentinel
};
// when we have more res's above bump this to be that + 1 at a minimum
#define MAX_RES_NUM 20

#define ETC1 -1
#define ETC2 -2

static Evas_Object *win = NULL, *subwin = NULL, *image = NULL, *rend = NULL;

typedef struct
{
   int from_w, from_h, to_w, to_h;
   Eina_Bool last : 1;
} Mip;

static Mip *
_resolutions_calc(int w, int h)
{
   int i, j = 0;
   int nw, nh, pw = 0, ph = 0;
   Mip *mips = calloc(1, sizeof(Mip) * MAX_RES_NUM);

   if (!mips) return NULL;
   for (i = 0; resolutions[i]; i += 2)
     {
        nh = resolutions[i + 1];
        nw = (w * nh) / h;
        if ((nh >= h) || (nw >= w))
          {
             mips[j].from_w = pw;
             mips[j].from_h = ph;
             mips[j].to_w = 1000000000;
             mips[j].to_h = 1000000000;
             mips[j].last = EINA_TRUE;
             break;
          }

        mips[j].from_w = pw;
        mips[j].from_h = ph;
        mips[j].to_w = nw;
        mips[j].to_h = nh;

        j++;
        pw = nw + 1;
        ph = nh + 1;
     }
   return mips;
}

EAPI int
elm_main(int argc, char **argv)
{
   const char *edje_cc, *mode, *file, *outfile;
   int bg_r = 64;
   int bg_g = 64;
   int bg_b = 64;
   char dir_buf[128], img_buf[256], edc_buf[256], cmd_buf[1024], qual_buf[64];
   const char *dir, *quality_string = NULL, *img_name = NULL;
   Mip *mips = NULL;
   int i, imw, imh, w, h, quality;
   int ret = 0, mips_num = 0;
   Eina_Bool alpha;
   FILE *f;

   if (argc <= 1)
     {
        printf("USAGE: enlightenment_wallpaper_gen EDJE FILL INPUT OUT QUALITY [R G B]\n"
               "\n"
               "  EDJE    is edje_cc command to use (full path or in $PATH)\n"
               "  FILL    is one of:\n"
               "    stretch\n"
               "    tile\n"
               "    center (requires R G B)\n"
               "    scale_in (requires R G B)\n"
               "    scale_out\n"
               "    pan\n"
               "  INPUT   is some image file to put into the wallpaper edj\n"
               "  OUT     is the paht/location of the wallpaper edj output\n"
               "  QUALITY is:\n"
               "    0-100 (0-99 is lossy, 100 is lossless compression)\n"
               "    etc1\n"
               "    etc2\n"
               "  R, G, B are Red, Green and Blue values 0 to 255\n"
               "\n"
               "  e.g.\n"
               "    enlightenment_wallpaper_gen edje_cc scale_out cat.jpg wallpaper.edj 80\n"
               "    enlightenment_wallpaper_gen edje_cc center mylogo.png wallpaper.edj 100 48 64 64\n"
               "    enlightenment_wallpaper_gen /usr/local/bin/edje_cc tile pattern.jpg wallpaper.edj 98\n"
               "    enlightenment_wallpaper_gen /opt/e/bin/edje_cc scale_out hugeimg.jpg wallpaper.edj etc1\n"
               );
        return 1;
     }

   elm_config_preferred_engine_set("buffer");
   win = elm_win_add(NULL, "Wallpaper-Gen", ELM_WIN_BASIC);
   elm_win_norender_push(win);
   evas_object_show(win);

   subwin = elm_win_add(win, "inlined", ELM_WIN_INLINED_IMAGE);
   rend = elm_win_inlined_image_object_get(subwin);
   elm_win_norender_push(subwin);
   evas_object_show(subwin);

   if (argc < 6) return 2;
   edje_cc = argv[1];
   mode = argv[2];
   file = argv[3];
   outfile = argv[4];
   if      (!strcmp(argv[5], "etc1")) quality = ETC1;
   else if (!strcmp(argv[5], "etc2")) quality = ETC2;
   else
     {
        quality = atoi(argv[5]);
        if      (quality < 0)   quality = 0;
        else if (quality > 100) quality = 100;
     }

   image = evas_object_image_filled_add(evas_object_evas_get(subwin));
   evas_object_image_file_set(image, file, NULL);
   evas_object_image_size_get(image, &w, &h);
   if ((w <= 0) || (h <= 0)) return 3;
   alpha = evas_object_image_alpha_get(image);
   elm_win_alpha_set(subwin, alpha);
   evas_object_show(image);

   snprintf(dir_buf, sizeof(dir_buf), "/tmp/e_bg-XXXXXX");
   dir = mkdtemp(dir_buf);
   if (!dir) return 4;

   if ((!strcmp(mode, "center")) ||
       (!strcmp(mode, "scale_in")))
     { // need backing rgb color here
        if (argc < 9) return 5;
        bg_r = atoi(argv[6]);
        bg_g = atoi(argv[7]);
        bg_b = atoi(argv[8]);
     }
   if ((!strcmp(mode, "stretch")) ||
       (!strcmp(mode, "scale_in")) ||
       (!strcmp(mode, "scale_out")) ||
       (!strcmp(mode, "pan")))
     { // need to produce multiple scaled versions
        mips = _resolutions_calc(w, h);
        if (!mips) return 6;
        for (i = 0; mips[i].to_w; i++)
          {
             mips_num++;
             if (mips[i].last)
               {
                  imw = w;
                  imh = h;
               }
             else
               {
                  imw = mips[i].to_w;
                  imh = mips[i].to_h;
               }
             evas_object_resize(subwin, imw, imh);
             evas_object_resize(image, imw, imh);
             elm_win_render(subwin);
             if (mips[i].last)
               snprintf(img_buf, sizeof(img_buf), "%s/img.png",
                        dir);
             else
               snprintf(img_buf, sizeof(img_buf), "%s/img-%ix%i.png",
                        dir, imw, imh);
             if (!evas_object_image_save(rend, img_buf, NULL, "compress=0"))
               {
                  ret = 7;
                  goto cleanup;
               }
          }
     }
   // no multiple resolutions -0 save out original
   if (!mips)
     {
        evas_object_resize(subwin, w, h);
        evas_object_resize(image, w, h);
        elm_win_render(subwin);
        snprintf(img_buf, sizeof(img_buf), "%s/img.png", dir);
        if (!evas_object_image_save(rend, img_buf, NULL, "compress=0"))
          {
             ret = 8;
             goto cleanup;
          }
     }
   if ((quality == ETC1) && (alpha)) quality = ETC2; // etc1 -> etc2 if alpha
   if (quality == 100) quality_string = "COMP";
   else
     {
        if      (quality == ETC1) quality_string = "LOSSY_ETC1";
        else if (quality == ETC2) quality_string = "LOSSY_ETC2";
        else
          {
             snprintf(qual_buf, sizeof(qual_buf), "LOSSY %i", quality);
             quality_string = qual_buf;
          }
     }
   // generate edc
   snprintf(edc_buf, sizeof(edc_buf), "%s/bg.edc", dir);
   f = fopen(edc_buf, "w");
   if (!f) goto cleanup;
   if ((mips) && (mips_num > 1))
     {
        fprintf(f,
                "images {\n"
                " set { name: \"img\";\n");
        for (i = mips_num - 1; i >= 0; i--)
          {
             fprintf(f,
                     "  image {\n");
             if (mips[i].last)
               {
                  fprintf(f,
                          "   image: \"img.png\" %s;\n",
                          quality_string);
               }
             else
               {
                  imw = mips[i].to_w;
                  imh = mips[i].to_h;
                  fprintf(f,
                          "   image: \"img-%ix%i.png\" %s;\n",
                          imw, imh, quality_string);
               }
             fprintf(f,
                     "   size: %i %i %i %i;\n"
                     "  }\n",
                     mips[i].from_w, mips[i].from_h,
                     mips[i].to_w, mips[i].to_h);
          }
        fprintf(f,
                " }\n"
                "}\n");
        img_name = "img";
     }
   else
     {
        fprintf(f, "images.image: \"img.png\" %s;\n", quality_string);
        img_name = "img.png";
     }
   fprintf(f,
           "collections {\n"
           " group { name: \"e/desktop/background\";\n"
           "  data.item: \"noanimation\" \"1\";\n");
   if (!strcmp(mode, "stretch"))
     {
        fprintf(f, "  data { item: \"style\" \"0\"; }\n");
        fprintf(f,
                "  parts {\n"
                "   part { name: \"bg\"; mouse_events: 0;\n"
                "    description { state: \"default\" 0;\n"
                "     image {\n"
                "      normal: \"%s\";\n"
                "      scale_hint: STATIC;\n"
                "     }\n"
                "    }\n"
                "   }\n"
                "  }\n"
                , img_name);
     }
   else if (!strcmp(mode, "tile"))
     {
        fprintf(f, "  data { item: \"style\" \"1\"; }\n");
        fprintf(f,
                "  parts {\n"
                "   part { name: \"bg\"; mouse_events: 0;\n"
                "    description { state: \"default\" 0;\n"
                "     image {\n"
                "      normal: \"%s\";\n"
                "     }\n"
                "     fill.size.relative: 0 0;\n"
                "     fill.size.offset: %i %i;\n"
                "    }\n"
                "   }\n"
                "  }\n"
                , img_name, w, h);
     }
   else if (!strcmp(mode, "center"))
     {
        fprintf(f, "  data { item: \"style\" \"2\"; }\n");
        fprintf(f,
                "  parts {\n"
                "   part { name: \"col\"; type: RECT; mouse_events: 0;\n"
                "    description { state: \"default\" 0;\n"
                "     color: %i %i %i 255;\n"
                "    }\n"
                "   }\n"
                "   part { name: \"bg\"; mouse_events: 0;\n"
                "    description { state: \"default\" 0;\n"
                "     image {\n"
                "      normal: \"%s\";\n"
                "     }\n"
                "     min: %i %i; max: %i %i;\n"
                "    }\n"
                "   }\n"
                "  }\n"
                , bg_r, bg_g, bg_b, img_name, w, h, w, h);
     }
   else if (!strcmp(mode, "scale_in"))
     {
        fprintf(f, "  data { item: \"style\" \"3\"; }\n");
        fprintf(f,
                "  parts {\n"
                "   part { name: \"col\"; type: RECT; mouse_events: 0;\n"
                "    description { state: \"default\" 0;\n"
                "     color: %i %i %i 255;\n"
                "    }\n"
                "   }\n"
                "   part { name: \"bg\"; mouse_events: 0;\n"
                "    description { state: \"default\" 0;\n"
                "     image {\n"
                "      normal: \"%s\";\n"
                "      scale_hint: STATIC;\n"
                "     }\n"
                "     aspect: (%i/%i) (%i/%i); aspect_preference: BOTH;\n"
                "    }\n"
                "   }\n"
                "  }\n"
                , bg_r, bg_g, bg_b, img_name, w, h, w, h);
     }
   else if (!strcmp(mode, "scale_out"))
     {
        fprintf(f, "  data { item: \"style\" \"4\"; }\n");
        fprintf(f,
                "  parts {\n"
                "   part { name: \"bg\"; mouse_events: 0;\n"
                "    description { state: \"default\" 0;\n"
                "     image {\n"
                "      normal: \"%s\";\n"
                "      scale_hint: STATIC;\n"
                "     }\n"
                "     aspect: (%i/%i) (%i/%i); aspect_preference: NONE;\n"
                "    }\n"
                "   }\n"
                "  }\n"
                , img_name, w, h, w, h);
     }
   else if (!strcmp(mode, "pan"))
     {
        fprintf(f, "  data { item: \"style\" \"5\"; }\n");
        fprintf(f,
                "  script {\n"
                "   public cur_anim; public cur_x; public cur_y;\n"
                "   public prev_x; public prev_y;\n"
                "   public total_x; public total_y;\n"
                "   public pan_bg(val, Float:v) {\n"
                "    new Float:x, Float:y, Float:px, Float: py;\n"
                "    px = get_float(prev_x);\n"
                "    py = get_float(prev_y);\n"
                "    if (get_int(total_x) > 1) {\n"
                "     x = float(get_int(cur_x)) / (get_int(total_x) - 1);\n"
                "     x = px - (px - x) * v;\n"
                "    } else {\n"
                "     x = 0.0;\n"
                "     v = 1.0;\n"
                "    }\n"
                "    if (get_int(total_y) > 1) {\n"
                "     y = float(get_int(cur_y)) / (get_int(total_y) - 1);\n"
                "     y = py - (py - y) * v;\n"
                "    } else {\n"
                "     y = 0.0;\n"
                "     v = 1.0; }\n"
                "    set_state_val(PART:\"bg\", STATE_ALIGNMENT, x, y);\n"
                "    if (v >= 1.0) {\n"
                "     set_int(cur_anim, 0);\n"
                "     set_float(prev_x, x);\n"
                "     set_float(prev_y, y);\n"
                "     return 0;\n"
                "    }\n"
                "    return 1;\n"
                "   }\n"
                "   public message(Msg_Type:type, id, ...) {\n"
                "    if ((type == MSG_FLOAT_SET) && (id == 0)) {\n"
                "     new ani;\n"
                "     get_state_val(PART:\"bg\", STATE_ALIGNMENT, prev_x, prev_y);\n"
                "     set_int(cur_x, round(getfarg(3)));\n"
                "     set_int(total_x, round(getfarg(4)));\n"
                "     set_int(cur_y, round(getfarg(5)));\n"
                "     set_int(total_y, round(getfarg(6)));\n"
                "     ani = get_int(cur_anim);\n"
                "     if (ani > 0) cancel_anim(ani);\n"
                "     ani = anim(getfarg(2), \"pan_bg\", 0);\n"
                "     set_int(cur_anim, ani);\n"
                "    }\n"
                "   }\n"
                "  }\n"
                "  parts {\n"
                "   part { name: \"bg\"; mouse_events: 0;\n"
                "    description { state: \"default\" 0.0;\n"
                "     image {\n"
                "      normal: \"%s\";\n"
                "      scale_hint: STATIC;\n"
                "     }\n"
                "     aspect: (%i/%i) (%i/%i); aspect_preference: NONE;\n"
                "    }\n"
                "   }\n"
                "   program {\n"
                "    signal: \"load\"; source: \"\";\n"
                "    script {\n"
                "     custom_state(PART:\"bg\", \"default\", 0.0);\n"
                "     set_state(PART:\"bg\", \"custom\", 0.0);\n"
                "     set_float(prev_x, 0.0);\n"
                "     set_float(prev_y, 0.0);\n"
                "    }\n"
                "   }\n"
                "  }\n"
                , img_name, w, h, w, h);
     }
   fprintf(f,
           " }\n"
           "}\n");
   fclose(f);
   if (snprintf(cmd_buf, sizeof(cmd_buf),
                "%s -fastdecomp -id %s -fd %s -sd %s -vd %s -dd %s -md %s "
                "%s/bg.edc %s",
                edje_cc, dir, dir, dir, dir, dir, dir,
                dir, outfile) >= (int)sizeof(cmd_buf))
     {
        ret = 9;
        goto cleanup;
     }
   ret = system(cmd_buf);
cleanup:
   free(mips);
   ecore_file_recursive_rm(dir);
   evas_object_del(win);
   return ret;
}
ELM_MAIN()
