#include "e.h"

#include "eldbus_geo_clue2_manager.h"
#include "eldbus_geo_clue2_client.h"
#include "eldbus_geo_clue2_location.h"

#include <float.h>

/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);

/* and actually define the gadcon class that this module provides (just 1) */
static const E_Gadcon_Client_Class _gadcon_class =
{
   GADCON_CLIENT_CLASS_VERSION,
     "geolocation",
     {
        _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL, NULL
     },
   E_GADCON_CLIENT_STYLE_PLAIN
};

/* actual module specifics */
typedef struct _Instance Instance;

struct _Instance
{
   E_Gadcon_Client *gcc;
   Evas_Object     *icon;
   E_Gadcon_Popup  *popup;
   Evas_Object     *popup_label;
   Evas_Object     *popup_latitude;
   Evas_Object     *popup_longitude;
   Evas_Object     *popup_altitude;
   Evas_Object     *popup_speed;
   Evas_Object     *popup_heading;
   Evas_Object     *popup_accuracy;
   Evas_Object     *popup_description;
   int             in_use;
   int             available_accur_level;
   Eldbus_Connection *conn;
   Eldbus_Proxy *manager;
   Eldbus_Proxy *client;
   Eldbus_Proxy *location;
   double latitude;
   double longitude;
   double accuracy;
   double altitude;
   double speed;
   double heading;
   const char *description;
};

#define GCLUE_ACCURACY_LEVEL_NONE 0;
#define GCLUE_ACCURACY_LEVEL_COUNTRY 1;
#define GCLUE_ACCURACY_LEVEL_CITY 4;
#define GCLUE_ACCURACY_LEVEL_NEIGHBORHOOD 5;
#define GCLUE_ACCURACY_LEVEL_STREET 6;
#define GCLUE_ACCURACY_LEVEL_EXACT 8;

static Eina_List *geolocation_instances = NULL;
static E_Module *geolocation_module = NULL;

void
popup_update(Instance *inst)
{
   char buf[PATH_MAX];

   if (!inst->popup)
     return;

   snprintf(buf, sizeof(buf), _("Latitude:  %f"), inst->latitude);
   e_widget_label_text_set(inst->popup_latitude, buf);

   snprintf(buf, sizeof(buf), _("Longitude:  %f"), inst->longitude);
   e_widget_label_text_set(inst->popup_longitude, buf);

   if (eina_dbl_exact(inst->altitude, -DBL_MAX))
     snprintf(buf, sizeof(buf), _("Altitude:  %f"), inst->altitude);
   else
     snprintf(buf, sizeof(buf), _("Altitude:  N/A"));

   e_widget_label_text_set(inst->popup_altitude, buf);

   if (eina_dbl_exact(inst->speed, -1.0))
     snprintf(buf, sizeof(buf), _("Speed:  %f"), inst->speed);
   else
     snprintf(buf, sizeof(buf), _("Speed: N/A"));

   e_widget_label_text_set(inst->popup_speed, buf);

   if (eina_dbl_exact(inst->heading, -1.0))
     snprintf(buf, sizeof(buf), _("Heading:  %f"), inst->heading);
   else
     snprintf(buf, sizeof(buf), _("Heading: N/A"));

   e_widget_label_text_set(inst->popup_heading, buf);

   snprintf(buf, sizeof(buf), _("Accuracy:  %.1f m"), inst->accuracy);
   e_widget_label_text_set(inst->popup_accuracy, buf);
}

void
popup_del(Instance *inst)
{
   E_FREE_FUNC(inst->popup, e_object_del);
}

static void
_popup_del_cb(void *obj)
{
   popup_del(e_object_data_get(obj));
}

static void
_popup_autoclose_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   popup_del((Instance *)data);
}

