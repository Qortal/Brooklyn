/* Ask about compositing */
#include "e_wizard.h"
#include "e_wizard_api.h"
#include <Evas_GL.h>

static Eina_Bool do_gl = 0;
static Eina_Bool do_vsync = 0;


static void
check_add(Evas_Object *box, const char *txt, Eina_Bool *val)
{
   Evas_Object *ck;

   ck = elm_check_add(box);
   E_ALIGN(ck, 0, 0.5);
   elm_object_text_set(ck, txt);
   elm_check_state_pointer_set(ck, val);
   elm_box_pack_end(box, ck);
   evas_object_show(ck);
}

E_API int
wizard_page_show(E_Wizard_Page *pg EINA_UNUSED)
{
   Evas_Object *o, *of;

   api->wizard_title_set(_("Compositing"));

   of = elm_frame_add(e_comp->elm);
   elm_object_text_set(of, _("Settings"));

   o = elm_box_add(of);
   elm_box_homogeneous_set(o, 1);
   elm_object_content_set(of, o);
   if (e_comp->gl)
     {
        Evas_GL *gl;

        gl = evas_gl_new(e_comp->evas);
        if (gl)
          {
             const char *str;
             Evas_GL_API *glapi = evas_gl_api_get(gl);
             str = (char*)glapi->glGetString(GL_RENDERER);
             if (str && (!strcasestr(str, "llvmpipe")))
               do_gl = do_vsync = 1;
             evas_gl_free(gl);
          }
     }
   if (!do_gl)
     {
#ifdef HAVE_WAYLAND
        if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
          {
             if (ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_OPENGL_DRM))
               {
                  void *egl = dlopen("libEGL.so.1", RTLD_NOW | RTLD_LOCAL);
                  if (!egl) egl = dlopen("libEGL.so", RTLD_NOW | RTLD_LOCAL);
                  if (egl)
                    {
                       do_gl = 1;
                       dlclose(egl);
                    }
               }
             do_vsync = 1;
          }
#endif
#ifndef HAVE_WAYLAND_ONLY
        if (e_comp->comp_type == E_PIXMAP_TYPE_X)
          {
             Ecore_X_Window_Attributes att;

             memset((&att), 0, sizeof(Ecore_X_Window_Attributes));
             ecore_x_window_attributes_get(ecore_x_window_root_first_get(), &att);
             if ((att.depth > 8) &&
                 (ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_OPENGL_X11)))
               {
                  Ecore_Evas *ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 32, 32);
                  if (ee)
                    {
                       do_gl = do_vsync = 1;
                       ecore_evas_free(ee);
                    }
               }
          }
#endif
     }
   check_add(o, _("Hardware Accelerated (OpenGL)"), &do_gl);
   check_add(o, _("Tear-free Rendering"), &do_vsync);

   evas_object_show(of);
   api->wizard_page_show(of);

   return 1; /* 1 == show ui, and wait for user, 0 == just continue */
}

E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
   E_Comp_Config *conf = NULL;

   conf = e_comp_config_get();
   if (do_gl)
     {
        conf->engine = E_COMP_ENGINE_GL;
        conf->smooth_windows = 1;
        conf->vsync = do_vsync;
     }
   else
     {
        conf->engine = E_COMP_ENGINE_SW;
        conf->smooth_windows = 0;
        conf->vsync = 0;
     }

   e_comp_internal_save();

   return 1;
}
