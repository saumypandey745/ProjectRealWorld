# PROJECT_STATE
Phase: 0 — Pre-production  |  Tier: S (Target)  |  Last updated: 2026-10-21

## Done
- Master Prompt Pack ingested and saved to `docs/MASTER_PROMPT.md`.
- Project memory system initialized (`PROJECT_STATE.md`, `DECISIONS.md`, `RISKS.md`, `ASSET_SPECS.md`, `ASSET_LICENSES.md`, `PERF_LOG.md`, `API.md`, `CHANGELOG.md`).
- Dev machine hardware environment inspected and documented in A0 profile.

## In progress
- Phase 0: Pre-production baseline, architecture decisions (ADRs), and initial repo setup.

## Next (max 5)
1. Complete ADRs for Engine version, URP settings, Graphics API fallback, UI system, Audio, and Physics model.
2. Initialize Unity Project folder structure and assembly definition files (`.asmdef`).
3. Set up CI/CD workflows for GameCI (GitHub Actions) for automated testing and builds.
4. Establish World & Story Bible v0 for Solace Bay.
5. Review Phase 0 Gate checklist and obtain approval before Phase 1.

## Known issues (ID | severity | area | note)
- ISS-001 | P2 | Workspace | Existing workspace contains Unreal Engine C++ project (`ProjectRealWorld`). Need confirmation on workspace layout (separate directory or root replacement for Unity).

## Perf snapshot (device | build | avg / p99 frame ms | memory | thermal)
- Reference Target: Mid-range Android (Snapdragon 7-series, 6GB RAM) | Target: 30 FPS (avg <= 28ms, p99 <= 40ms) | Memory <= 1.45 GB
- Dev Workstation: Ubuntu 22.04 LTS, Intel i5-3470, 11 GB RAM, Intel HD Graphics

## Open questions
1. Should the Unity Project live directly in a subfolder (e.g. `UnityProject/` or `Client/`) or replace the root directory of this repository?

## How to build / run / test
- Pending Phase 0 project bootstrap & CI setup.
