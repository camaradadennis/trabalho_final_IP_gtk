#pragma once

#include <glib-object.h>

#define ITEM_RELATORIO_MENSAL_TYPE (item_relatorio_mensal_get_type())
G_DECLARE_FINAL_TYPE(ItemRelatorioMensal, item_relatorio_mensal, ITEM, RELATORIO_MENSAL, GObject)

ItemRelatorioMensal *item_relatorio_mensal_new_from_carga(const Carga *carga);

int item_relatorio_mensal_get_origem(const ItemRelatorioMensal *self);
int item_relatorio_mensal_get_mes_n(const ItemRelatorioMensal *self);
void item_relatorio_mensal_add(ItemRelatorioMensal *self, const ItemRelatorioMensal *other);


