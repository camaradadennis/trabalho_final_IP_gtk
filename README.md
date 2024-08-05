# Grãos

Trabalho final da disciplina de Introdução à Programação,
ministrado para alunos do primeiro período do curso de Bacharelado em Sistemas
de Informação na Universidade Federal de Goiás.

Os requisitos do trabalho eram:
- Criação de um programa para gerenciar um catálogo de grãos recebidos por uma agroindústria fictícia.
- Uma função para inserir um novo carregamento de grãos, que lesse os dados do carregamento de um arquivo de texto.
- Uma função para exibir estatísticas de carregamentos mês-a-mês.
- Uma função para exibir estatísticas globais do catálogo.
- Manutenção de um arquivo binário com os dados de carregamentos recebidos.

Todos os campos dos relatórios estatísticos, além dos dados que deveriam ser armazenados
por carregamento também foram especificados nos requisitos.

Trabalho realizado pelo grupo 8.

## Compilação e dependências

Este programa utiliza a [biblioteca GTK](https://gtk.org/), que deve estar
instalada para a compilação.

O sistema de compilação utilizado é o [Meson](https://mesonbuild.com/), de modo
que o mais fácil é, do diretóro raíz do projeto:

```
$ meson setup build
$ cd build && meson compile
```

Todo o desenvolvimento foi feito utilizando a toolchain GCC.

## Funcionamento

O aplicativo tira proveito do sistema de objetos fornecidos pela biblioteca.

Assim, a lógica do programa está contida nos objetos criados:
- Uma instância de GraosApp está na raíz do aplicativo e é responsável por gerenciar o thread model, a inicialização e encerramento.
- Uma instância de GraosAppWin mantém a estrutura da janela principal e promove a ligação entre as demais funções.
- Uma instância de GraosDb é responsável por manter a base de dados.
- Uma instância de NovoCarregamento lê e exibe o novo carregamento.
- Uma instância de RelatorioMensal, que mantém uma referência para GraosDb, constrói e exibe a lista de itens do relatório mensal.
- Uma instância de RelatorioGeral, que mantém uma referência para GraosDb, constrói e exibe lista de itens do relatório geral.

Cada item nos relatórios geral e mensal também é um objeto, cujas propriedades
são os campos exibidos nos respectivos relatórios. A lista destes itens é construída
e mantida pelas instâncias de relatório.

Do ponto de vista da organização do código-fonte, cada objeto possui um arquivo
.c e um header (.h). Além disso, os objetos que lidam com exibição na janela
(derivados de GtkWidget) tem sua hierarquia declarada em arquivos .ui (que são
XMLs).
