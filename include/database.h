#pragma once

#include "carga.h"

#include <glib-object.h>

#define GRAOS_DB_TYPE (graos_db_get_type())
#define GRAOS_DB_ERROR (graos_db_error_quark())

G_DECLARE_FINAL_TYPE(GraosDb, graos_db, GRAOS, DB, GObject)

GraosDb *graos_db_new(const char *arquivo);

gboolean graos_db_salvar_nova(GraosDb *self, const Carga *nova_carga, GError **err);
int graos_db_get_n_items(GraosDb *self);
Carga *graos_db_get_item(GraosDb *self, unsigned pos);
