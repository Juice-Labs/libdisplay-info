ref_edid_decode = find_program('edid-decode', native: true, required: false)
if not ref_edid_decode.found()
	test('edid-decode-not-found', find_program('false'))
	subdir_done()
endif

test_harness = find_program('./edid-decode-check.sh', native: true)

test_cases = [
	'dell-2408wfp-dp',
	'hp-5dq99aa-hdmi',
	'panasonic-mei96a2-dp',
	'samsung-s27a950d-dp',
	'sun-gh19ps-dvi',
]

foreach tc : test_cases
	test(
		tc,
		test_harness,
		args: [files('data/' + tc + '.edid')],
		env: [
			'REF_EDID_DECODE=' + ref_edid_decode.full_path(),
			'DI_EDID_DECODE=' + di_edid_decode.full_path(),
		],
		depends: [di_edid_decode],
	)
endforeach
