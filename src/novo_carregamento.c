#include "app.h"
#include "appwin.h"
#include "carga.h"

#include "novo_carregamento.h"

#include <stdio.h>
#include <string.h>

#define STR_BUF_DEFAULT_SIZE 1024

struct _NovoCarregamento {
    GtkBox parent;

    GObject *text_area;

    Carga *carga;
    int n_amostras;
    Amostra *amostras;
};

G_DEFINE_TYPE(NovoCarregamento, novo_carregamento, GTK_TYPE_BOX);

static unsigned parse_success_signal;

Carga *
novo_carregamento_get_carga(NovoCarregamento *self)
{
    g_return_val_if_fail(self->carga, NULL);

    // Uma cópia do recurso. Quem adquirí-la fica responsável por sua liberação.
    // Isso evita race conditions onde, por exemplo, GraosDb poderia tentar
    // salvar a nova carga logo após NovoCarregamento tê-la destruído.
    Carga *copia = g_new(Carga, 1);
    copia_carga(copia, self->carga);

    return copia;
}

static void
novo_carregamento_reset(NovoCarregamento *self)
{
    g_free(self->carga);
    g_free(self->amostras);
    self->n_amostras = 0;
}

static void
set_text_buffer_from_nova(NovoCarregamento *self)
{
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(self->text_area);

    float peso_limpo = self->carga->peso_bruto_t * (1 - self->carga->perc_impurezas / 100);

    GString *str_buf = g_string_sized_new(STR_BUF_DEFAULT_SIZE);

    g_string_append_printf(str_buf,
        "Origem: %03d             Núm. de Amostras: %03d             Data:%02d/%02d\n"
        "Umidade: %04.1f%%           Peso limpo (t): %06.2f          Transgênico: %s\n\n",
        self->carga->origem, self->n_amostras, self->carga->dia_receb,
        self->carga->mes_receb, self->carga->perc_umidade, peso_limpo,
        self->carga->tp_transgenico? "Sim" : "Não"
    );

    int amostras_por_faixa[get_n_faixas_gu()][self->n_amostras + 1];

    for (int i = 0; i < get_n_faixas_gu(); ++i)
        amostras_por_faixa[i][0] = 0;

    for (int i = 0; i < self->n_amostras; ++i)
    {
        int faixa = get_faixa_gu(self->amostras[i].grau_umidade);

        amostras_por_faixa[faixa][0] += 1;

        amostras_por_faixa[faixa][amostras_por_faixa[faixa][0]] =
            self->amostras[i].ident;
    }

    for (int i = 0; i < get_n_faixas_gu(); ++i)
    {
        g_string_append_printf(str_buf,
            "Umidade: %s                                Quant.: %03d\n"
            "Ident. das Amostras: ",
            get_faixa_gu_nome(i), amostras_por_faixa[i][0]
        );

        for (int j = 1; j <= amostras_por_faixa[i][0]; ++j)
        {
            g_string_append_printf(str_buf,
                "%04d, ", amostras_por_faixa[i][j]
            );
        }

        g_string_append(str_buf, "\n\n");
    }

    gtk_text_buffer_set_text(buffer, str_buf->str, str_buf->len);

    g_string_free(str_buf, TRUE);
}

static void
set_text_buffer_from_error(NovoCarregamento *self, const char *file_contents, size_t len)
{
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(self->text_area);

    gtk_text_buffer_set_text(buffer,
        "Erro de formatação do arquivo: verifique se o arquivo abaixo está "
        "formatado corretamente antes de tentar abrí-lo.\n\n"
        "---------------------------------------------------------------\n\n\n",
        -1
    );

    gtk_text_buffer_insert_at_cursor(
        buffer,
        file_contents,
        len
    );
}

static float
PercentualUmidade(const Amostra amostras[], int n)
{

    float somaNumerador = 0.0;
    float somaDivisor = 0.0;

    for(int i = 0; i < n; ++i)
    {
        float diff =
            amostras[i].peso_bruto_kg * 1000 - amostras[i].peso_impurezas_g;

        somaNumerador += amostras[i].grau_umidade * diff;
        somaDivisor += diff;
    }

    if (somaDivisor != 0)
        return somaNumerador / somaDivisor;
    else
        return -1;
}

