# Architectural Decision Records (ADRs)

Log format:
`ADR-<Number>: <Title>`
- **Status:** [Proposed | Accepted | Rejected | Superseded]
- **Context:** Problem statement and constraints.
- **Options Considered:** Option A vs Option B vs Option C.
- **Trade-offs:** Pros and Cons of each.
- **Decision:** Selected option and technical justification.
- **Consequences:** Impact on architecture, performance, dev-time, and verification plan.

---

## ADR-001: Engine & Render Pipeline Selection
- **Status:** Accepted
- **Context:** Project Metro is a living open-world mobile title targeting mid-range Android (6GB RAM, Snapdragon 7-series) and iPhone 12+ with strict 30 FPS thermal stability.
- **Options Considered:**
  1. Unity LTS with Universal Render Pipeline (URP) Forward Renderer
  2. Unity LTS with URP Forward+
  3. Unreal Engine 5.4+ (existing repo base) with Mobile Forward
- **Trade-offs:**
  - UE5 is extremely heavy for solo dev iteration on mobile, has large binary footprints, and dev workstation lacks dedicated GPU to run UE5 Editor smoothly.
  - Unity URP Forward is battle-tested on mobile, has low baseline overhead, robust SRP Batcher support, and low SetPass calls on mobile tile-based GPUs.
  - Forward+ increases memory for light clustering; only beneficial if scene has >8 active real-time lights per chunk.
- **Decision:** Unity 6 LTS / 2022.3 LTS with URP Forward Renderer.
- **Consequences:** Low memory footprint, fast iteration, high compatibility across Vulkan and Metal.

---

## ADR-002: Graphics API & Android Fallback Policy
- **Status:** Accepted
- **Context:** Android ecosystem has driver fragmentation across Adreno, Mali, and PowerVR.
- **Options Considered:**
  1. Vulkan Only
  2. Vulkan primary with OpenGL ES 3.2 fallback
  3. OpenGL ES 3.2 only
- **Trade-offs:**
  - Vulkan gives lower CPU overhead and multi-threaded command buffers, but old/budget devices have driver bugs.
  - GLES3-only wastes CPU performance on modern chipsets.
- **Decision:** Vulkan primary, GLES3 fallback with an automated driver blocklist for known buggy chipsets.
- **Consequences:** Maximum performance on modern devices while preserving device coverage.

---

## ADR-003: UI Technology (Runtime HUD vs Editor Tooling)
- **Status:** Accepted
- **Context:** Mobile UI requires high responsiveness, minimal per-frame GC allocations, dynamic canvas scaling, and customizable touch controls.
- **Options Considered:**
  1. uGUI + TextMeshPro for both runtime and editor
  2. UI Toolkit for both runtime and editor
  3. Hybrid: uGUI + TMP for runtime HUD/Menus; UI Toolkit for custom Editor Tools
- **Trade-offs:**
  - UI Toolkit for runtime on mobile still has edge cases with complex touch interactions, safe-area adjustments, and third-party asset integrations.
  - uGUI is extremely stable, predictable on mobile, and allows canvas segregation to avoid dirty layout redraws. UI Toolkit excels in editor windows (GraphView for missions).
- **Decision:** Hybrid approach — uGUI + TextMeshPro for all runtime gameplay HUD and menus; UI Toolkit for Editor tooling (Mission Editor, Chunk Baker).
- **Consequences:** Zero runtime risk, high editor productivity.

---

## ADR-004: Vehicle Physics Model
- **Status:** Accepted
- **Context:** Arcade-realistic driving feel on touch controls without WheelCollider instability or physics jitter.
- **Options Considered:**
  1. Unity WheelCollider
  2. Custom Raycast Suspension & Tire Friction Model
  3. Pure Kinematic spline movement
- **Trade-offs:**
  - WheelCollider is prone to edge catching, unpredictable spring oscillation at low frame rates, and high CPU cost for multiple vehicles.
  - Custom raycast model allows full mathematical control over slip curves, weight transfer, drift handbrake feel, and costs <0.1ms per car.
- **Decision:** Custom 4-point raycast vehicle physics model with Pacejka-lite slip curves and scriptable tuning profiles.
- **Consequences:** Consistent, punchy handling tailored for mobile touch controls and deterministic physics budgets.

---

## ADR-005: Chunk Grid Size for World Streaming
- **Status:** Accepted
- **Context:** World streaming grid size determines Addressable asset granularity, memory spikes on load, and draw call batching.
- **Options Considered:**
  1. 128m × 128m cells
  2. 256m × 256m cells
- **Trade-offs:**
  - 256m chunks create larger asset bundles, causing larger instantiation spikes (>8ms) and memory over-allocation.
  - 128m chunks provide tight streaming granularity, keeping instantiation time-slice cost ≤2ms per frame and allowing fast memory recycling.
- **Decision:** 128m × 128m chunk grid with concentric simulation rings (R0 to R3).
- **Consequences:** Strict budget compliance, zero hitching, predictable memory footprint.
