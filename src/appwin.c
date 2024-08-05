#include "app.h"
#include "appwin.h"
#include "database.h"

#include "novo_carregamento.h"
#include "relatorio_geral.h"
#include "relatorio_mensal.h"

#include <gtk/gtk.h>

#define DB_FILE "../Exemplos/XGraosRecebidos-%d.dat"
#define WINDOW_TITLE "UFG-BSI-IP (COOPERATIVA AGRÍCOLA GRÃO DO VALE V1.0)"

struct _GraosAppWin
{
    GtkApplicationWindow parent;

    GtkWidget *menu_btn;
    GtkWidget *header_label;
    GtkWidget *salvar_btn;
    GtkWidget *view_area;

    GDateTime *data_atual;
    GraosDb *db;
};

G_DEFINE_TYPE(GraosAppWin, graos_app_win, GTK_TYPE_APPLICATION_WINDOW);

static void
graos_app_win_ch_func(GraosAppWin *self, const char *function_name)
{
    const char *header_text =
        WINDOW_TITLE "\n"
        "ANO: %d <%s>";

    GString *str_buf = g_string_new(NULL);

    g_string_printf(str_buf, header_text,
                    g_date_time_get_year(self->data_atual),
                    function_name);

    gtk_label_set_label(GTK_LABEL(self->header_label), str_buf->str);

    g_string_free(str_buf, TRUE);

    gtk_widget_set_sensitive(self->salvar_btn, FALSE);
}

static void
salvar_nova_carga(GraosAppWin *self, gpointer data)
{
    NovoCarregamento *novo = NOVO_CARREGAMENTO(
        gtk_stack_get_child_by_name(
            GTK_STACK(self->view_area),
            "novo_carregamento"
        )
    );

    Carga *nova_carga = novo_carregamento_get_carga(novo);
    GError *err = NULL;

    if (!graos_db_salvar_nova(self->db, nova_carga, &err))
    {
        GtkAlertDialog *alert = gtk_alert_dialog_new("%s", err->message);
        gtk_alert_dialog_show(alert, GTK_WINDOW(self));
        g_object_unref(alert);
        g_clear_error(&err);
    }

    g_free(nova_carga);

    gtk_widget_set_sensitive(self->salvar_btn, FALSE);
}

static void
novo_carregamento_open(GObject *src, GAsyncResult *result, gpointer data)
{
    GtkFileDialog *dialog = GTK_FILE_DIALOG(src);
    GraosAppWin *self = GRAOS_APP_WIN(data);

    GError *err = NULL;
    GFile *file;
    char *contents;
    size_t len;

    GtkAlertDialog *alert;

    if (
        (file = gtk_file_dialog_open_finish(dialog, result, &err))
        && g_file_load_contents(file, NULL, &contents, &len, NULL, &err)
    )
    {
        if (g_utf8_validate(contents, len, NULL))
        {
            GtkWidget *novo = gtk_stack_get_child_by_name(
                GTK_STACK(self->view_area),
                "novo_carregamento"
            );

            graos_app_win_ch_func(self, "Novo Carregamento");

            gtk_stack_set_visible_child_name(
                GTK_STACK(self->view_area),
                "novo_carregamento"
            );

            mostra_nova_carga(NOVO_CARREGAMENTO(novo), contents, len);
        }
        else
        {
            alert = gtk_alert_dialog_new("%s", "Arquivo inválido para nova carga");
            gtk_alert_dialog_show(alert, GTK_WINDOW(self));
            g_object_unref(alert);
        }

        g_free(contents);
    }
    else if (err->code != GTK_DIALOG_ERROR_DISMISSED)
    {
        alert = gtk_alert_dialog_new("%s", err->message);
        gtk_alert_dialog_show(alert, GTK_WINDOW(self));
        g_object_unref(alert);
    }

    g_clear_error(&err);

    if (G_IS_FILE(file))
        g_object_unref(file);
}

static void
open_file_dialog(GSimpleAction *action, GVariant *parameter, gpointer data)
{
    GtkFileDialog *dialog = gtk_file_dialog_new();

    gtk_file_dialog_set_title(dialog, "Selecione o arquivo do novo carregamento");

    gtk_file_dialog_open(
        dialog, GTK_WINDOW(data),
        NULL, novo_carregamento_open, data
    );

    g_object_unref(dialog);
}

static void
exibir_relatorio_mensal(GSimpleAction *action, GVariant *parameter, gpointer data)
{
    GraosAppWin *win = GRAOS_APP_WIN(data);

    graos_app_win_ch_func(win, "Relatório Mensal");
    gtk_stack_set_visible_child_name(GTK_STACK(win->view_area), "rel_mensal");
}

static void
exibir_relatorio_geral(GSimpleAction *action, GVariant *parameter, gpointer data)
{
    GraosAppWin *win = GRAOS_APP_WIN(data);

    graos_app_win_ch_func(win, "Relatório Geral");
    gtk_stack_set_visible_child_name(GTK_STACK(win->view_area), "rel_geral");
}

