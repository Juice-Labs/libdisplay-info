# libdisplay-info

EDID and DisplayID library.

Goals:

- Provide a set of high-level, easy-to-use, opinionated functions as well as
  low-level functions to access detailed information.
- Simplicity and correctness over performance and resource usage.
- Well-tested and fuzzed.

## Contributing

Open issues and merge requests on the [GitLab project].

In general, the [Wayland contribution guidelines] should be followed. In
particular, each commit must carry a Signed-off-by tag to denote that the
submitter adheres to the [Developer Certificate of Origin 1.1]. This project
follows the [freedesktop.org Contributor Covenant].

## Building

libdisplay-info is built using [Meson]. It has no dependencies.

    meson build/
    ninja -C build/

## Testing

The low-level EDID library is tested against [edid-decode]. `test/data/`
contains a small collection of EDID blobs and diffs between upstream
`edid-decode` and our `di-edid-decode` clone. Our CI ensures the diffs are
up-to-date. A patch should never make the diffs grow larger. To add a new EDID
blob or update a diff, use `test/edid-decode-diff.sh test/data/<edid>`.

## Fuzzing

To fuzz libdisplay-info with [AFL], the library needs to be instrumented:

    CC=afl-gcc meson build/
    ninja -C build/
    afl-fuzz -i test/data/ -o afl/ build/di-edid-decode

[GitLab project]: https://gitlab.freedesktop.org/emersion/libdisplay-info
[Wayland contribution guidelines]: https://gitlab.freedesktop.org/wayland/wayland/-/blob/main/CONTRIBUTING.md
[Developer Certificate of Origin 1.1]: https://developercertificate.org/
[freedesktop.org Contributor Covenant]: https://www.freedesktop.org/wiki/CodeOfConduct/
[Meson]: https://mesonbuild.com/
[edid-decode]: https://git.linuxtv.org/edid-decode.git/
[AFL]: https://lcamtuf.coredump.cx/afl/
