#!/bin/sh

# jemalloc ships no committed `configure` script and, unlike a typical Autotools
# project, only wants `autoconf` to run, never `autoheader`. The default
# `autoreconf -vfi` based flow of cmake-ports therefore fails, so we drive the
# generation ourselves through this entrypoint.
#
# cmake-ports invokes the configure entrypoint from the port's build (object)
# directory, which is the source directory with a "-build" suffix. We generate
# `configure` in the source tree and then run it out-of-tree from here.

set -e

srcdir="${PWD%-build}"

(cd "$srcdir" && autoconf)

exec "$srcdir/configure" "$@"
