#include "app.h"
#include "carga.h"
#include "database.h"
#include "item_relatorio_mensal.h"
#include "relatorio_mensal.h"

struct _RelatorioMensal {
    GtkBox parent;

    GtkWidget *header_mes;
    GtkWidget *seletor_mes;
    GtkWidget *columnview;
    GtkWidget *column_mes;
    GtkFilter *string_filter;
    GListStore *data_model;
    GraosDb *db;
};

G_DEFINE_TYPE(RelatorioMensal, relatorio_mensal, GTK_TYPE_BOX);

static gboolean
eh_mesma_origem_mes(gconstpointer a, gconstpointer b)
{
    const ItemRelatorioMensal *item_a = (ItemRelatorioMensal *) a;
    const ItemRelatorioMensal *item_b = (ItemRelatorioMensal *) b;

    if (item_relatorio_mensal_get_origem(item_a) == item_relatorio_mensal_get_origem(item_b)
        && item_relatorio_mensal_get_mes_n(item_a) == item_relatorio_mensal_get_mes_n(item_b))
        return TRUE;
    else
        return FALSE;
}

static void
relatorio_mensal_add(RelatorioMensal *self, const Carga *carga)
{
    g_return_if_fail(carga_eh_valida(carga));

    ItemRelatorioMensal *novo = item_relatorio_mensal_new_from_carga(carga);
    unsigned pos;

    if (g_list_store_find_with_equal_func(self->data_model,
                                          G_OBJECT(novo),
                                          eh_mesma_origem_mes,
                                          &pos))
    {
        item_relatorio_mensal_add(
            g_list_model_get_item(G_LIST_MODEL(self->data_model), pos),
            novo
        );
    }
    else
        g_list_store_append(self->data_model, novo);

    g_object_unref(novo);
}

static void
relatorio_mensal_update(RelatorioMensal *self, gpointer data)
{
    relatorio_mensal_add(
        self,
        graos_db_get_item(self->db, graos_db_get_n_items(self->db) - 1)
    );
}

static void
relatorio_mensal_constructed(GObject *gobj)
{
    RelatorioMensal *self = RELATORIO_MENSAL(gobj);

    for (unsigned i = 0; i < graos_db_get_n_items(self->db); ++i)
    {
        const Carga *carga = graos_db_get_item(self->db, i);

        relatorio_mensal_add(self, carga);
    }

    g_signal_connect_swapped(
        self->db, "db-updated", G_CALLBACK(relatorio_mensal_update), self);

    G_OBJECT_CLASS(relatorio_mensal_parent_class)->constructed(gobj);
}

static void
relatorio_mensal_set_property(GObject *obj,
                              guint property_id,
                              const GValue *val,
                              GParamSpec *pspec)
{
    RelatorioMensal *self = RELATORIO_MENSAL(obj);

    switch (property_id)
    {
        case 1:
            self->db = GRAOS_DB(g_value_dup_object(val));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
    }
}

static void
relatorio_mensal_dispose(GObject *gobj)
{
    RelatorioMensal *self = RELATORIO_MENSAL(gobj);

    g_object_unref(self->db);

    G_OBJECT_CLASS(relatorio_mensal_parent_class)->dispose(gobj);
}

static void
change_mes_cb(GtkRange *source, gpointer data)
{
    RelatorioMensal *self = RELATORIO_MENSAL(data);
    GtkStringFilter *filter = GTK_STRING_FILTER(self->string_filter);

    const char *meses[] = {
        "TODOS",
        "JANEIRO",
        "FEVEREIRO",
        "MARÇO",
        "ABRIL",
        "MAIO",
        "JUNHO",
        "JULHO",
        "AGOSTO",
        "SETEMBRO",
        "OUTUBRO",
        "NOVEMBRO",
        "DEZEMBRO"
    };

    int mes = gtk_range_get_value(source);

    gtk_frame_set_label(GTK_FRAME(self->header_mes), meses[mes]);

    if (mes == 0)
        gtk_string_filter_set_search(filter, NULL);
    else
        gtk_string_filter_set_search(filter, meses[mes]);
}

static void
relatorio_mensal_init(RelatorioMensal *self)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect(
        self->seletor_mes,
        "value-changed",
        G_CALLBACK(change_mes_cb),
        self
    );

    gtk_column_view_sort_by_column(
        GTK_COLUMN_VIEW(self->columnview),
        GTK_COLUMN_VIEW_COLUMN(self->column_mes),
        GTK_SORT_ASCENDING
    );
}

static void
relatorio_mensal_class_init(RelatorioMensalClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);
    GtkWidgetClass *wid_cls = GTK_WIDGET_CLASS(cls);

    obj_cls->dispose = relatorio_mensal_dispose;
    obj_cls->constructed = relatorio_mensal_constructed;
    obj_cls->set_property = relatorio_mensal_set_property;

    g_type_ensure(ITEM_RELATORIO_MENSAL_TYPE);

    gtk_widget_class_set_template_from_resource(
        wid_cls, APP_NAME_PATH "/relatorio_mensal.ui");

    gtk_widget_class_bind_template_child(
        wid_cls, RelatorioMensal, seletor_mes);

    gtk_widget_class_bind_template_child(
        wid_cls, RelatorioMensal, header_mes);

    gtk_widget_class_bind_template_child(
        wid_cls, RelatorioMensal, data_model);

    gtk_widget_class_bind_template_child(
        wid_cls, RelatorioMensal, columnview);

    gtk_widget_class_bind_template_child(
        wid_cls, RelatorioMensal, column_mes);

    gtk_widget_class_bind_template_child(
        wid_cls, RelatorioMensal, string_filter);

    g_object_class_install_property(
        obj_cls,
        1,
        g_param_spec_object(
            "data-base", NULL, NULL, GRAOS_DB_TYPE,
            G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE
        )
    );
}

GtkWidget *
relatorio_mensal_new(GraosDb *db)
{
    return g_object_new(RELATORIO_MENSAL_TYPE, "data-base", db, NULL);
}
