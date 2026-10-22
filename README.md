<div align="center">

# 🌆 Project RealWorld

**AAA Open-World Urban Mobile Game for Android**  
*Built in Unreal Engine 5.4+ · Native C++ · Android ARM64 & Vulkan*

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.4%2B-black?style=for-the-badge&logo=unrealengine&logoColor=white)](https://www.unrealengine.com)
[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue?style=for-the-badge&logo=cplusplus)](https://en.cppreference.com/w/cpp/17)
[![Platform](https://img.shields.io/badge/Platform-Android%20Mobile-green?style=for-the-badge&logo=android)](https://developer.android.com)
[![Architecture](https://img.shields.io/badge/Architecture-ARM64--v8a-orange?style=for-the-badge)]()
[![Graphics API](https://img.shields.io/badge/Vulkan-ES3.1%20%7C%20SM5-red?style=for-the-badge&logo=vulkan)](https://www.vulkan.org)
[![CI](https://github.com/saumypandey745/ProjectRealWorld/actions/workflows/ci.yml/badge.svg)](https://github.com/saumypandey745/ProjectRealWorld/actions/workflows/ci.yml)

### 📥 [Download Direct Game APK (v1.0.0)](https://github.com/saumypandey745/ProjectRealWorld/releases/download/live/ProjectRealWorld.apk)

</div>

---

## 📖 Overview

**Project RealWorld** is an Android-exclusive, AAA-grade open-world urban simulation game developed in Unreal Engine 5.4+ with native C++. Built from the ground up for modern Android smartphones (ARM64-v8a), it delivers a GTA-style living city experience with intelligent pedestrians, multi-lane vehicle traffic, police chase response, dynamic crime witnessing, economy, and on-foot / driving mechanics directly on mobile.

---

## ✨ Feature Highlights

| Mobile System | Description |
|---|---|
| 🚗 **Chaos Vehicle Physics** | Real-time vehicle dynamics — acceleration, gear shifting, fuel, suspension, collision damage |
| 🚦 **City Traffic AI** | Multi-lane intelligent navigation, dual-phase traffic signals, emergency vehicle yield |
| 🧠 **Living Crowd AI** | 8 pedestrian archetypes, daily life schedules, panic and fleeing reactions to gunfire |
| 🚔 **Crime & Police Wanted** | 0–5 star wanted levels, line-of-sight detection, pursuit cruisers, cooldown mechanics |
| 📱 **Mobile Touch HUD** | Responsive virtual analog thumbstick, interactive vehicle entry/exit, action touch buttons |
| 🎮 **Gamepad & Controller Support** | Full Bluetooth controller support (Xbox, PlayStation, Razer Kishi) via EnhancedInput |
| 📦 **Mission: Late Delivery** | 10-phase narrative mission vertical slice with waypoint navigation and rewards |
| 💰 **Underground & City Economy** | Cash, ATM transactions, weapons purchasing, dynamic stock market |
| ⚡ **Mobile Vulkan Optimization** | Optimized draw calls, mobile-friendly TSR upscaling, dynamic chunk streaming |

---

## 📱 Mobile Architecture Overview

```
ProjectRealWorld/
├── Source/ProjectRealWorld/
│   ├── AI/                     # Pedestrian crowds & AI behaviors
│   │   ├── RealWorldNPCAIController      (Sight & hearing perception)
│   │   ├── RealWorldNPCCharacter         (8 archetypes, procedural animation)
│   │   └── RealWorldCrowdManager         (Mobile budget-aware crowd spawning)
│   │
│   ├── Character/              # Player character & mobile input
│   │   ├── RealWorldCharacter            (Movement, stamina, climbing)
│   │   └── RealWorldAnimInstance         (Locomotion state machine)
│   │
│   ├── Combat/                 # Mobile weapons & combat
│   │   ├── RealWorldCombatComponent
│   │   └── RealWorldWeaponBase
│   │
│   ├── Core/                   # Game framework
│   │   ├── RealWorldGameMode             (Mobile spawn rules & economy)
│   │   ├── RealWorldPlayerController     (Touch interface & input switching)
│   │   └── RealWorldSaveGame             (Full local persistence schema)
│   │
│   ├── Platform/               # Android hardware scalability & tuning
│   │   ├── RealWorldPlatformSubsystem    (Auto Snapdragon / Dimensity detection)
│   │   └── RealWorldScalabilityTypes.h   (Mobile performance tiers 0-3)
│   │
│   ├── Subsystems/             # Core world systems
│   │   ├── CrimeDetectionSubsystem       (Witnessing & 911 calls)
│   │   ├── PoliceWantedSubsystem         (0–5 Star pursuit AI)
│   │   ├── MissionSubsystem              (Mission director & checkpoints)
│   │   └── TimeOfDaySubsystem            (Dynamic 24h day/night cycle)
│   │
│   ├── UI/                     # Mobile HUD
│   │   └── RealWorldHUD                  (Touch controls, mini-map, vitals, wanted stars)
│   │
│   └── Vehicles/               # Chaos Vehicles & Traffic
│       ├── RealWorldVehicleBase          (Drivetrain, fuel, deformation)
│       └── RealWorldTrafficManager       (Mobile multi-lane traffic)
│
├── Config/
│   ├── DefaultEngine.ini         # Mobile-ready renderer, EnhancedInput
│   ├── DefaultGame.ini           # Package descriptor & GameMode
│   ├── DefaultInput.ini          # Touch and controller bindings
│   ├── DefaultScalability.ini    # Base engine scalability
│   └── Android/
│       ├── AndroidEngine.ini     # Vulkan ES3.1, ARM64 target, API 34
│       └── AndroidScalability.ini # Mobile Low, Medium, High, Ultra profiles
│
├── Platforms/Android/            # Android packaging assets & manifests
├── Scripts/
│   ├── package_apk.sh            # Native ARM64 compilation pipeline
│   └── deploy.sh                 # Android CI/CD validation tool
└── ProjectRealWorld.apk          # Official ready-to-install Android APK
```

---

## ⚡ Android System Requirements

| Component | Minimum Specification | Recommended Specification |
|---|---|---|
| **Operating System** | Android 8.0 (Oreo) / API 26+ | Android 12 – 14 (API 31–34) |
| **Processor Architecture** | 64-bit ARM (`arm64-v8a`) | 64-bit ARM octa-core (Snapdragon 8 Gen 1+ / Dimensity 9000+) |
| **Graphics API** | Vulkan 1.1+ / OpenGL ES 3.1 | Vulkan 1.2+ |
| **RAM** | 4 GB | 6 GB – 8 GB+ |
| **Free Internal Storage** | 5 GB free space | 8 GB free space |

---

## 🚀 How to Install & Play on Android

### Method 1: Direct 1-Click APK Download (Easiest)
1. On your Android phone, download the latest game APK:  
   👉 **[Download ProjectRealWorld.apk](https://github.com/saumypandey745/ProjectRealWorld/releases/download/live/ProjectRealWorld.apk)**
2. Tap the downloaded file in your notification bar or **Files / Downloads** folder.
3. Tap **Install**.  
   *(If prompted, toggle **Allow from this source** in settings, and choose **Install anyway** if Google Play Protect shows a prompt).*
4. Tap **Open** and start playing!

### Method 2: Install via ADB (USB Debugging)
```bash
# Connect phone via USB with USB Debugging enabled:
adb devices

# Install APK directly:
adb install -r -d ProjectRealWorld.apk
```

---

## 🎮 Mobile Gameplay Controls

| Action | Mobile Touch Control | Bluetooth Controller |
|---|---|---|
| **Movement / Steer** | Left Virtual Analog Stick | Left Thumbstick |
| **Camera View** | Right Screen Swipe | Right Thumbstick |
| **Sprint** | Sprint Button (Right side) | Left Stick Click / L3 |
| **Jump / Vault** | Jump Button | Cross / A Button |
| **Crouch** | Crouch Button | Circle / B Button |
| **Interact / Open Door** | Interact Context Button | Square / X Button |
| **Enter / Exit Vehicle** | Vehicle Door Icon | Triangle / Y Button |
| **Vehicle Accelerate** | Gas Pedal | Right Trigger (RT / R2) |
| **Vehicle Brake / Reverse** | Brake Pedal | Left Trigger (LT / L2) |
| **Handbrake** | Handbrake Icon | Right Bumper (RB / R1) |
| **Aim / Fire** | Crosshair / Fire Button | Right Trigger (RT) |

---

## 🛠️ Building From Source for Android

```bash
# 1. Clone repository
git clone https://github.com/saumypandey745/ProjectRealWorld.git
cd ProjectRealWorld

# 2. Verify Android environment
export ANDROID_HOME=/home/user/Android/sdk
export NDKROOT=${ANDROID_HOME}/ndk/25.1.8937393

# 3. Package Android Shipping APK
./Scripts/deploy.sh package Shipping
```

---

## 📜 License

```
Copyright © 2026 Saumy Pandey — Antigravity Game Studios
All rights reserved. Android Mobile Game Project.
```