static float
PercentualImpurezas(const Amostra amostras[], int n)
{
    float somaPeso = 0.0;
    float somaImpurezas = 0.0;

    for(int i = 0; i < n; ++i)
    {
        somaPeso += amostras[i].peso_bruto_kg * 1000; // convertendo para gramas
        somaImpurezas += amostras[i].peso_impurezas_g;
    }

    if (somaPeso != 0)
        return somaImpurezas / somaPeso;
    else
        return -1;
}

static gboolean
parse_nova_carga(NovoCarregamento *self, char *file_contents)
{
    int n_amostras;
    char *linha = strtok(file_contents, "\n");

    if (!linha)
        return FALSE;

    Carga *nova_carga = g_new(Carga, 1);

    int n_campos_linha = sscanf(linha,
                                "%d %d %f %d %d",
                                &nova_carga->origem,
                                &nova_carga->cod_carga,
                                &nova_carga->peso_bruto_t,
                                &n_amostras,
                                &nova_carga->tp_transgenico
    );

    if (n_campos_linha != 5 || n_amostras <= 0)
    {
        g_free(nova_carga);
        return FALSE;
    }

    Amostra *amostras = g_new(Amostra, n_amostras);

    int i = 0;
    while ((linha = strtok(NULL, "\n")) != NULL && i < n_amostras)
    {
        n_campos_linha = sscanf(linha,
                                "%d %f %d %f",
                                &amostras[i].ident,
                                &amostras[i].peso_bruto_kg,
                                &amostras[i].peso_impurezas_g,
                                &amostras[i].grau_umidade
        );

        if (n_campos_linha == 4)
            ++i;
        else
            break;
    }

    if (i != n_amostras)
    {
        g_free(amostras);
        g_free(nova_carga);
        return FALSE;
    }

    GDateTime *now = g_date_time_new_now_local();

    nova_carga->mes_receb = g_date_time_get_month(now);
    nova_carga->dia_receb = g_date_time_get_day_of_month(now);

    g_date_time_unref(now);

    nova_carga->perc_impurezas = PercentualImpurezas(amostras, n_amostras);
    nova_carga->perc_umidade = PercentualUmidade(amostras, n_amostras);

    if (carga_eh_valida(nova_carga))
    {
        self->carga = nova_carga;
        self->amostras = amostras;
        self->n_amostras = n_amostras;
        return TRUE;
    }

    g_free(nova_carga);
    g_free(amostras);
    return FALSE;
}

void
mostra_nova_carga(NovoCarregamento *self, const char *file_contents, size_t len)
{
    novo_carregamento_reset(self);

    // A cópia é necessária, porque parser_nova_carga() usa strtok, que
    // modifica a string.
    char *copia = g_strndup(file_contents, len);

    if (parse_nova_carga(self, copia))
    {
        set_text_buffer_from_nova(self);
        g_signal_emit(self, parse_success_signal, 0);
    }
    else
        set_text_buffer_from_error(self, file_contents, len);

    g_free(copia);
}

static void
novo_carregamento_init(NovoCarregamento *self)
{
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void
novo_carregamento_class_init(NovoCarregamentoClass *cls)
{
    //GObjectClass *obj_cls = G_OBJECT_CLASS(cls);
    GtkWidgetClass *wid_cls = GTK_WIDGET_CLASS(cls);

    gtk_widget_class_set_template_from_resource(
        wid_cls, APP_NAME_PATH "/novo_carregamento.ui");

    gtk_widget_class_bind_template_child(
        wid_cls, NovoCarregamento, text_area);

    parse_success_signal = g_signal_new(
        "parse-success",
        G_TYPE_FROM_CLASS(cls),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        0, NULL, NULL, NULL, G_TYPE_NONE, 0
    );
}

GtkWidget *
novo_carregamento_new()
{
    return g_object_new(NOVO_CARREGAMENTO_TYPE, NULL);
}
