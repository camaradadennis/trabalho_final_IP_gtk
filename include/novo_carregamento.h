#pragma once

#include <gtk/gtk.h>

#define NOVO_CARREGAMENTO_TYPE (novo_carregamento_get_type())
G_DECLARE_FINAL_TYPE(NovoCarregamento, novo_carregamento, NOVO, CARREGAMENTO, GtkBox)

GtkWidget *novo_carregamento_new();

void mostra_nova_carga(NovoCarregamento *self, const char *file_contents, size_t len);
Carga *novo_carregamento_get_carga(NovoCarregamento *self);
