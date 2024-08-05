#pragma once

#include "database.h"

#include <gtk/gtk.h>

#define RELATORIO_MENSAL_TYPE (relatorio_mensal_get_type())
G_DECLARE_FINAL_TYPE(RelatorioMensal, relatorio_mensal, RELATORIO, MENSAL, GtkBox)

GtkWidget *relatorio_mensal_new(GraosDb *db);
