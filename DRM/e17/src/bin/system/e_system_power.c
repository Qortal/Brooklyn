#include "e_system.h"

char *_cmd_halt = NULL;
char *_cmd_reboot = NULL;
char *_cmd_suspend = NULL;
char *_cmd_hibernate = NULL;

static void
_cb_power_halt(void *data EINA_UNUSED, const char *params EINA_UNUSED)
{
   if (_cmd_halt) e_system_run(_cmd_halt);
}

static void
_cb_power_reboot(void *data EINA_UNUSED, const char *params EINA_UNUSED)
{
   if (_cmd_reboot) e_system_run(_cmd_reboot);
}

static void
_cb_power_suspend(void *data EINA_UNUSED, const char *params EINA_UNUSED)
{
   if (_cmd_suspend) e_system_run(_cmd_suspend);
}

static void
_cb_power_hibernate(void *data EINA_UNUSED, const char *params EINA_UNUSED)
{
   if (_cmd_hibernate) e_system_run(_cmd_hibernate);
}

static void
_power_halt_init(void)
{
#if defined (__FreeBSD__) || defined (__OpenBSD__)
   _cmd_halt = strdup("shutdown -p now");
#else
   if (ecore_file_app_installed("systemctl"))
     _cmd_halt = strdup("systemctl poweroff");
   else
     _cmd_halt = strdup("shutdown -h now");
#endif
   // linux systemd: PATH/systemctl poweroff
   //           bsd: /sbin/shutdown -p no
   // *            : /sbin/shutdown -h now
}

static void
_power_reboot_init(void)
{
#if defined (__FreeBSD__) || defined (__OpenBSD__)
   _cmd_reboot = strdup("shutdown -r now");
#else
   if (ecore_file_app_installed("systemctl"))
     _cmd_reboot = strdup("systemctl reboot");
   else
     _cmd_reboot = strdup("shutdown -r now");
#endif
   // linux systemd: PATH/systemctl reboot
   //             *: /sbin/shutdown -r now
}

static void
_power_suspend_init(void)
{
#if defined (__FreeBSD__) || defined (__OpenBSD__)
   if (ecore_file_app_installed("zzz"))
     _cmd_suspend = strdup("zzz");
#else
   if (ecore_file_app_installed("systemctl"))
     _cmd_suspend = strdup("systemctl suspend");
   else if (ecore_file_app_installed("sleep.sh"))
     _cmd_suspend = strdup("sleep.sh");
   else if (ecore_file_can_exec("/etc/acpi/sleep.sh"))
     _cmd_suspend = strdup("/etc/acpi/sleep.sh force");
   else if (ecore_file_app_installed("pm-suspend"))
     _cmd_suspend = strdup("pm-suspend");
   else if (ecore_file_can_exec("/etc/acpi/pm-suspend"))
     _cmd_suspend = strdup("/etc/acpi/pm-suspend");
#endif
   // linux systemd: PATH/systemctl suspend
   //           bsd: /usr/sbin/zzz
   //             *:
   //    PATH/sleep.sh
   //    /etc/acpi/sleep.sh force
   //    PATH/pm-suspend
   //    /etc/acpi/pm-suspend
}

static void
_power_hibernate_init(void)
{
#if defined (__FreeBSD__) || defined (__OpenBSD__)
   if (ecore_file_app_installed("acpiconf"))
     _cmd_hibernate = strdup("acpiconf -s4");
#else
   if (ecore_file_app_installed("systemctl"))
     _cmd_hibernate = strdup("systemctl hibernate");
   else if (ecore_file_app_installed("hibernate.sh"))
     _cmd_hibernate = strdup("hibernate.sh");
   else if (ecore_file_can_exec("/etc/acpi/hibernate.sh"))
     _cmd_hibernate = strdup("/etc/acpi/hibernate.sh force");
   else if (ecore_file_app_installed("pm-hibernate"))
     _cmd_hibernate = strdup("pm-hibernate");
   else if (ecore_file_can_exec("/etc/acpi/pm-hibernate"))
     _cmd_hibernate = strdup("/etc/acpi/pm-hibernate");
#endif
   // linux systemd: PATH/systemctl hibernate
   // bsd: acpiconf -s4
   // if exist:
   //    PATH/hibernate.sh
   //    /etc/acpi/hibernate.sh force
   //    PATH/pm-hibernate
   //    /etc/acpi/pm-hibernate
}

void
e_system_power_init(void)
{
   _power_halt_init();
   _power_reboot_init();
   _power_suspend_init();
   _power_hibernate_init();
   e_system_inout_command_register("power-halt",       _cb_power_halt, NULL);
   e_system_inout_command_register("power-reboot",     _cb_power_reboot, NULL);
   e_system_inout_command_register("power-suspend",    _cb_power_suspend, NULL);
   e_system_inout_command_register("power-hibernate",  _cb_power_hibernate, NULL);
}

void
e_system_power_shutdown(void)
{
   // only shutdown things we really have to - no need to free mem etc.
}
