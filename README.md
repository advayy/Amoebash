# Amoebash!

## Table of Contents

1. [Game Description](#game-description)
2. [Game Controls Guide](#game-controls-guide)
   - [Mouse Controls](#mouse-controls)
   - [Keyboard Controls](#keyboard-controls)
3. [Build Instructions](#build-instructions)
   - [Prerequisites](#prerequisites)
4. [Technical Features](#technical-features)
   - [Parallax Scrolling Backgrounds](#parallax-scrolling-backgrounds)
   - [Camera Controls](#camera-controls)
   - [Custom Integrated Assets](#custom-integrated-assets)
   - [Linear Interpolation (LERP)](#linear-interpolation-lerp)
   - [Instanced Rendering](#instanced-rendering)
   - [Procedural Mapping](#procedural-mapping)
   - [Precise Collisions](#precise-collisions)
   - [Text Rendering with FreeType](#text-rendering-with-freetype)
   - [Music with SDL_mixer](#music-with-sdlmixer)
   - [Saving with nlohmann/json](#saving-with-nlohmannjson)
5. [License](#license)
6. [Credits](#credits)
   - [Development Team](#development-team)
   - [Music](#music)
   - [Assets](#assets)
   - [Font](#font)

---

### **Game Description:**

Amoebash is a rogue-lite where you control the nucleus of a brain-eating amoeba on a single mission: infect a human. Start your journey through the nose, navigating hostile environments and battling enemies with a mix of swimming, dashing, and shooting. Your ultimate goal? Reach the brain. But first, you'll need to survive hordes of foes, each stronger than the last, and defeat the multi-stage Brain Boss, who deflects your attacks in an attempt to stop you.

The dungeon floors are procedurally generated, ensuring each run is unique and challenging. Defeat enemies to earn buffs, and use them to grow stronger as you progress.

Don’t worry if you die—you’ll always have the chance to regroup. Each time you fail, you’ll keep at least one of the buffs you’ve collected and get another shot at success. If you discover a powerful buff, it might give you the edge you need for your next attempt.

As you defeat enemies, you'll also earn "Germoney" that can be spent in the Shop to unlock new buffs or upgrade your ability to carry more buffs after each death. Every run counts, and no run is ever truly wasted.

---

## **Game Controls Guide**

### **Mouse Controls**

- **Menu Navigation:**Left-Click on buttons to access the **Shop**, **Information Screens**, or to **Start the Game**.
- **In-Game Actions:**
  - Left-Click to dash in the chosen direction as the **Amoeba**.
  - Attacks will automatically trigger during the dash.
  - Right-Click to shoot projectiles.

---

### **Keyboard Controls**

- **Space:** Pause or resume gameplay.
- **S:** Shoot a projectile.
- **O:** Instantly trigger the **Game Over** screen.
- **Q:** Quit the game and return to the **Start Screen**.

---

## Build Instructions

Follow the steps below to build and run the Amoebash project from source.

### Prerequisites

Before building, ensure that you have the following dependencies installed on your system:

- **CMake** (version 3.1 or higher)
- **GLFW** (for handling the windowing system)
- **SDL2** (for audio and input)
- **FreeType** (for text rendering)
- **GLM** (for mathematics)
- **pkg-config** (for Linux/macOS users)

On **macOS**, you can install these dependencies via Homebrew:

```bash
brew install glfw sdl2 freetype glm pkg-config
```

or on Linux

```
sudo apt-get install libglfw3-dev libsdl2-dev libfreetype6-dev libglm-dev pkg-config
```

Once dependencies are installed, switch to the Amoebash directory and run the following on command line

```
cmake ..
make
```

Then run the following to start the game.

```
./amoebash
```

---

## **Technical Features**

### Parallax Scrolling Backgrounds

In our game, the background consists of **3 layers** that move at different speeds to create a **parallax effect** as the player moves.

### Camera Controls

The camera follows the player’s movements to keep them always in frame. This is achieved through the use of **matrices** to recalculate the camera position dynamically.

### Custom Integrated Assets

Our game uses **pixel-art style assets** for all in-game entities. These assets are loaded and utilized across different systems in the game and designed by Advay Rajguru.

### Linear Interpolation (LERP)

We use **linear interpolation (LERP)** to create smooth transitions in various systems.

- **Camera movement**: LERP is used in the `updateCamera()` function to smoothly transition the camera position.
- **Enemy patrols**: LERP is also applied in the `step()` method to smoothly move enemies along their patrol paths.

### Instanced Rendering

To optimize rendering performance, we use **instanced rendering** for particles, allowing us to draw multiple instances in one batch, reducing overhead.

- Each particle is represented by a **RenderRequest**, but they are skipped from the normal `RenderSystem::draw()` loop and rendered separately via instancing.
- Each particle’s transformation (position, rotation, scale) is computed and stored in an **instance VBO**.
- A single `glDrawElementsInstanced` call renders all particles in one go, improving performance.
  - Check out `RenderSystem::drawInstancedParticles()` for the full implementation.
- The **ParticleSystem** manages the simulation, spawning, and lifecycle of particles, while the **RenderSystem** efficiently batches the rendering.

### Procedural Mapping

The game uses a **procedural mapping** system to generate a new level layout each time. The process works as follows:

- A **2D grid of tiles** is generated where each tile is randomly marked as either a wall or an empty space.
- The layout is then refined using **cellular automata rules** to ensure that the paths are coherent and navigable. A portal and player are placed on the map and checked to see there is a sufficient, non-trivial path between them.
- This allows for a unique game experience every time the player starts a new game.

### Precise Collisions

We implemented the **Separating Axis Theorem (SAT)** for precise collision detection and resolution between the player and the walls in the map.

- The collision detection is handled in:
  - `/src/collision_detect.cpp` – SAT is used to accurately determine if and where the player collides with walls.
- After a collision is detected, the code:
  - Identifies the edge of the wall that the player collided with.
  - Moves the player out of the collision while preserving their original velocity, ensuring **smooth movement** without unnatural stutter or jerks.

### Text Rendering with FreeType

The game uses **FreeType** to render high-quality text. Here's how it works:

- FreeType is used to load font files (e.g., `.ttf` files) and render the characters as textures.
- The **text rendering system** uses these textures to display text on the screen, allowing dynamic, high-quality fonts to be used in the game’s UI and menus.

The relevant code for text rendering can be found in the **text rendering system** files, where FreeType is initialized and the text is rendered to the screen using OpenGL.

### Music with SDL_mixer

For handling **music and sound effects**, the game uses **SDL_mixer**

### Saving with nlohmann/json

For saving and loading game data, the game uses **nlohmann/json**.

- **Game State Saving**: The game state (e.g., player progress) is saved in JSON format, which makes it easy to serialize and deserialize complex data structures.

---

## License

This project is licensed under the **Creative Commons Attribution-NonCommercial 4.0 International License (CC BY-NC 4.0)**.

Please refer to the [LICENSE](LICENSE) file for detailed information.

### Key Points:

- You are free to use, share, and adapt the project for **non-commercial purposes** only.
- **Commercial use** is not allowed (selling the game or using it in paid services).
- You must provide proper **attribution** to the author(s) of the work when using or modifying it.

---

## Credits

This game uses various assets and music, which are credited to their respective authors. Please check below for proper attribution:

### Development Team:

- Advay Rajguru
- Mercury Mcindoe
- Shrey Gangwar
- Dany Raihan
- Hazel Chen
- Saurav Banna

### Music:

- **"All Sound Effects and Music"** by Carter Woodworth, licensed under **CC BY-NC 4.0**.

### Assets:

- **"Pixel Art"** by [Advay Rajguru](https://www.advay.dev), licensed under **CC BY-NC 4.0**.
- **"Effect Artwork"** by Hazel Chen, licensed under **CC BY-NC 4.0**.

### Font:

This game uses the **Pixelify Sans** font, designed by **Stefie Justprince**, which is licensed under the **SIL Open Font License, Version 1.1**.

- **Font Name**: Pixelify Sans
- **Designer**: Stefie Justprince
- **License**: [SIL Open Font License, Version 1.1](https://openfontlicense.org)
- **Copyright**: 2021 The Pixelify Sans Project Authors ([GitHub Repository](https://github.com/eifetx/Pixelify-Sans))

You can access the font here: [Google Fonts - Pixelify Sans](https://fonts.google.com/specimen/Pixelify+Sans).
