# Performance Log

Har phase ya major optimization pass ke baad on-device metrics capture karke yahan record honge.

## Benchmark Test Scenes
1. **Downtown Rush Hour:** 40 NPCs, 10 vehicles, high building density, shadow cascades active.
2. **Highway Sprint:** Fast camera motion, dynamic chunk streaming at 120 km/h, road LOD swapping.
3. **Gunfight + Police Chase:** 4 police pursuit cars, 10 armed NPCs, active VFX particles, bullet decals, dynamic audio voices.

## Log Entries
| Date | Phase | Test Scene | Device | OS/Driver | Tier | Avg FPS | p99 Frame (ms) | Draw Calls | SetPass | Managed Heap | Native Memory | Thermal Trend | Status |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Baseline | P0 | N/A | Target Reference (Snapdragon 7-series) | Android 13 Vulkan | Med | 30 | ≤ 40ms | ≤ 500 | ≤ 150 | ≤ 150 MB | ≤ 1.45 GB | Stable 30 min | Pending Phase 1 |
