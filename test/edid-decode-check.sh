#!/bin/sh -eu

workdir="$(mktemp -d)"
cleanup() {
	rm -rf "$workdir"
}
trap cleanup EXIT

edid="$1"
"$REF_EDID_DECODE" -s <"$edid" >"$workdir/ref"
"$DI_EDID_DECODE" <"$edid" >"$workdir/di"

if [ -f "$edid.diff" ]; then
	patch "$workdir/ref" "$edid.diff"
fi

diff -u "$workdir/ref" "$workdir/di"
