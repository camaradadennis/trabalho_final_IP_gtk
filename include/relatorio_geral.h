#pragma once

#include "database.h"

#include <gtk/gtk.h>

#define RELATORIO_GERAL_TYPE (relatorio_geral_get_type())
G_DECLARE_FINAL_TYPE(RelatorioGeral, relatorio_geral, RELATORIO, GERAL, GtkBox)

GtkWidget *relatorio_geral_new(GraosDb *db);
