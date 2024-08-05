#pragma once

#include <glib-object.h>

#define ITEM_RELATORIO_GERAL_TYPE (item_relatorio_geral_get_type())
G_DECLARE_FINAL_TYPE(ItemRelatorioGeral, item_relatorio_geral, ITEM, RELATORIO_GERAL, GObject)

ItemRelatorioGeral *item_relatorio_geral_new_from_carga(const Carga *carga);

int item_relatorio_geral_get_origem(const ItemRelatorioGeral *self);
void item_relatorio_geral_add(ItemRelatorioGeral *self, const ItemRelatorioGeral *other);
