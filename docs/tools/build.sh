#!/usr/bin/env bash
# Build PiTrex baremetal targets inside the "pitrex-dev" distrobox, in a scratch copy of
# the tree so tracked build artifacts in the repo are not touched.
#
# Usage: docs/tools/build.sh <zero1|zero2> <subdir> [more subdirs...]
#   e.g. docs/tools/build.sh zero1 pitrex/loader aae
# Output stays in $SCRATCH/<subdir>; see the printed paths.
#
# One-time setup (Ubuntu 22.04 gives arm-none-eabi-gcc 10.3.1):
#   distrobox create --yes --name pitrex-dev --image docker.io/library/ubuntu:22.04
#   distrobox enter pitrex-dev -- sudo apt-get install -y gcc-arm-none-eabi \
#       libnewlib-arm-none-eabi binutils-arm-none-eabi build-essential
set -euo pipefail
target=${1:?zero1 or zero2}; shift
[ $# -ge 1 ] || { echo "give at least one subdir" >&2; exit 2; }
repo=${REPO_SRC:-$(cd "$(dirname "$0")/../.." && pwd)}   # REPO_SRC: build another source tree (e.g. git archive of an old commit)
scratch=${SCRATCH:-$HOME/.cache/pitrex-build}
mkdir -p "$scratch"
rsync -a --exclude=.git --exclude=.idea "$repo"/ "$scratch"/
for d in "$@"; do
  # MAKE_IMAGE= and SERIAL_TRANSFER= override the author's Rules.mk copy/serial steps.
  podman exec -w "$scratch/$d" pitrex-dev \
    make all BUILD_FOR="$target" MAKE_IMAGE= SERIAL_TRANSFER=
  echo "built in $scratch/$d:"; ls -l "$scratch/$d"/*.img "$scratch/$d"/*.pit 2>/dev/null || true
done
