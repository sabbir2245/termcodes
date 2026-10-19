#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <file.cpp> [args...]" >&2
    exit 1
fi

src=$1
shift

if [[ ! -f "$src" ]]; then
    echo "Error: file not found: $src" >&2
    exit 1
fi

out=$(mktemp "${TMPDIR:-/tmp}/$(basename "${src%.*}").XXXXXX")
trap 'rm -f "$out"' EXIT

g++ -std=c++17 -Wall -Wextra -pthread "$src" -o "$out"
"$out" "$@"
