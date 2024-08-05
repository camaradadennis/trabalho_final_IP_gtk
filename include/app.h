#pragma once

#include <gtk/gtk.h>

#define GRAOS_APP_TYPE (graos_app_get_type())
G_DECLARE_FINAL_TYPE(GraosApp, graos_app, GRAOS, APP, GtkApplication)

#define APP_NAME_DOT "si_ip.grupo8.graos"
#define APP_NAME_PATH "/si_ip/grupo8/graos"

GraosApp *graos_app_new();
