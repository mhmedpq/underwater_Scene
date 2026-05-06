# 🐟 Underwater Scene — 2D OpenGL / GLUT

**Name:** Muhammed Awad Farag Hamed  
**Course:** Computer Graphics

---

## Overview

This program renders an animated **2D underwater scene** using OpenGL and GLUT.
The scene features colourful fish swimming across the screen, rising bubbles,
a sandy seabed with rocks, and a deep-water colour gradient — all running in
real time at ~60 fps.

---

## Screenshot Preview

```
┌─────────────────────────────────────────────────────┐
│  🌊  Deep blue water gradient (top → bottom)        │
│                                                     │
│    🐠   ><(((º>          🐡  ><(((º>                │
│                  🐟  ><(((º>                        │
│   ○  ○    ○  ○  ○    ○    ○   ○   (bubbles)         │
│                                                     │
│▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│
│  🪨      🪨       🪨       🪨      🪨   (seabed)    │
└─────────────────────────────────────────────────────┘
```

---

## Features

| Feature | Details |
|---|---|
| **Fish** | Up to 100 fish, each with a unique colour, size, and speed |
| **Animation** | Tail wagging, vertical sinusoidal swimming path |
| **Bubbles** | 300 bubbles rising with gentle horizontal drift |
| **Background** | Vertical blue gradient simulating water depth |
| **Seabed** | Sandy floor with layered stripe detail and 3D-style rocks |
| **Interactivity** | Add fish / bubbles, randomise colours, reset scene |

---

## Controls

| Key | Action |
|---|---|
| `F` | Add a new fish (up to 100) |
| `B` | Spawn 10 more bubbles |
| `C` | Randomise all fish colours |
| `R` | Reset the scene to its initial state |
| `ESC` | Exit the program |

---

## How to Build & Run

### Requirements

- A C++ compiler (`g++`)
- OpenGL development libraries
- GLUT or FreeGLUT

**Install dependencies on Ubuntu / Debian:**
```bash
sudo apt-get install freeglut3-dev
```

**Install dependencies on Fedora / RHEL:**
```bash
sudo dnf install freeglut-devel
```

### Compile

```bash
g++ main.cpp -o underwater -lGL -lGLU -lglut
```

### Run

```bash
./underwater
```

---

## Project Structure

```
.
└── main.cpp        # All source code in a single file
```

### Code Organisation

| Function | Purpose |
|---|---|
| `main()` | Initialise GLUT, set up OpenGL state, enter main loop |
| `initFish()` | Populate the scene with 8 preset fish |
| `initBubbles()` | Populate the scene with 300 initial bubbles |
| `drawBackground()` | Draw the water gradient, seabed, and rocks |
| `drawFish()` | Draw a single fish (body, fins, tail, eye) |
| `drawBubbles()` | Draw all bubbles with transparency |
| `drawRock()` | Draw a single polygon rock on the seabed |
| `updateFish()` | Advance fish positions and animations each frame |
| `updateBubbles()` | Advance bubble positions and respawn from the bottom |
| `display()` | Main render callback — clears and redraws the scene |
| `timer()` | 16 ms timer callback driving the animation loop |
| `keyboard()` | Handle key presses for user interaction |
| `reshape()` | Handle window resize, maintain orthographic projection |

---

## Data Structures

### `Fish`

```cpp
struct Fish {
    float x, y;       // Current position
    float baseY;       // Centre of vertical oscillation
    float speed;       // Horizontal speed (pixels/frame)
    float direction;   // +1 = right,  -1 = left
    float size;        // Uniform scale factor
    float r, g, b;     // Body colour (RGB 0–1)
    float time;        // Animation phase for tail/swim cycle
};
```

### `Bubble`

```cpp
struct Bubble {
    float x, y;       // Current position
    float speed;       // Rise speed (pixels/frame)
    float radius;      // Bubble size
};
```

---

## How the Animations Work

### Fish swimming
Each fish oscillates vertically using a sine wave:
```
y = baseY + sin(time × 1.5) × 12
```
The tail wags using a separate sine wave applied as a rotation:
```
tailAngle = sin(time × 3.0) × 25°
```
When a fish reaches the edge of the screen it reverses direction.

### Bubbles rising
Each bubble rises every frame and drifts horizontally with:
```
x += sin(globalTime × 1.2 + y × 0.05) × 0.3
```
When a bubble exits the top of the screen it is respawned at the bottom.

---

## Configuration Constants

These can be adjusted at the top of `main.cpp`:

| Constant | Default | Description |
|---|---|---|
| `WIN_W` | `1200` | Window width in pixels |
| `WIN_H` | `800` | Window height in pixels |
| `TIMER_MS` | `16` | Frame interval (~60 fps) |
| `MAX_FISH` | `40` | Maximum number of fish |
| `INIT_BUBBLES` | `300` | Initial bubble count |

---

## Academic Notes

- Uses **orthographic projection** (`gluOrtho2D`) — no depth or perspective.
- All drawing is done with **immediate mode** OpenGL primitives (`GL_TRIANGLES`, `GL_POLYGON`, `GL_LINE_LOOP`, etc.).
- **Alpha blending** (`GL_BLEND`) is enabled for translucent bubble rendering.
- The animation loop is driven by `glutTimerFunc` rather than `glutIdleFunc` to maintain a stable frame rate.

---

## License

This project was created for academic purposes as part of a Computer Graphics course assignment.
