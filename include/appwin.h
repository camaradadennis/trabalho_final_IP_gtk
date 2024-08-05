#pragma once

#include <gtk/gtk.h>

#define GRAOS_APP_WIN_TYPE (graos_app_win_get_type())
G_DECLARE_FINAL_TYPE(GraosAppWin, graos_app_win, GRAOS, APP_WIN, GtkApplicationWindow)

GtkWindow *graos_app_win_new(GtkApplication *app);
