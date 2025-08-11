# Animagic Profiles

## What is a Profile?
A JSON preset for quick application of assembly settings.

## Example
```json
{
  "name": "Discord GIF",
  "target_format": "gif",
  "loop": 0,
  "delay_ms": 80,
  "gif": { "optimize": true, "dither": true, "colors": 128 },
  "webp": {}
}
```

## Storage
Profiles are stored in:
- Linux: `~/.config/animagic/profiles/`
Each profile = `profile-name.json`
