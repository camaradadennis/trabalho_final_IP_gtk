#include "app.h"

#include <gtk/gtk.h>

int
main()
{
    GraosApp *app = graos_app_new();

    int status = g_application_run(G_APPLICATION(app), 0, NULL);

    g_object_unref(app);
    return status;
}
