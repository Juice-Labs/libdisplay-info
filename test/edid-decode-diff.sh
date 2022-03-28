#!/bin/sh -eu

REF_EDID_DECODE="${REF_EDID_DECODE:-edid-decode}"
DI_EDID_DECODE="${DI_EDID_DECODE:-di-edid-decode}"

workdir="$(mktemp -d)"
cleanup() {
	rm -rf "$workdir"
}
trap cleanup EXIT

for edid in "$@"; do
	"$REF_EDID_DECODE" -s <"$edid" >"$workdir/ref"
	"$DI_EDID_DECODE" <"$edid" >"$workdir/di"
	if ! diff -u --label ref "$workdir/ref" --label di "$workdir/di" >"$workdir/diff"; then
		cp "$workdir/diff" "$edid.diff"
	else
		rm "$edid.diff"
	fi
done