void
popup_new(Instance *inst)
{
   Evas_Object *list, *oa;
   Evas *evas;
   char buf[PATH_MAX];

   inst->popup = e_gadcon_popup_new(inst->gcc, EINA_FALSE);
   evas = e_comp->evas;

   list = e_widget_list_add(evas, 0, 0);

   oa = e_widget_label_add(evas, _("Location information:"));
   e_widget_list_object_append(list, oa, 1, 1, 0.5);

   snprintf(buf, sizeof(buf), _("Latitude:  %f"), inst->latitude);
   inst->popup_latitude = e_widget_label_add(evas, buf);
   e_widget_list_object_append(list, inst->popup_latitude, 1, 1, 0.5);

   snprintf(buf, sizeof(buf), _("Longitude:  %f"), inst->longitude);
   inst->popup_longitude = e_widget_label_add(evas, buf);
   e_widget_list_object_append(list, inst->popup_longitude, 1, 1, 0.5);

   if (eina_dbl_exact(inst->altitude, -DBL_MAX))
     snprintf(buf, sizeof(buf), _("Altitude:  %f"), inst->altitude);
   else
     snprintf(buf, sizeof(buf), _("Altitude:  N/A"));

   inst->popup_altitude = e_widget_label_add(evas, buf);
   e_widget_list_object_append(list, inst->popup_altitude, 1, 1, 0.5);

   if (eina_dbl_exact(inst->speed, -1.0))
     snprintf(buf, sizeof(buf), _("Speed:  %f"), inst->speed);
   else
     snprintf(buf, sizeof(buf), _("Speed: N/A"));

   inst->popup_speed = e_widget_label_add(evas, buf);
   e_widget_list_object_append(list, inst->popup_speed, 1, 1, 0.5);

   if (eina_dbl_exact(inst->heading, -1.0))
     snprintf(buf, sizeof(buf), _("Heading:  %f"), inst->heading);
   else
     snprintf(buf, sizeof(buf), _("Heading: N/A"));

   inst->popup_heading = e_widget_label_add(evas, buf);
   e_widget_list_object_append(list, inst->popup_heading, 1, 1, 0.5);

   snprintf(buf, sizeof(buf), _("Accuracy:  %.1f m"), inst->accuracy);
   inst->popup_accuracy = e_widget_label_add(evas, buf);
   e_widget_list_object_append(list, inst->popup_accuracy, 1, 1, 0.5);

   popup_update(inst);

   e_gadcon_popup_content_set(inst->popup, list);
   e_comp_object_util_autoclose(inst->popup->comp_object,
                                _popup_autoclose_cb, NULL, inst);
   e_object_data_set(E_OBJECT(inst->popup), inst);
   E_OBJECT_DEL_SET(inst->popup, _popup_del_cb);

   e_gadcon_popup_show(inst->popup);
}

void
cb_client_start(Eldbus_Proxy *proxy EINA_UNUSED, void *data EINA_UNUSED,
		Eldbus_Pending *pending EINA_UNUSED, Eldbus_Error_Info *error EINA_UNUSED)
{
   DBG("Client proxy start callback received");
}

void
cb_client_stop(Eldbus_Proxy *proxy EINA_UNUSED, void *data EINA_UNUSED,
		Eldbus_Pending *pending EINA_UNUSED, Eldbus_Error_Info *error EINA_UNUSED)
{
   DBG("Client proxy stop callback received");
}

