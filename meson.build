project('graos', 'c',
    version: '0.1',
    default_options: [
        'c_std=c17',
    ]
)

includes = include_directories('include')

gnome = import('gnome')
gtk_dep = dependency('gtk4')

resources = gnome.compile_resources(
    'resources',
    'graos.gresource.xml',
    source_dir: 'ui',
)

c_sources = files(
    'src/app.c',
    'src/appwin.c',
    'src/carga.c',
    'src/database.c',
    'src/item_relatorio_geral.c',
    'src/item_relatorio_mensal.c',
    'src/main.c',
    'src/novo_carregamento.c',
    'src/relatorio_geral.c',
    'src/relatorio_mensal.c',
)

executable(
    meson.project_name(),
    c_sources,
    resources,
    include_directories: includes,
    dependencies: gtk_dep,
    win_subsystem: 'windows'
)
