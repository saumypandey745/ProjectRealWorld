# Project Risk Register

| # | Risk Description | Severity | Probability | Impact | Mitigation Strategy | Owner | Status |
|---|---|---|---|---|---|---|---|
| 1 | Scope explosion (solo dev) | High | High | Project abandonment | Enforce S/M/L Tiers, strict cut list, phase gates before advancing | TD / Lead | Active |
| 2 | Mobile thermal throttling | Critical | High | Frame drops below 20 FPS | Hard 30 FPS target, Adaptive Performance governor, tier-based quality scaling | Tech Lead | Active |
| 3 | Realistic characters too heavy for mobile | High | Medium | High draw calls & memory exhaustion | Strict budget (Hero ≤30k tris, NPC ≤6k), LODs, VAT for crowd, shared atlases | Tech Artist | Active |
| 4 | World streaming hitches | High | Medium | Stutter >100ms when driving fast | Time-sliced instantiation (≤2ms/frame), predictive prefetch, ready gate | Engine Dev | Active |
| 5 | Memory leaks / OOM kills (iOS jetsam, Android lmkd) | Critical | Medium | App crash | Object pooling, Addressables discipline, Memory Profiler CI baselines | Core Dev | Active |
| 6 | Shader/PSO first-use hitches | High | High | Visible frame freeze on first explosion/weapon fire | Shader prewarming, strict variant stripping, PSO cache warm at load | Graphics Dev | Active |
| 7 | Art volume & quality bottleneck | High | High | Inconsistent visual quality / stalled roadmap | Licensed asset packs, modular kits, automated importer/validator | Tech Artist | Active |
| 8 | IP & legal exposure (GTA similarity, real brands) | Critical | Low | Store ban / legal action | 100% original IP, fictional brands, clean audio licensing log | Legal / PM | Active |
| 9 | App store rejection (Content, privacy, 16KB pages) | Critical | Medium | Delayed launch | Early policy compliance check, Play Data safety & Apple privacy labels | DevOps | Active |
| 10 | Vulkan / driver fragmentation on Android | High | High | Visual corruption / crashes on budget chipsets | Device testing matrix, GLES3 fallback, automatic driver blocklist | Platform Dev | Active |
| 11 | Backend security & cloud save abuse | High | Medium | Data loss / server breach | OWASP API checks, Argon2id, JWT rotation, atomic save writes | Backend Dev | Active |
| 12 | Save corruption & data loss | Critical | Low | Player progress lost | Atomic file writes (temp -> flush -> rename), 2 rotating backups | Core Dev | Active |
| 13 | Package / Engine upgrade breaking changes | Medium | Medium | Regressions in physics / rendering | Pinned package versions, strict lockfiles, scheduled upgrade windows | TD | Active |
| 14 | Vehicle feel on touch controls | High | Medium | Poor player engagement | Early raycast prototype, customizable assists, virtual steering options | Gameplay Dev | Active |
| 15 | Crowd rendering scalability | High | Medium | CPU bottleneck at >50 NPCs | Phase 2 R&D spike (VAT vs pooled simplified rigs), ECS lite sim | AI / Graphics | Active |
| 16 | Dev machine limitations (Linux i5 / integrated GPU) | High | High | Slow local iteration | Cloud CI builds (GameCI), lightweight greybox testing, profiling on phone | TD | Active |
