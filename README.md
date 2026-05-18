# 3D Cursor Robotic Arm

A robotic arm controlled by a **barcode-scanned spherical dome** (“3D cursor”). By scanning different barcodes inside the dome with a barcode scanner, the arm moves to different points / presets.

## Demo
- Video/demo: https://www.youtube.com/watch?v=vmy95ypN5ZI

## What this is
A small, lightweight arm intended for quick iteration during a 4‑day sprint:
- **3D‑printed structure** (links, brackets, mounts)
- **SG90 micro servo** for a light-duty joint / end-effector motion
- **Larger servo** (MG996R-class) for higher-torque joint motion
- **28BYJ-48 5V stepper motor** for the base axis / positioning
- **Arduino Nano** as the main controller

## Bill of Materials (BOM)

| Category | Item | Qty | Notes |
|---|---:|---:|---|
| 3D Printed | Arm links (upper/lower) | 1 set | Print in PLA/PETG |
| 3D Printed | Servo brackets / mounts | 1 set | For SG90 + MG996R |
| 3D Printed | Stepper mount | 1 | For 28BYJ-48 |
| 3D Printed | Base plate + lid | 1 set | Houses electronics |
| 3D Printed | End-effector / claw parts | 1 set | See CAD folder |
| 3D Printed | Cable guides / strain relief | as needed | Optional but helpful |
| Electronics | Arduino Nano | 1 | Main controller |
| Electronics | SG90 micro servo | 1 | Small joint / gripper |
| Electronics | MG996R servo (or similar) | 1 | Higher torque joint |
| Electronics | 28BYJ-48 5V stepper motor | 1 | Base axis |
| Electronics | Stepper driver board (ULN2003) | 1 | Common driver for 28BYJ-48 |
| Electronics | Power supply breakout boards | 1+ | As used in build |
| Electronics | 9V batteries | 1+ | As used in build |
| Electronics | Jumper wires | 1 set | Male/male + male/female |
| Electronics | Breadboard / perfboard | 0–1 | Optional |
| Hardware | M3 screws + nuts | assorted | Primary fasteners |
| Hardware | Heat-set inserts | optional | Recommended for durability |
| Hardware | Zip ties / Velcro | assorted | Cable management |
| Tools | 3D printer | 1 | — |
| Tools | Soldering iron + solder | 1 | — |
| Tools | Hex drivers / screwdrivers | 1 set | — |

## Pictures

Build photos (preview):

<img src="https://cdn.hackclub.com/019e39be-f8bb-73e1-bd95-b19ef0ed9d60/img_7740.jpeg" alt="Arm build photo 1" width="420" />

<img src="https://cdn.hackclub.com/019e39be-fbeb-781c-8327-cb57865b881d/img_7742.jpeg" alt="Arm build photo 2" width="420" />

## CAD

<img src="https://cdn.hackclub.com/019e39d6-7b43-7dc2-86f7-af6b988df20b/image.png" alt="CAD Photo" width="420" />

## Electronics

Wiring / electronics photo (preview):

<img src="https://cdn.hackclub.com/019e39be-f5b6-799e-b6e0-9c5d24bdb92a/img_7741.jpeg" alt="Electronics photo" width="420" />

## Firmware / Control

Firmware / control notes (preview):

<img src="https://cdn.hackclub.com/019e39c8-6bf0-71b5-a9b2-512f90c365fa/img_7751.jpeg" alt="Firmware photo 1" width="420" />

<img src="https://cdn.hackclub.com/019e39c8-6e88-7f0c-953f-dc29c5c4e641/img_7752.jpeg" alt="Firmware photo 2" width="420" />
