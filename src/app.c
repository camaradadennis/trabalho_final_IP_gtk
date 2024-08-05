#include "app.h"
#include "appwin.h"

#include <gtk/gtk.h>

#define FLOAT2STR_MAX_LEN 8

struct _GraosApp
{
    GtkApplication parent;
};

G_DEFINE_TYPE(GraosApp, graos_app, GTK_TYPE_APPLICATION);

static void
float2str(const GValue *src, GValue *dest)
{
    char str_buf[FLOAT2STR_MAX_LEN];

    g_snprintf(str_buf, FLOAT2STR_MAX_LEN, "%7.2f", g_value_get_float(src));
    g_value_set_string(dest, str_buf);
}

static void
graos_app_init(GraosApp *app)
{
}

static void
quit_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    g_application_quit(G_APPLICATION(user_data));
}

static void
graos_app_activate(GApplication *app)
{
    GtkWindow *win = graos_app_win_new(GTK_APPLICATION(app));
    gtk_window_present(win);
}

static void
graos_app_startup(GApplication *app)
{
    const GActionEntry app_entries[] = {
        {"quit", quit_activated, NULL, NULL, NULL}
    };

    G_APPLICATION_CLASS(graos_app_parent_class)->startup(app);

    g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries,
                                    G_N_ELEMENTS(app_entries),
                                    app);

    g_value_register_transform_func(G_TYPE_FLOAT, G_TYPE_STRING, float2str);
}

static void
graos_app_class_init(GraosAppClass *class)
{
    G_APPLICATION_CLASS(class)->startup = graos_app_startup;
    G_APPLICATION_CLASS(class)->activate = graos_app_activate;
}

GraosApp *
graos_app_new()
{
    return g_object_new(GRAOS_APP_TYPE,
                        "application-id", APP_NAME_DOT,
                        "flags", G_APPLICATION_DEFAULT_FLAGS,
                        NULL);
}
