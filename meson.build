project(
	'libdisplay-info',
	'c',
	version: '0.0.0',
	license: 'MIT',
	meson_version: '>= 0.52.1',
	default_options: [
		'c_std=c11',
		'warning_level=3',
	],
)

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
	'-Wundef',
	'-Wmissing-prototypes',
	'-Walloca',
	'-Wdeclaration-after-statement',
	'-Wconversion',

	'-Wno-unused-parameter',
	'-Wno-missing-field-initializers',
]), language: 'c')

di_lib = library(
	'display-info',
	[
		'edid.c',
		'info.c',
	],
	include_directories: include_directories('include'),
	install: true,
)

install_subdir(
	'include/libdisplay-info',
	install_dir: get_option('includedir'),
)

pkgconfig = import('pkgconfig')
pkgconfig.generate(
	di_lib,
	description: 'EDID and DisplayID library',
)

di_dep = declare_dependency(
	link_with: di_lib,
	include_directories: include_directories('include'),
)

executable(
	'di-edid-decode',
	'di-edid-decode.c',
	dependencies: di_dep,
	install: true,
)
