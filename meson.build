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
