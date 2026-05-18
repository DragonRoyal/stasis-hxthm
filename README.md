# stasis-hxthm

Hackathon-built **desktop robotic arm** prototype combining mostly 3D‑printed parts with hobby servos and a stepper motor, driven by an **Arduino Nano** for a fast, demoable build.

## Demo
- Video/demo: **TODO** (add link)

## What this is
A small, lightweight arm intended for quick iteration during a 4‑day sprint:
- **3D‑printed structure** (links, brackets, mounts)
- **SG90 micro servo** for a light-duty joint / end-effector motion
- **Larger servo** for higher-torque joint motion
- **Stepper motor** for a base axis or positioning stage
- **Arduino Nano** as the main controller

> If you have a specific arm name (e.g., “Stasis Arm v1”) or the number of axes/DOF, add it here.

## Bill of Materials (BOM)
_This list is intentionally hackathon-friendly; exact specs can be filled in later._

### 3D-printed parts (PLA/PETG recommended)
- Arm links (upper/lower)
- Servo brackets / mounts
- Stepper mount
- Base plate
- End-effector mount (gripper mount or tool plate)
- Cable guides / strain relief
- Spacers / bushings (as needed)

### Electronics
- 1× **Arduino Nano**
- 1× **SG90 micro servo**
- 1× **Larger servo** (e.g., MG996R-class or equivalent)
- 1× **Stepper motor** (e.g., NEMA 17-class or equivalent)
- 1× Stepper driver (A4988 / DRV8825 / etc.)
- Power supply breakout boards (as used in the build)
- 9V batteries (as used in the build)
- Jumper wires (male/male, male/female)
- Breadboard or perfboard (optional)
- Assorted connectors (Dupont, screw terminals, etc.)

### Hardware / fasteners
- Assorted M3 screws + nuts (primary)
- Heat-set inserts (optional but recommended)
- Zip ties / Velcro for cable management

### Tools
- 3D printer
- Soldering iron + solder
- Hex drivers / screwdrivers

## Repository layout (suggested)
- `cad/` — CAD source files + STLs
- `electronics/` — schematics, wiring diagrams, pinouts
- `firmware/` — Arduino sketches / control code
- `software/` — host-side scripts (Python) / testing tools
- `media/` — photos, renders, demo video assets

## Pictures
Add build photos and renders here.

- TODO: `media/photos/`
- TODO: finished arm photo
- TODO: wiring close-ups

## CAD
- TODO: link to CAD files in `cad/`
- TODO: exported STLs in `cad/stl/`

## Electronics
- TODO: wiring diagram in `electronics/wiring-diagram.png`
- TODO: pin map (Nano pins → servos/stepper driver)
- TODO: power distribution notes (breakout boards + 9V batteries)

## Firmware / Control
- TODO: describe how joints are controlled (PWM for servos, STEP/DIR for stepper)
- TODO: calibration procedure / limits

## Hackathon scope (original)
- Duration: 4 days
- Focus: hardware + electronics prototyping
- Goal: build a working demo by the end of Day 4

## Suggested 4-day plan
1. **Day 1**: Define problem, architecture, and parts list.
2. **Day 2**: Assemble hardware and wire core electronics.
3. **Day 3**: Integrate firmware/software and test subsystems.
4. **Day 4**: Stabilize, demo prep, and final presentation.
