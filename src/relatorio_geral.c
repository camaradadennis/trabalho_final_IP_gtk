#include "app.h"
#include "database.h"

#include "item_relatorio_geral.h"
#include "relatorio_geral.h"

struct _RelatorioGeral {
    GtkBox parent;

    GtkWidget *columnview;
    GtkWidget *column_origem;
    GListStore *data_model;
    GraosDb *db;
};

G_DEFINE_TYPE(RelatorioGeral, relatorio_geral, GTK_TYPE_BOX);

static gboolean
eh_mesma_origem(gconstpointer a, gconstpointer b)
{
    const ItemRelatorioGeral *item_a = (ItemRelatorioGeral *) a;
    const ItemRelatorioGeral *item_b = (ItemRelatorioGeral *) b;

    if (item_relatorio_geral_get_origem(item_a) == item_relatorio_geral_get_origem(item_b))
        return TRUE;
    else
        return FALSE;
}

static void
relatorio_geral_add(RelatorioGeral *self, const Carga *carga)
{
    g_return_if_fail(carga_eh_valida(carga));

    ItemRelatorioGeral *novo = item_relatorio_geral_new_from_carga(carga);
    unsigned pos;

    if (g_list_store_find_with_equal_func(self->data_model,
                                          G_OBJECT(novo),
                                          eh_mesma_origem,
                                          &pos))
    {
        item_relatorio_geral_add(
            g_list_model_get_item(G_LIST_MODEL(self->data_model), pos),
            novo
        );
    }
    else
        g_list_store_append(self->data_model, novo);

    g_object_unref(novo);
}

static void
relatorio_geral_update(RelatorioGeral *self, gpointer data)
{
    relatorio_geral_add(
        self,
        graos_db_get_item(self->db, graos_db_get_n_items(self->db) - 1)
    );
}

static void
relatorio_geral_constructed(GObject *gobj)
{
    RelatorioGeral *self = RELATORIO_GERAL(gobj);

    for (unsigned i = 0; i < graos_db_get_n_items(self->db); ++i)
    {
        const Carga *carga = graos_db_get_item(self->db, i);

        relatorio_geral_add(self, carga);
    }

    g_signal_connect_swapped(
        self->db, "db-updated", G_CALLBACK(relatorio_geral_update), self);

    G_OBJECT_CLASS(relatorio_geral_parent_class)->constructed(gobj);
}

static void
relatorio_geral_set_property(GObject *obj,
                             guint property_id,
                             const GValue *val,
                             GParamSpec *pspec)
{
    RelatorioGeral *self = RELATORIO_GERAL(obj);

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
relatorio_geral_dispose(GObject *gobj)
{
    RelatorioGeral *self = RELATORIO_GERAL(gobj);

    g_object_unref(self->db);

    G_OBJECT_CLASS(relatorio_geral_parent_class)->dispose(gobj);
}

static void
relatorio_geral_init(RelatorioGeral *self)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    gtk_column_view_sort_by_column(
        GTK_COLUMN_VIEW(self->columnview),
        GTK_COLUMN_VIEW_COLUMN(self->column_origem),
        GTK_SORT_ASCENDING
    );
}

static void
relatorio_geral_class_init(RelatorioGeralClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);
    GtkWidgetClass *wid_cls = GTK_WIDGET_CLASS(cls);

    obj_cls->dispose = relatorio_geral_dispose;
    obj_cls->constructed = relatorio_geral_constructed;
    obj_cls->set_property = relatorio_geral_set_property;

    g_type_ensure(ITEM_RELATORIO_GERAL_TYPE);

    gtk_widget_class_set_template_from_resource(
        wid_cls, APP_NAME_PATH "/relatorio_geral.ui");

    gtk_widget_class_bind_template_child(
        wid_cls, RelatorioGeral, columnview);

    gtk_widget_class_bind_template_child(
        wid_cls, RelatorioGeral, column_origem);

    gtk_widget_class_bind_template_child(
        wid_cls, RelatorioGeral, data_model);

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
relatorio_geral_new(GraosDb *db)
{
    return g_object_new(RELATORIO_GERAL_TYPE, "data-base", db, NULL);
}
