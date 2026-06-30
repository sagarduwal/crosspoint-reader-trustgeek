#!/usr/bin/env bash
# Downloads Lua 5.4.7 sources into lib/LuaRuntime/vendor/lua-5.4.7
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VENDOR_DIR="$ROOT/lib/LuaRuntime/vendor/lua-5.4.7"
if [[ -d "$VENDOR_DIR/src" ]]; then
  echo "Lua vendor already present at $VENDOR_DIR"
  exit 0
fi
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
curl -fsSL https://www.lua.org/ftp/lua-5.4.7.tar.gz | tar -xz -C "$TMP"
mkdir -p "$ROOT/lib/LuaRuntime/vendor"
mv "$TMP/lua-5.4.7" "$VENDOR_DIR"
echo "Installed Lua 5.4.7 to $VENDOR_DIR"