static void
_geolocation_cb_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Instance *inst = data;
   Evas_Event_Mouse_Down *ev = event;

   if (ev->button == 1)
     {
        if (inst->popup)
          {
             popup_del(inst);
             /* FIXME: There is a problem with starting a client again after stopping it in
              * GeoClue2 2.0.0 Need to test with a newer version to see if that is solved */
             //geo_clue2_client_stop_call(inst->client, cb_client_stop, inst);
          }
        else
          {
             geo_clue2_client_start_call(inst->client, cb_client_start, inst);
             popup_new(inst);
	  }
     }
   if (ev->button == 3)
     {
        E_Zone *zone;
        E_Menu *m;
        E_Menu_Item *mi;
        int x, y;

        zone = e_zone_current_get();

        m = e_menu_new();

        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, _("Settings"));
        e_util_menu_item_theme_icon_set(mi, "configure");

        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);

        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, NULL, NULL);
        e_menu_activate_mouse(m, zone, x + ev->output.x, y + ev->output.y,
                              1, 1, E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

void
cb_location_prop_latitude_get(void *data EINA_UNUSED, Eldbus_Pending *p EINA_UNUSED,
		              const char *propname EINA_UNUSED, Eldbus_Proxy *proxy EINA_UNUSED,
		              Eldbus_Error_Info *error_info EINA_UNUSED, double value)
{
   Instance *inst = data;
   inst->latitude = value;

   popup_update(inst);

   DBG("Location property Latitude: %f", value);
}

void
cb_location_prop_longitude_get(void *data EINA_UNUSED, Eldbus_Pending *p EINA_UNUSED,
		               const char *propname EINA_UNUSED, Eldbus_Proxy *proxy EINA_UNUSED,
		               Eldbus_Error_Info *error_info EINA_UNUSED, double value)
{
   Instance *inst = data;
   inst->longitude = value;

   popup_update(inst);

   DBG("Location property Longitude: %f", value);
}

void
cb_location_prop_accuracy_get(void *data EINA_UNUSED, Eldbus_Pending *p EINA_UNUSED,
		              const char *propname EINA_UNUSED, Eldbus_Proxy *proxy EINA_UNUSED,
		              Eldbus_Error_Info *error_info EINA_UNUSED, double value)
{
   Instance *inst = data;
   inst->accuracy = value;

   popup_update(inst);

   DBG("Location property Accuracy: %f", value);
}

void
cb_location_prop_altitude_get(void *data EINA_UNUSED, Eldbus_Pending *p EINA_UNUSED,
		              const char *propname EINA_UNUSED, Eldbus_Proxy *proxy EINA_UNUSED,
		              Eldbus_Error_Info *error_info EINA_UNUSED, double value)
{
   Instance *inst = data;
   inst->altitude = value;

   popup_update(inst);

   DBG("Location property Altitude: %f", value);
}

void
cb_location_prop_speed_get(void *data EINA_UNUSED, Eldbus_Pending *p EINA_UNUSED,
		           const char *propname EINA_UNUSED, Eldbus_Proxy *proxy EINA_UNUSED,
		           Eldbus_Error_Info *error_info EINA_UNUSED, double value)
{
   Instance *inst = data;
   inst->speed = value;

   popup_update(inst);

   DBG("Location property Speed: %f", value);
}

void
cb_location_prop_heading_get(void *data EINA_UNUSED, Eldbus_Pending *p EINA_UNUSED,
		             const char *propname EINA_UNUSED, Eldbus_Proxy *proxy EINA_UNUSED,
		             Eldbus_Error_Info *error_info EINA_UNUSED, double value)
{
   Instance *inst = data;
   inst->heading = value;

   popup_update(inst);

   DBG("Location property Heading: %f", value);
}

void
cb_location_prop_description_get(void *data EINA_UNUSED, Eldbus_Pending *p EINA_UNUSED,
		                 const char *propname EINA_UNUSED, Eldbus_Proxy *proxy EINA_UNUSED,
		                 Eldbus_Error_Info *error_info EINA_UNUSED, const char *value)
{
   Instance *inst = data;
   inst->description = value;

   popup_update(inst);

   DBG("Location property Description: %s", value);
}

void
cb_client_prop_set(void *data EINA_UNUSED, const char *propname EINA_UNUSED,
                              Eldbus_Proxy *proxy EINA_UNUSED, Eldbus_Pending *p EINA_UNUSED,
                              Eldbus_Error_Info *error_info EINA_UNUSED)
{
   DBG("Client %s property set callback received", propname);
}

void
cb_client_location_updated_signal(void *data, const Eldbus_Message *msg)
{
   const char *new_path, *old_path;
   Instance *inst = data;

   DBG("Client LocationUpdated signal received");

   if (!eldbus_message_arguments_get(msg, "oo", &old_path, &new_path))
     {
        ERR("Error: could not get location update");
        return;
     }
   DBG("Client signal location path old: %s", old_path);
   DBG("Client signal location path new: %s", new_path);

   inst->location = geo_clue2_location_proxy_get(inst->conn, "org.freedesktop.GeoClue2", new_path);
   if (!inst->location)
     {
        ERR("Error: could not connect to GeoClue2 location proxy");
        return;
     }

   geo_clue2_location_latitude_propget(inst->location, cb_location_prop_latitude_get, inst);
   geo_clue2_location_longitude_propget(inst->location, cb_location_prop_longitude_get, inst);
   geo_clue2_location_accuracy_propget(inst->location, cb_location_prop_accuracy_get, inst);
   geo_clue2_location_altitude_propget(inst->location, cb_location_prop_altitude_get, inst);
   geo_clue2_location_speed_propget(inst->location, cb_location_prop_speed_get, inst);
   geo_clue2_location_heading_propget(inst->location, cb_location_prop_heading_get, inst);
   geo_clue2_location_description_propget(inst->location, cb_location_prop_description_get, inst);
}

void
cb_client_object_get(Eldbus_Proxy *proxy EINA_UNUSED, void *data, Eldbus_Pending *pending EINA_UNUSED,
		     Eldbus_Error_Info *error EINA_UNUSED, const char *client_path)
{
   Instance *inst = data;
   int accuracy;
   const char *desktopid;

   DBG("Client object path: %s", client_path);
   inst->client = geo_clue2_client_proxy_get(inst->conn, "org.freedesktop.GeoClue2", client_path);
   if (!inst->client)
     {
        ERR("Error: could not connect to GeoClue2 client proxy");
        return;
     }

   desktopid = "Enlightenment-module";
   geo_clue2_client_desktop_id_propset(inst->client, cb_client_prop_set, inst, desktopid);
   accuracy = GCLUE_ACCURACY_LEVEL_NONE;
   geo_clue2_client_requested_accuracy_level_propset(inst->client, cb_client_prop_set, inst, (void*)(intptr_t)accuracy);

   eldbus_proxy_signal_handler_add(inst->client, "LocationUpdated", cb_client_location_updated_signal, inst);
}

static void
cb_manager_props_changed(void *data, Eldbus_Proxy *proxy EINA_UNUSED, void *event)
{
   Eldbus_Proxy_Event_Property_Changed *ev;
   int val;
   Eina_Value v;
   Instance *inst = data;

   ev = event;
   DBG("Manager property changed: %s", ev->name);

   if (strcmp(ev->name, "InUse") == 0)
     {
        eina_value_setup(&v, EINA_VALUE_TYPE_INT);
        eina_value_convert(ev->value, &v);
        eina_value_get(&v, &val);
        inst->in_use = val;
        DBG("Manager InUse property changed to %i", inst->in_use);

        if (inst->in_use)
          edje_object_signal_emit(inst->icon, "e,state,location_on", "e");
	else
          edje_object_signal_emit(inst->icon, "e,state,location_off", "e");
     }
   if (strcmp(ev->name, "AvailableAccuracyLevel") == 0)
     {
        eina_value_setup(&v, EINA_VALUE_TYPE_INT);
        eina_value_convert(ev->value, &v);
        eina_value_get(&v, &val);
        inst->available_accur_level = val;
        DBG("Manager AvailableAccuracyLevel property changed to %i", inst->available_accur_level);
     }
}

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;
   char buf[4096];

   inst = E_NEW(Instance, 1);

   o = edje_object_add(gc->evas);
   snprintf(buf, sizeof(buf), "%s/e-module-geolocation.edj",
	    e_module_dir_get(geolocation_module));
   edje_object_file_set(o, buf, "e/modules/geolocation/main");
   evas_object_show(o);

   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;

   inst->gcc = gcc;
   inst->icon = o;

   inst->latitude = 0.0;
   inst->longitude = 0.0;
   inst->accuracy = 0.0;
   inst->altitude = 0.0;
   inst->speed = 0.0;
   inst->heading = 0.0;
   inst->description = NULL;
   inst->in_use = 0;
   edje_object_signal_emit(inst->icon, "e,state,location_off", "e");

   evas_object_event_callback_add(inst->icon,
                                  EVAS_CALLBACK_MOUSE_DOWN,
                                  _geolocation_cb_mouse_down,
                                  inst);

   geolocation_instances = eina_list_append(geolocation_instances, inst);

   inst->conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   if (!inst->conn)
     {
        ERR("Error: could not get system bus.");
        return NULL;
     }

   inst->manager = geo_clue2_manager_proxy_get(inst->conn, "org.freedesktop.GeoClue2", "/org/freedesktop/GeoClue2/Manager");
   if (!inst->manager)
     {
        ERR("Error: could not connect to GeoClue2 manager proxy");
        return NULL;
     }

   eldbus_proxy_event_callback_add(inst->manager, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED, cb_manager_props_changed, inst);

   geo_clue2_manager_get_client_call(inst->manager, cb_client_object_get, inst);

   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;

   inst = gcc->data;
   geolocation_instances = eina_list_remove(geolocation_instances, inst);
   evas_object_del(inst->icon);
   geo_clue2_location_proxy_unref(inst->location);
   geo_clue2_client_proxy_unref(inst->client);
   geo_clue2_manager_proxy_unref(inst->manager);
   eldbus_connection_unref(inst->conn);
   free(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient EINA_UNUSED)
{
   Instance *inst;
   Evas_Coord mw, mh;

   inst = gcc->data;
   mw = 0, mh = 0;
   edje_object_size_min_get(inst->icon, &mw, &mh);
   if ((mw < 1) || (mh < 1))
     edje_object_size_min_calc(inst->icon, &mw, &mh);
   if (mw < 4) mw = 4;
   if (mh < 4) mh = 4;
   e_gadcon_client_aspect_set(gcc, mw, mh);
   e_gadcon_client_min_size_set(gcc, mw, mh);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _("Geolocation");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas)
{
   Evas_Object *o;
   char buf[4096];

   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/e-module-geolocation.edj",
	    e_module_dir_get(geolocation_module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   static char buf[4096];

   snprintf(buf, sizeof(buf), "%s.%d", client_class->name,
            eina_list_count(geolocation_instances) + 1);
   return buf;
}

/* module setup */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Geolocation"
};

E_API void *
e_modapi_init(E_Module *m)
{
   geolocation_module = m;
   e_gadcon_provider_register(&_gadcon_class);
   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   e_gadcon_provider_unregister(&_gadcon_class);
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   return 1;
}
