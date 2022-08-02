#!/bin/sh -eu

workdir="$(mktemp -d)"
cleanup() {
	rm -rf "$workdir"
}
trap cleanup EXIT

edid="$1"
"$REF_EDID_DECODE" --skip-hex-dump --check --skip-sha <"$edid" >"$workdir/ref" || [ $? = 254 ]
"$DI_EDID_DECODE" <"$edid" >"$workdir/di" || [ $? = 254 ]

if [ -f "$edid.diff" ]; then
	patch "$workdir/ref" "$edid.diff"
fi

diff -u "$workdir/ref" "$workdir/di"
