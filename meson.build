project(
	'libdisplay-info',
	'c',
	version: '0.0.0',
	license: 'MIT',
	meson_version: '>= 0.55.0',
	default_options: [
		'c_std=c11',
		'warning_level=3',
	],
)

cc = meson.get_compiler('c')

math = cc.find_library('m', required: false)

add_project_arguments(['-D_POSIX_C_SOURCE=200809L'], language: 'c')

add_project_arguments(cc.get_supported_arguments([
	'-Wundef',
	'-Wmissing-prototypes',
	'-Walloca',
	'-Wdeclaration-after-statement',
	'-Wconversion',

	'-Wno-unused-parameter',
	'-Wno-missing-field-initializers',

	'-Werror=implicit',
]), language: 'c')

symbols_file = 'libdisplay-info.map'
symbols_flag = '-Wl,--version-script,@0@'.format(meson.current_source_dir() / symbols_file)

di_lib = library(
	'display-info',
	[
		'cta.c',
		'dmt-table.c',
		'edid.c',
		'info.c',
		'log.c',
	],
	include_directories: include_directories('include'),
	dependencies: [math],
	link_args: symbols_flag,
	link_depends: symbols_file,
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

di_edid_decode = executable(
	'di-edid-decode',
	'di-edid-decode.c',
	dependencies: [di_dep, math],
	install: true,
)

subdir('test')
