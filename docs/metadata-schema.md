# Animagic Metadata Schema

## Example
```json
{
  "source": {
    "path": "/projects/intro/loop.webp",
    "format": "webp",
    "width": 512,
    "height": 512,
    "frames": 42,
    "loop": 0,
    "per_frame_delay_ms": [80, 80, 80],
    "disposal_modes": ["none", "none"]
  },
  "assembly_prefs": {
    "target_format": "gif",
    "loop": 0,
    "delay_ms": 80,
    "fps": null,
    "gif": { "optimize": true, "dither": true, "colors": 256 },
    "webp": { "lossless": false, "quality": 85, "method": 4, "near_lossless": 0, "alpha_quality": 100 }
  },
  "stills": {
    "folder": "/projects/intro/frames",
    "pattern": "frame_%04d.png",
    "format": "png"
  },
  "created_at": "2025-08-11T15:57:29+00:00",
  "version": 1
}
```

## Fields
- **source**: Original animation details.
- **assembly_prefs**: All reassembly settings.
- **stills**: Location & format of extracted frames.
- **created_at**: Timestamp (ISO 8601).
- **version**: Schema version (int).
