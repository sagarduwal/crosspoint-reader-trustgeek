# Hello sample app (Phase 0)

Copy this folder to the SD card to smoke-test the Lua runtime before the Applications UI exists:

```text
/.crosspoint/apps/hello/main.lua      ← copy from apps/samples/hello/main.lua
/.crosspoint/apps/hello/manifest.json ← optional for Phase 0 manual runs
```

Register the app in `/.crosspoint/apps/registry.json`:

```json
{
  "version": 1,
  "apps": [
    {
      "id": "hello",
      "name": "Hello App",
      "version": "1.0.0",
      "installed_at": "2026-06-28T00:00:00Z"
    }
  ]
}
```

Build firmware with the App Store environment:

```bash
pio run -e app_store
```

Expected serial output when the app is launched:

```text
INF LUA Hello app rendered 15 display lines
```

The e-ink screen shows ASCII art, greeting text, and device info. Press **Back** to return to Installed.

`cp.display` API used here: `clear()`, `center(y, text)`, `text(x, y, text)`.
