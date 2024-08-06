#include "carga.h"
#include "item_relatorio_mensal.h"

static const char *nome_mes[] = {
    NULL, // Porque não existe mês 0
    "Janeiro",
    "Fevereiro",
    "Março",
    "Abril",
    "Maio",
    "Junho",
    "Julho",
    "Agosto",
    "Setembro",
    "Outubro",
    "Novembro",
    "Dezembro"
};

struct _ItemRelatorioMensal {
    GObject parent;

    int mes_num;
    int origem;
    int cargas;
    int gu_faixa_1;
    int gu_faixa_2;
    int gu_faixa_3;
    int gu_extra;
};

typedef enum {
    MES = 1,
    ORIGEM,
    CARGAS,
    GU_FAIXA_1,
    GU_FAIXA_2,
    GU_FAIXA_3,
    GU_EXTRA,
    N_CAMPOS
} ItemRelatorioMensalProp;

static GParamSpec *item_relatorio_mensal_properties[N_CAMPOS] = { NULL, };

G_DEFINE_TYPE(ItemRelatorioMensal, item_relatorio_mensal, G_TYPE_OBJECT)

int
item_relatorio_mensal_get_origem(const ItemRelatorioMensal *self)
{
    return self->origem;
}

int
item_relatorio_mensal_get_mes_n(const ItemRelatorioMensal *self)
{
    return self->mes_num;
}

/* item_relatorio_mensal_add()
 * Esta função soma os valores já armazenados em `self` com os valores de `other`
 * e emite um sinal "notify" quando algum dos valores de fato mudou.
 * O sinal é importante por que é capturado pela columnviewcolumn, que atualiza
 * o valor daquela propriedade na tabela.
 */
void
item_relatorio_mensal_add(ItemRelatorioMensal *self, const ItemRelatorioMensal *other)
{
    g_return_if_fail(self->origem == other->origem && self->mes_num == other->mes_num);

    GObject *gobj = G_OBJECT(self);

    self->cargas += other->cargas;
    g_object_notify_by_pspec(gobj, item_relatorio_mensal_properties[CARGAS]);

    // `other` codidifica os valores de um único carregamento,
    // por isso, ele pode estar nas faixas 1, 2, 3 ou extra.
    // Nunca em mais de uma.
    if (other->gu_faixa_1)
    {
        self->gu_faixa_1 += other->gu_faixa_1;
        g_object_notify_by_pspec(gobj, item_relatorio_mensal_properties[GU_FAIXA_1]);
    }
    else if (other->gu_faixa_2)
    {
        self->gu_faixa_2 += other->gu_faixa_2;
        g_object_notify_by_pspec(gobj, item_relatorio_mensal_properties[GU_FAIXA_2]);
    }
    else if (other->gu_faixa_3)
    {
        self->gu_faixa_3 += other->gu_faixa_3;
        g_object_notify_by_pspec(gobj, item_relatorio_mensal_properties[GU_FAIXA_3]);
    }
    else
    {
        self->gu_extra += other->gu_extra;
        g_object_notify_by_pspec(gobj, item_relatorio_mensal_properties[GU_EXTRA]);
    }
}

static void
item_relatorio_mensal_fit(ItemRelatorioMensal *self, const Carga *carga)
{
    int faixa_gu = get_faixa_gu(carga->perc_umidade);

    self->origem = carga->origem;
    self->mes_num = carga->mes_receb;
    ++self->cargas;

    switch (faixa_gu)
    {
        case 0:
            ++self->gu_faixa_1;
            break;
        case 1:
            ++self->gu_faixa_2;
            break;
        case 2:
            ++self->gu_faixa_3;
            break;
        case 3:
            ++self->gu_extra;
            break;
        default:
            g_warning("Faixa GU inválida: %d", faixa_gu);
    }
}

static void
item_relatorio_mensal_get_property(GObject *obj,
                                   guint property_id,
                                   GValue *val,
                                   GParamSpec *pspec)
{
    ItemRelatorioMensal *self = ITEM_RELATORIO_MENSAL(obj);

    switch ((ItemRelatorioMensalProp) property_id)
    {
        case MES:
            g_value_set_static_string(val, nome_mes[self->mes_num]);
            break;
        case ORIGEM:
            g_value_set_int(val, self->origem);
            break;
        case CARGAS:
            g_value_set_int(val, self->cargas);
            break;
        case GU_FAIXA_1:
            g_value_set_int(val, self->gu_faixa_1);
            break;
        case GU_FAIXA_2:
            g_value_set_int(val, self->gu_faixa_2);
            break;
        case GU_FAIXA_3:
            g_value_set_int(val, self->gu_faixa_3);
            break;
        case GU_EXTRA:
            g_value_set_int(val, self->gu_extra);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
    }
}

static void
item_relatorio_mensal_init(ItemRelatorioMensal *self)
{
}

static void
item_relatorio_mensal_class_init(ItemRelatorioMensalClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);

    obj_cls->get_property = item_relatorio_mensal_get_property;

    item_relatorio_mensal_properties[MES] =
        g_param_spec_string(
            "mes", NULL, NULL, NULL,
            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS
        );

    // O campo 0 de item_relatorio_mensal_properties é reservado
    // e o campo 1 já foi construído acima
    const char *prop_names[] = {
        NULL, NULL, "origem", "cargas",
        "gu-faixa-1", "gu-faixa-2", "gu-faixa-3", "gu-extra"
    };

    for (ItemRelatorioMensalProp prop = ORIGEM; prop < N_CAMPOS; ++prop)
    {
        item_relatorio_mensal_properties[prop] =
            g_param_spec_int(
                prop_names[prop], NULL, NULL, 0, G_MAXINT32, 0,
                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY
            );
    }

    g_object_class_install_properties(obj_cls, N_CAMPOS,
        item_relatorio_mensal_properties);
}

ItemRelatorioMensal *
item_relatorio_mensal_new()
{
    return g_object_new(ITEM_RELATORIO_MENSAL_TYPE, NULL);
}

ItemRelatorioMensal *
item_relatorio_mensal_new_from_carga(const Carga *carga)
{
    ItemRelatorioMensal *self = item_relatorio_mensal_new();

    item_relatorio_mensal_fit(self, carga);

    return self;
}
