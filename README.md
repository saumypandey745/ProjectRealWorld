<div align="center">

# 🌆 Project RealWorld

**AAA Open-World Urban Simulation Framework**  
*Built in Unreal Engine 5.4+ · C++ · Cross-Platform PC & Android*

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.4%2B-black?style=for-the-badge&logo=unrealengine&logoColor=white)](https://www.unrealengine.com)
[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue?style=for-the-badge&logo=cplusplus)](https://en.cppreference.com/w/cpp/17)
[![Platform](https://img.shields.io/badge/Platform-PC%20%7C%20Android-green?style=for-the-badge&logo=android)](https://developer.android.com)
[![License](https://img.shields.io/badge/License-Proprietary-red?style=for-the-badge)](LICENSE)
[![Build](https://img.shields.io/badge/Build-Development%20Editor-orange?style=for-the-badge)]()

</div>

---

## 📖 Overview

**Project RealWorld** is a modular, systems-driven AAA open-world architecture built entirely in Unreal Engine 5.4+ using C++. It simulates a living, breathing city — with intelligent NPCs, realistic vehicle physics, a dynamic crime and police response system, mission-driven economy, and a fully cross-platform rendering pipeline that runs from budget Android phones to RTX 4090 PCs.

This is not a prototype. Every system is production-grade — designed to be extended into a full GTA-style open-world title.

---

## ✨ Feature Highlights

| System | Description |
|--------|-------------|
| 🚗 **Chaos Vehicle Physics** | Full drivetrain simulation — gears, fuel, collision, 7-tier damage |
| 🚦 **City Traffic AI** | Multi-lane roads, dual-phase traffic lights, police yielding, LOD system |
| 🧠 **Smart NPC Crowd** | 8 archetypes, daily schedules, weather & gunfire reactions |
| 🔍 **Crime & Witnessing** | Line-of-sight checks, 911 delay, evidence decay, severity scaling |
| ⭐ **Police Wanted System** | 0–5 star wanted levels, pursuit AI, search zones, cooldown |
| 📦 **Mission: Late Delivery** | 10-stage vertical slice — cargo, evasion, harbor docking, reward |
| 💰 **Authoritative Economy** | Cash, bank, ATM, transaction ledger, pre-purchase validation |
| 🌦️ **Dynamic World Events** | Accidents, fights, storms — time/weather-weighted probability |
| 💾 **Save / Load v2** | Backwards-compatible schema, NaN corruption checks, full state persistence |
| 📱 **Cross-Platform Rendering** | Auto-detects hardware — scales from Mobile Low (30fps) to PC Ultra (Nanite/Lumen) |

---

## 🏗️ Architecture Overview

```
ProjectRealWorld/
├── Source/ProjectRealWorld/
│   ├── AI/                     # NPC intelligence & crowd management
│   │   ├── RealWorldNPCAIController      (Perception: sight + hearing)
│   │   ├── RealWorldNPCCharacter         (8 archetypes, FACS animation)
│   │   ├── RealWorldCrowdManager         (Budget-aware crowd spawning)
│   │   ├── RealWorldCrowdTypes.h         (Enums: archetypes, behaviors)
│   │   └── RealWorldScheduleTypes.h      (Daily schedule data types)
│   │
│   ├── Character/              # Player character & animation
│   │   ├── RealWorldCharacter            (Movement, stamina, vault)
│   │   ├── RealWorldAnimInstance         (State machine, blendspace)
│   │   └── RealWorldCharacterCustomization.h
│   │
│   ├── Combat/                 # Combat & weapon systems
│   │   ├── RealWorldCombatComponent
│   │   └── RealWorldWeaponBase
│   │
│   ├── Core/                   # Game framework classes
│   │   ├── RealWorldGameMode             (Cash init, spawn policy)
│   │   ├── RealWorldPlayerController     (Enhanced Input, vehicle swap)
│   │   ├── RealWorldPlayerState          (Economy authority)
│   │   ├── RealWorldPlayerStart          (Spawn validation)
│   │   └── RealWorldSaveGame             (v2 persistence schema)
│   │
│   ├── Environment/            # Atmosphere & weather visuals
│   │   └── RealWorldAtmosphereController
│   │
│   ├── Input/                  # Enhanced Input configuration
│   │   └── RealWorldInputConfig
│   │
│   ├── Interaction/            # World interaction system
│   │   ├── InteractableInterface         (UInterface contract)
│   │   ├── RealWorldInteractionComponent (Raycast, distance check)
│   │   ├── RealWorldDoor                 (Animated door logic)
│   │   ├── RealWorldPickup               (Item pickup + inventory)
│   │   └── RealWorldStoreItem            (Price display, restocking)
│   │
│   ├── Materials/              # Runtime material system
│   │   └── RealWorldMaterialManager      (Weather wet, skin, LOD)
│   │
│   ├── Platform/               # Cross-platform hardware abstraction
│   │   ├── RealWorldPlatformSubsystem    (Auto hardware detection)
│   │   ├── RealWorldSettingsSubsystem    (User settings persistence)
│   │   └── RealWorldScalabilityTypes.h   (Preset & tier enums)
│   │
│   ├── Subsystems/             # World-level game systems
│   │   ├── CrimeDetectionSubsystem       (LoS witnessing, 911 delay)
│   │   ├── PoliceWantedSubsystem         (0–5 star system)
│   │   ├── MissionSubsystem              (Mission state machine)
│   │   ├── Mission_LateDelivery          (10-stage vertical slice)
│   │   ├── DynamicWorldEventSubsystem    (Ambient events)
│   │   ├── WeatherSubsystem              (Rain, fog, storm)
│   │   ├── TimeOfDaySubsystem            (Day/Night cycle)
│   │   └── RealWorldAudioSubsystem       (MetaSound integration)
│   │
│   ├── UI/                     # Heads-up display
│   │   └── RealWorldHUD                  (Vitals, vehicle cluster, wanted stars)
│   │
│   ├── Vehicles/               # Vehicle gameplay & traffic
│   │   ├── RealWorldVehicleBase          (ChaosVehicle, fuel, damage)
│   │   └── RealWorldTrafficManager       (Multi-lane AI, traffic lights)
│   │
│   └── World/                  # Procedural city generation
│       ├── RealWorldCityBlockBuilder     (Grid-based block spawner)
│       ├── RealWorldModularBuilding      (Swappable facade system)
│       ├── RealWorldPCGEnvironment       (PCG foliage & props)
│       ├── RealWorldSpawnerSubsystem     (Budget-aware world spawner)
│       └── RealWorldStreetProps          (Street furniture, signage)
│
├── Config/
│   ├── DefaultEngine.ini         # Nanite, Lumen, VSM, World Partition, EnhancedInput
│   ├── DefaultScalability.ini    # PC tiers: Low → Medium → High → Epic → Cinematic
│   ├── DefaultGame.ini           # Project metadata, GameMode config
│   ├── DefaultInput.ini          # Input system bindings
│   ├── Android/
│   │   ├── AndroidEngine.ini     # Mobile: Vulkan ES3.1, ARM64, API 34
│   │   └── AndroidScalability.ini # Mobile tiers: Low → Medium → High → Ultra
│   └── IOS/
│       ├── IOSEngine.ini         # Metal rendering, minimum iOS 15.0
│       └── IOSScalability.ini    # iOS quality profiles
│
├── ProjectRealWorld.uproject     # Engine: 5.4 | Plugins: 13 registered
├── Source/ProjectRealWorld.Target.cs       # Game target
└── Source/ProjectRealWorldEditor.Target.cs # Editor target
```

---

## 📐 Cross-Platform Rendering Architecture

```
┌─────────────────────────────────────────────────────────────┐
│               URealWorldPlatformSubsystem                   │
│              (Auto Hardware Detection on Boot)              │
└────────────────────────┬────────────────────────────────────┘
                         │
          ┌──────────────┴──────────────┐
          │                             │
    ┌─────▼─────┐               ┌───────▼──────┐
    │  PC / Editor│               │    Android   │
    │  (Linux/Win)│               │  (ARM64)     │
    └─────┬───── ┘               └───────┬───────┘
          │                              │
   ┌──────▼──────────┐         ┌─────────▼────────┐
   │ PC Scalability  │         │ Mobile Scalability│
   │ Low   → 0       │         │ Low   → 0         │
   │ Medium → 1      │         │ Medium → 1        │
   │ High  → 2       │         │ High  → 2         │
   │ Epic  → 3       │         │ Ultra → 3         │
   │ Cine  → Cine    │         └──────────────────-┘
   └─────────────────┘
          │
   ┌──────▼──────────────────────────────┐
   │ PC Ultra: Nanite + Lumen + VSM + TSR │
   │ PC High:  Nanite + Lumen + VSM + TSR │
   │ PC Med:   No Nanite, Lumen SW, CSM   │
   │ PC Low:   FXAA, No Lumen, CSM 1x     │
   └──────────────────────────────────────┘
```

---

## 🔌 Plugins Used

| Plugin | Purpose |
|--------|---------|
| `EnhancedInput` | Input mapping contexts, gamepad & touch |
| `ChaosVehiclesPlugin` | Physics-driven vehicle simulation |
| `Metasound` | Procedural audio & sound systems |
| `PCG` | Procedural city environment generation |
| `MotionWarping` | Root motion warping for vaulting |
| `ControlRig` | Character procedural animation |
| `RigVM` | ControlRig virtual machine backend |
| `HairStrands` | Groom hair rendering |
| `Niagara` | VFX particle systems |
| `StateTree` | NPC behavior trees |
| `GameplayStateTree` | Gameplay-integrated StateTree |
| `LiveLink` | Real-time animation data streaming |
| `ModelingToolsEditorMode` | Editor-only mesh modeling tools |

---

## 🖥️ System Requirements

### PC (Minimum)
| Component | Minimum | Recommended |
|-----------|---------|-------------|
| OS | Windows 10 64-bit / Ubuntu 22.04 | Windows 11 / Ubuntu 22.04 |
| CPU | 6-core @ 3.0GHz | Intel i7-12700K / Ryzen 7 7700X |
| RAM | 16 GB | 32 GB |
| GPU | GTX 1070 / RX 5700 (SM5, Vulkan) | RTX 4080 / RX 7900 XTX |
| Storage | 100 GB SSD | 100 GB NVMe SSD |
| DirectX | DirectX 12 | DirectX 12 |

### Android (Minimum)
| Component | Minimum |
|-----------|---------|
| Android Version | 8.0 (Oreo) / API 26 |
| Architecture | ARM64-v8a |
| GPU | Vulkan 1.1+ or OpenGL ES 3.1 |
| RAM | 3 GB |
| Storage | 4 GB free |

---

## 🛠️ Build & Setup

### Prerequisites

- [Unreal Engine 5.4+](https://www.unrealengine.com/download)
- Visual Studio 2022 / JetBrains Rider (Windows) or Clang 16/17 (Linux)
- .NET 8.0 SDK (required by UnrealBuildTool)

### PC Build

```bash
# 1. Clone the repository
git clone https://github.com/saumypandey745/ProjectRealWorld.git
cd ProjectRealWorld

# 2. Generate project files (Linux)
/path/to/UnrealEngine/Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh \
    -project="$(pwd)/ProjectRealWorld.uproject" -game -engine

# 3. Compile Development Editor
/path/to/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \
    ProjectRealWorldEditor Linux Development \
    -project="$(pwd)/ProjectRealWorld.uproject"

# 4. Launch Editor
/path/to/UnrealEngine/Engine/Binaries/Linux/UnrealEditor \
    "$(pwd)/ProjectRealWorld.uproject"
```

**Windows:**
1. Right-click `ProjectRealWorld.uproject` → **Generate Visual Studio project files**
2. Open `ProjectRealWorld.sln` in Visual Studio 2022
3. Build configuration: `Development Editor | Win64`
4. Build target: `ProjectRealWorldEditor`
5. Launch: double-click `ProjectRealWorld.uproject`

### Android Build

```bash
# Requires: Android Studio, SDK (API 34), NDK r25c, OpenJDK 17
# Set env vars:
export ANDROID_HOME=/path/to/android-sdk
export ANDROID_NDK_ROOT=/path/to/android-ndk-r25c
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

# Package via RunUAT
/path/to/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun \
    -project="$(pwd)/ProjectRealWorld.uproject" \
    -platform=Android \
    -cookflavor=ETC2 \
    -configuration=Development \
    -build -cook -package -stage -deploy \
    -targetplatform=Android_ARM64

# Install & run on connected device
adb install -r ProjectRealWorld.apk
```

### Play In Editor

After the Editor launches:
1. Open map: `Content/Maps/L_WorldPartition_City`
2. Press **Alt + P** (Play In Editor)
3. Test player controls, NPC interactions, vehicles, crime system

---

## 🎮 Key Gameplay Controls (Default — PC)

| Action | Key / Mouse |
|--------|------------|
| Move | WASD |
| Sprint | Shift |
| Crouch | C |
| Jump | Space |
| Aim | Right Mouse Button |
| Interact | E |
| Enter / Exit Vehicle | F |
| Vehicle Throttle | W |
| Vehicle Brake | S |
| Vehicle Handbrake | Space (in vehicle) |
| Camera | Mouse |

---

## 🗺️ Default Map

```
GameDefaultMap = /Game/Maps/L_WorldPartition_City
```

World Partition enabled with 12,800 unit grid cells, 25,600 unit streaming range, and HLOD at 51,200 unit range.

---

## 📊 Project Statistics

| Metric | Count |
|--------|-------|
| C++ Header Files | 45 |
| C++ Source Files | 39 |
| Config Files | 12 |
| Registered Plugins | 13 |
| Gameplay Subsystems | 8 |
| NPC Archetypes | 8 |
| Vehicle Damage States | 7 |
| Mission Stages | 10 |
| Scalability Presets (PC) | 5 |
| Scalability Presets (Mobile) | 4 |

---

## 🔗 Links

- **Repository**: [github.com/saumypandey745/ProjectRealWorld](https://github.com/saumypandey745/ProjectRealWorld)
- **Engine**: [Unreal Engine 5.4 Release Notes](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5.4-release-notes)

---

## 📜 License

```
Copyright © 2026 Saumy Pandey — Antigravity Game Studios
All rights reserved. Private proprietary project.
Unauthorized copying, distribution, or modification is strictly prohibited.
```

---

<div align="center">
<b>Built with ❤️ using Unreal Engine 5.4 · C++ · by Saumy Pandey</b>
</div>