static void
exibir_about_dialog(GSimpleAction *action, GVariant *parameter, gpointer data)
{
    GtkWindow *win = GTK_WINDOW(data);

    GtkAlertDialog *about = gtk_alert_dialog_new("%s",
        "Programa desenvolvido pelos alunos de Bacharelado em Sistemas de Informação "
        "na UFG:\n"
        "Grupo 8\n"
        "Kathleen Caroline Barbosa de Oliveira\n"
        "Dennis Lucas Gonçalves\n"
        "João Vitor Lemos Bonfim\n"
        "Victor Hugo Dutra Ferreira\n"
        "Yasser Gabriel Saboya Charafeddine\n"
    );

    gtk_alert_dialog_show(about, win);
    g_object_unref(about);
}

static void
enable_salvar_btn_cb(GraosAppWin *self, gpointer data)
{
    gtk_widget_set_sensitive(self->salvar_btn, TRUE);
}

static void
graos_app_win_constructed(GObject *obj)
{
    GraosAppWin *win = GRAOS_APP_WIN(obj);

    GtkWidget *relatorio_mensal = relatorio_mensal_new(win->db);
    gtk_stack_add_named(GTK_STACK(win->view_area), relatorio_mensal, "rel_mensal");

    GtkWidget *relatorio_geral = relatorio_geral_new(win->db);
    gtk_stack_add_named(GTK_STACK(win->view_area), relatorio_geral, "rel_geral");

    GtkWidget *novo = novo_carregamento_new();
    gtk_stack_add_named(GTK_STACK(win->view_area), novo, "novo_carregamento");

    g_signal_connect_swapped(
        NOVO_CARREGAMENTO(novo), "parse-success",
        G_CALLBACK(enable_salvar_btn_cb), win
    );

    gtk_stack_set_visible_child_name(GTK_STACK(win->view_area), "novo_carregamento");

    graos_app_win_ch_func(win, "Principal");

    G_OBJECT_CLASS(graos_app_win_parent_class)->constructed(obj);
}

static const GActionEntry menu_entries[] = {
    {"novo", open_file_dialog, NULL, NULL, NULL},
    {"rel_mensal", exibir_relatorio_mensal, NULL, NULL, NULL},
    {"rel_geral", exibir_relatorio_geral, NULL, NULL, NULL},
    {"sobre", exibir_about_dialog, NULL, NULL, NULL}
};

static void
graos_app_win_init(GraosAppWin *self)
{
    GtkBuilder *menu_builder;
    GMenuModel *menu_model;

    gtk_widget_init_template(GTK_WIDGET(self));

    gtk_window_set_title(GTK_WINDOW(self), WINDOW_TITLE);

    menu_builder = gtk_builder_new_from_resource(APP_NAME_PATH "/menu.ui");

    menu_model = G_MENU_MODEL(gtk_builder_get_object(menu_builder, "menu"));

    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(self->menu_btn), menu_model);

    g_object_unref(menu_builder);

    g_action_map_add_action_entries(G_ACTION_MAP(self), menu_entries,
                                    G_N_ELEMENTS(menu_entries),
                                    self);

    g_signal_connect_swapped(
        self->salvar_btn, "clicked",
        G_CALLBACK(salvar_nova_carga), self
    );

    self->data_atual = g_date_time_new_now_local();

    GString *str_buf = g_string_new(NULL);
    g_string_printf(str_buf, DB_FILE, g_date_time_get_year(self->data_atual));

    self->db = GRAOS_DB(graos_db_new(str_buf->str));

    g_string_free(str_buf, TRUE);
}

static void
graos_app_win_dispose(GObject *object)
{
    GraosAppWin *win = GRAOS_APP_WIN(object);

    g_date_time_unref(win->data_atual);
    g_object_unref(win->db);

    G_OBJECT_CLASS(graos_app_win_parent_class)->dispose(object);
}

static void
graos_app_win_class_init(GraosAppWinClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);
    GtkWidgetClass *wid_cls = GTK_WIDGET_CLASS(cls);

    obj_cls->dispose = graos_app_win_dispose;
    obj_cls->constructed = graos_app_win_constructed;

    gtk_widget_class_set_template_from_resource(
        wid_cls, APP_NAME_PATH "/window.ui");

    gtk_widget_class_bind_template_child(
        wid_cls, GraosAppWin, menu_btn);
    gtk_widget_class_bind_template_child(
        wid_cls, GraosAppWin, header_label);
    gtk_widget_class_bind_template_child(
        wid_cls, GraosAppWin, salvar_btn);
    gtk_widget_class_bind_template_child(
        wid_cls, GraosAppWin, view_area);
}

GtkWindow *
graos_app_win_new(GtkApplication *app)
{
    return g_object_new(GRAOS_APP_WIN_TYPE, "application", app, NULL);
}
