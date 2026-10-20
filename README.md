# Project RealWorld 🌆
### AAA Open-World Architecture & Gameplay Systems in Unreal Engine 5.4+

Project RealWorld is an open-world urban simulation framework engineered with modular C++ architecture, Chaos Vehicles, non-omniscient crime & police response, smart crowd schedules, dynamic environmental systems, and an authoritative economy.

---

## 🚀 Key Systems & Architecture

- **Vehicle Gameplay (`ChaosVehicles`)**: Physics-driven acceleration, braking, steering, transmission gearing, fuel depletion, collision dynamics, and 7-tier damage states.
- **City Traffic Simulation (`ARealWorldTrafficManager`)**: Multi-lane road networks, dual-phase traffic light cycles (N/S & E/W), stop lines, emergency vehicle yielding, and 4-tier distance LOD.
- **Non-Omniscient Crime & Witnessing (`UCrimeDetectionSubsystem`)**: Line-of-sight and noise perception checks, civilian 911 phone reporting delays, crime severity levels, and physical evidence decay.
- **Police & Wanted Heat Subsystem (`UPoliceWantedSubsystem`)**: 0–5 star wanted levels with dynamic dispatch response, search radius expansion, lost-target evasion countdown, and arrest interactions.
- **Vertical Slice Mission: "Late Delivery" (`UMissionSubsystem`)**: Complete 10-stage courier delivery loop with vehicle transport, container retrieval, street obstruction evasion, harbor docking, cash rewards, and state persistence.
- **Authoritative Economy (`ARealWorldPlayerState`)**: Cash and bank accounts, transaction ledger tracking, pre-purchase validation, and ATM deposit/withdrawal support.
- **Storefront Shopping (`ARealWorldStoreItem`)**: Interactive convenience store items with real-time price displays, clerk alertness, and restocking timers.
- **Dynamic World Encounters (`UDynamicWorldEventSubsystem`)**: Spontaneous ambient street events (accidents, fights, breakdowns, storms) with time-of-day and weather-weighted probability.
- **MetaHuman & Crowd Reactivity (`ARealWorldNPCCharacter`)**: 8 NPC archetypes, daily routine schedules, facial FACS curve setups, and reactions to sprinting, collisions, gunfire, and weather.
- **Save / Load Persistence (`URealWorldSaveGame`)**: Version 2 backwards-compatible schema with NaN corruption checks, player state, vehicle states, mission progress, and time/weather persistence.
- **Minimal HUD (`ARealWorldHUD`)**: On-foot vitals (health, stamina, cash, clock), vehicle telemetry cluster (speedometer, gear, fuel, condition), wanted stars/pursuit status, and mission waypoint distance.

---

## 🛠️ How to Build and Run in Unreal Engine 5.4+

1. **Clone the repository**:
   ```bash
   git clone https://github.com/saumypandey745/ProjectRealWorld.git
   cd ProjectRealWorld
   ```

2. **Generate Project Files**:
   - Right-click `ProjectRealWorld.uproject` and select **"Generate Visual Studio project files"** (or use `UnrealBuildTool` on Linux).

3. **Compile the Editor Target**:
   - Open `ProjectRealWorld.sln` in Visual Studio 2022 (or open the project folder in JetBrains Rider / VS Code).
   - Set build configuration to **Development Editor** (`Win64` or `Linux`).
   - Build target: `ProjectRealWorldEditor`.

4. **Launch the Project**:
   - Double-click `ProjectRealWorld.uproject` or launch from the IDE.
   - Load map: `L_WorldPartition_City`.
   - Press **Play In Editor (Alt + P)** to start gameplay!

---

## 📜 License
Private proprietary project by Saumy Pandey. All rights reserved.
