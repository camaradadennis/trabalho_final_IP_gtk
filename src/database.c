#include "carga.h"
#include "database.h"

#include <gio/gio.h>
#include <glib-object.h>

struct _GraosDb {
    GObject parent;

    GFile *arquivo;
    Carga *contents;
    int n_itens;
};

G_DEFINE_TYPE(GraosDb, graos_db, G_TYPE_OBJECT)
G_DEFINE_QUARK (graos-db-error-quark, graos_db_error)

typedef enum {
    PROP_FILE = 1,
    N_PROPERTIES
} GraosDbProperty;

typedef enum {
    GRAOS_DB_ERROR_NO_NOVA_CARGA = 1
} GraosDbError;

static GParamSpec *graos_db_properties[N_PROPERTIES] = { NULL, };

static unsigned db_updated_signal;

gboolean
graos_db_is_empty(GraosDb *self)
{
    g_assert(GRAOS_IS_DB(self));

    if (self->contents && self->n_itens > 0)
        return TRUE;
    else
        return FALSE;
}

int
graos_db_get_n_items(GraosDb *self)
{
    g_assert(GRAOS_IS_DB(self));

    return self->n_itens;
}

Carga *
graos_db_get_item(GraosDb *self, unsigned pos)
{
    g_return_val_if_fail(GRAOS_IS_DB(self) || pos >= self->n_itens, NULL);

    return &(self->contents[pos]);
}

gboolean
graos_db_salvar_nova(GraosDb *self, const Carga *nova_carga, GError **err)
{
    if (!nova_carga)
    {
        g_set_error(
            err,
            GRAOS_DB_ERROR,
            GRAOS_DB_ERROR_NO_NOVA_CARGA,
            "Erro ao salvar a nova carga: nova carga inválida."
        );
        return FALSE;
    }

    GFileOutputStream *fstream = g_file_append_to(
        self->arquivo,
        G_FILE_CREATE_NONE,
        NULL,
        err
    );

    if (!fstream)
        return FALSE;

    if (g_output_stream_write(G_OUTPUT_STREAM(fstream),
                              nova_carga,
                              sizeof(Carga),
                              NULL,
                              err) == -1)
    {
        g_output_stream_close(G_OUTPUT_STREAM(fstream), NULL, NULL);
        return FALSE;
    }

    if (!g_output_stream_close(G_OUTPUT_STREAM(fstream), NULL, err))
        return FALSE;

    ++(self->n_itens);

    self->contents = g_renew(Carga, self->contents, self->n_itens);

    copia_carga(&(self->contents[self->n_itens - 1]), nova_carga);

    g_signal_emit(self, db_updated_signal, 0);

    return TRUE;
}

static void
graos_db_set_property(GObject *obj,
                      guint property_id,
                      const GValue *val,
                      GParamSpec *pspec)
{
    GraosDb *self = GRAOS_DB(obj);

    switch ((GraosDbProperty) property_id)
    {
        case PROP_FILE:
            if (G_IS_FILE(self->arquivo))
                g_object_unref(self->arquivo);

            self->arquivo = g_file_new_for_path(g_value_get_string(val));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
    }
}

static void
graos_db_get_property(GObject *obj,
                      guint property_id,
                      GValue *val,
                      GParamSpec *pspec)
{
    GraosDb *self = GRAOS_DB(obj);

    switch ((GraosDbProperty) property_id)
    {
        case PROP_FILE:
            char *fname = g_file_get_path(self->arquivo);
            g_value_set_string(val, fname);
            g_free(fname);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
    }
}

static void
graos_db_constructed(GObject *obj)
{
    GraosDb *self = GRAOS_DB(obj);

    GError *err = NULL;

    GBytes *bytes = g_file_load_bytes(self->arquivo, NULL, NULL, &err);

    if (!bytes)
    {
        g_warning(
            "Não foi possível ler o banco de dados em %s: %s. "
            "Iniciando um novo.", g_file_get_path(self->arquivo), err->message);
        g_clear_error(&err);

        return;
    }

    size_t size;
    self->contents = (Carga *) g_bytes_unref_to_data(bytes, &size);

    if (size <= 0 || size % sizeof(Carga) != 0)
    {
        g_free(self->contents);
        self->contents = NULL;

        g_warning(
            "Não foi possível ler o banco de dados em %s: arquivo impróprio.",
            g_file_get_path(self->arquivo));

        return;
    }

    self->n_itens = size / sizeof(Carga);

    G_OBJECT_CLASS(graos_db_parent_class)->constructed(obj);
}

static void
graos_db_init(GraosDb *self)
{
    self->arquivo = NULL;
    self->contents = NULL;
    self->n_itens = 0;
}

static void
graos_db_dispose(GObject *obj)
{
    GraosDb *self = GRAOS_DB(obj);

    if (G_IS_FILE(self->arquivo))
        g_object_unref(self->arquivo);

    g_free(self->contents);

    G_OBJECT_CLASS(graos_db_parent_class)->dispose(obj);
}

static void
graos_db_class_init(GraosDbClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);

    obj_cls->dispose = graos_db_dispose;
    obj_cls->set_property = graos_db_set_property;
    obj_cls->get_property = graos_db_get_property;
    obj_cls->constructed = graos_db_constructed;

    graos_db_properties[PROP_FILE] =
        g_param_spec_string(
            "arquivo",
            NULL,
            NULL,
            NULL,
            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
        );

    g_object_class_install_properties(obj_cls, N_PROPERTIES, graos_db_properties);

    db_updated_signal = g_signal_new(
        "db-updated",
        G_TYPE_FROM_CLASS(cls),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        0, NULL, NULL, NULL, G_TYPE_NONE, 0
    );
}

GraosDb *
graos_db_new(const char *arquivo)
{
    g_return_val_if_fail(arquivo, NULL);

    return g_object_new(GRAOS_DB_TYPE, "arquivo", arquivo, NULL);
}
