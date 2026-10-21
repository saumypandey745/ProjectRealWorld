# Asset Specification Guidelines v0

## General Rules
- **Units:** 1 Unity Unit = 1 Meter.
- **Pivot Point:** Centered at feet for characters; centered at base/axle plane for vehicles; bottom-center for props/buildings.
- **Orientation:** Forward = +Z, Right = +X, Up = +Y.
- **Coordinate Space:** Right-handed converted cleanly to Unity Left-handed on import.
- **Textures:** ASTC compressed (6x6 default, 4x4 for normals/faces/UI). Power of two. Mipmaps enabled with Mip Streaming.

## Character Specs
| Class | Prefix | LOD0 Tris | LOD1 Tris | Max Bones | Textures | Max Materials |
|---|---|---|---|---|---|---|
| Hero (Protagonist) | `CHR_Hero_` | ≤ 30,000 | ≤ 12,000 | 90 (incl. face) | 2K Body, 2K Face (Base, Normal, Packed ORM) | 3 |
| Key NPC (Story/Mission) | `CHR_Key_` | ≤ 15,000 | ≤ 7,000 | 60 | 1K Body, 2K Face | 2 |
| Generic NPC (Modular) | `CHR_Gen_` | ≤ 6,000 | ≤ 2,500 | 45 | 1K Shared Atlas | 1-2 |
| Crowd (Far background) | `CHR_Crowd_` | ≤ 1,500 | VAT mesh | ≤ 30 | 512 Shared Atlas | 1 |

## Vehicle Specs
| Class | Prefix | LOD0 Tris | LOD1 Tris | Textures | Max Materials |
|---|---|---|---|---|---|
| Player Vehicle | `VEH_Ply_` | ≤ 25,000 | ≤ 10,000 | 2K Exterior, 1K Interior | 3 |
| Traffic Car | `VEH_Trf_` | ≤ 8,000 | ≤ 3,000 | 1K Shared Atlas | 1 |

## Environment Specs
- Modular building height: 3.5m per floor.
- Road width: 3.75m per lane + 2.0m sidewalk.
- Chunk dimensions: 128m × 128m bounds.
