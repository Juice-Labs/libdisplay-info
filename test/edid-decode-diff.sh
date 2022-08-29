#!/bin/sh -eu

REF_EDID_DECODE="${REF_EDID_DECODE:-edid-decode}"
DI_EDID_DECODE="${DI_EDID_DECODE:-di-edid-decode}"

workdir="$(mktemp -d)"
cleanup() {
	rm -rf "$workdir"
}
trap cleanup EXIT

for edid in "$@"; do
	diff="${edid%.edid}.diff"
	"$REF_EDID_DECODE" --skip-hex-dump --check --skip-sha <"$edid" >"$workdir/ref" || [ $? = 254 ]
	"$DI_EDID_DECODE" <"$edid" >"$workdir/di" || [ $? = 254 ]
	if ! diff -u --label ref "$workdir/ref" --label di "$workdir/di" >"$workdir/diff"; then
		cp "$workdir/diff" "$diff"
	else
		rm -f "$diff"
	fi
done
