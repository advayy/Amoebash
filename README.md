# team-01 (Infection Games)

---

## **Game Name:** Amoebash

---

#### **Members:**

- Advay Rajguru (85918902)
- Mercury Mcindoe (85594505)
- Saurav Banna (43442367)
- Shrey Gangwar (76979327)
- Hazel Chen (83988873)
- Dany Raihan (53341608)

---

#### **Game Description:**

Amoebash is a roguelike game where you play as the nucleus of a brain-eating amoeba with one goal: to infect a human being. You start by going through a person’s nose, and you’ll need to swim, dash, and slash through your opponents while facing various challenges, with the final goal of reaching the brain. If you make it to the end, you face off against a multi-segment Neuron Dragon that uses Electric Attacks to stop you. Chip away at the dragon segment by segment and you might just get the glory of taking over a human!

Throughout your journey, you’ll face off against hordes of enemies in procedurally generated dungeon floors, with each one getting harder and harder. Don’t worry though; each enemy you defeat has a chance of dropping buffs to strengthen yourself during your run. You’ll be able to get new weapons or power ups, which will help you later in the game.

Don’t sweat it if the Neuron Dragon or any other enemies get the best of you—just retreat and regroup! On your way out, you get to keep at least one buff you collected for the next run through a “Nucleus Menu”. You might’ve stumbled upon a powerful buff, giving you an edge for your next daring attempt, which means a good run is never wasted!

Each enemy can also give you “Germoney”, which you can use in the Shop to buy new buffs, or increase how many buffs you can keep when you die.

---

## 🎮 **Game Controls Guide**

### 🖱️ **Mouse Controls**

- **Menu Navigation:** Click on buttons to access the **Shop**, **Information Screens**, **Start the Game**, **Save the game** or **Collect Buffs from previous runs**.
- **In-Game Actions:** Click anywhere during gameplay to make the **Amoeba** dash in the chosen direction. **Attacks** will automatically trigger during the dash. You can also **Shoot!**

---

### ⌨️ **Keyboard Controls**

- **S:** Shoot with the pet bacteriophage
- **R:** Restart the game.
- **Space:** Pause or resume gameplay.
- **O:** Instantly trigger the **Game Over** screen.
- **Q:** Quit the game and return to the **Start Screen**.
- **L:** Load saved data on **Start Screen**.

---

## **Milestone Features**

### **M1**

Basic Features

- Parallax scrolling backgrounds
  - In our game, the background consists of 3 layers moving to create a parallax effect when the player moves. Take a look at `/data/textures/tiles/parallax_tile_1_128x.png` to see the different textures, and `/shaders/tile.fs.glsl` to see the implementation.
- Camera controls
  - As the player moves, the camera follows along. We use matrices to recalculate the camera position to always keep the player in frame. Check out the `updateCamera()` function in `/src/world_system.cpp` to see the implementation.
- Basic integrated assets
  - We have pixel-art style assets for all entities in the game. Check out the `/data/textures` folder for the asset files, and `/src/render_system.hpp` to see where we define them in code.
- Linear Interpolation
  - Within the `updateCamera()` method, we use LERP to smoothly move the camera.
  - We also use LERP during enemy patrol, in the `step()` method in `/src/physics_system.cpp`

### **M2**

Advanced Feature
- Precise Collisions
	- We have precise collisions among different entities (e.g. wall and player , mob and player , wall and mob). We use the "Separating Axis Theorem" to detect collisions happening among different entities.
	- You can take a look at 'collision_detect.cpp' and 'collision_detect.hpp' in the collisions folder for further detail.

### **M3**
Advanced Features
- Particle Systems with Instanced Rendering
  - Every entity has a RenderRequest, but particles are skipped in the normal `RenderSystem::draw()` loop—they’re drawn separately via instancing.
  - Each particle’s transformation (position, rotation, scale) is computed and stored in an instance VBO.
  - A single `glDrawElementsInstanced` call renders all particles in one batch, reducing overhead. Please see `RenderSystem::drawInstancedParticles()`
  - The `ParticleSystem` handles particle simulation, spawning, and lifecycle management, while `RenderSystem` efficiently batches their rendering.

Basic Features
- Reloadability
  - On any stage except for tutorial / boss stage, progress can be saved by clicking on the button when the game is paused.
  - If wanted to load the save progress, you can press "L" in the start screen to load the JSON and continue gameplay.
  - Functions `loadgame, savegame, loadprogress, saveprogress` do this by writing / reading JSON files (progress.json, world_status.json) from the save directory.
  - In `components.hpp`, we have MACROS defined to allow `to_json, from_json` for user-defined structures.

- Audio feedback
  - In `world_system.cpp`, we load audio files that we have been provided from a music artist.
  - For interactions such as "button click", "taking damage from enemy", "projectiles shot from enemies", "enemy dying" we trigger different audio files to indicate.
  - On clicking start and progressing into the game, we also have background music playing.

### **M4**
Advanced Features
- **A-star Path finding**
  - In `physics_system.cpp`, we use A* path finding for the 'dendrite' enenmies.
    - Dendrites spawn on the last two stages and find their way towards you.
    - Their path is recalculated every few seconds in order to adjust for the player movement in that time frame.

Basic Features
- Various buff stats, for instance in `applyBuff`, are applied with some capacity to prevent the game from becoming too easy.
- Accordingly, enemy damage and patterns were deeply tested to make the game more playable without it being too hard or too easy.