# team-01 (Infection Games)

---

## **Game Name:** Amoebash

---

#### **Members:**

* Advay Rajguru (85918902)
* Mercury Mcindoe (85594505)
* Saurav Banna (43442367)
* Shrey Gangwar (76979327)
* Hazel Chen (83988873)
* Dany Raihan (53341608)

---
#### **Game Description:**

Amoebash is a roguelike game where you play as the nucleus of a brain-eating amoeba with one goal: to infect a human being. You start by going through a person’s nose, and you’ll need to swim, dash, and slash through your opponents while facing various challenges, with the final goal of reaching the brain. If you make it to the end, you face off against a multi-segment  Neuron Dragon that uses Electric Attacks to stop you. Chip away at the dragon segment by segment and you might just get the glory of taking over a human!

Throughout your journey, you’ll face off against hordes of enemies in procedurally generated dungeon floors, with each one getting harder and harder. Don’t worry though; each enemy you defeat has a chance of dropping buffs to strengthen yourself during your run. You’ll be able to get new weapons or power ups, which will help you later in the game.

Don’t sweat it if the Neuron Dragon or any other enemies get the best of you—just retreat and regroup! On your way out, you get to keep at least one buff you collected for the next run through a “Nucleus Menu”. You might’ve stumbled upon a powerful buff, giving you an edge for your next daring attempt, which means a good run is never wasted!

Each enemy can also give you “Germoney”, which you can use in the Shop to buy new buffs, or increase how many buffs you can keep when you die.

---
## 🎮 **Game Controls Guide**  

### 🖱️ **Mouse Controls**  
- **Menu Navigation:** Click on buttons to access the **Shop**, **Information Screens**, or **Start the Game**.  
- **In-Game Actions:** Click anywhere during gameplay to make the **Amoeba** dash in the chosen direction. **Attacks** will automatically trigger during the dash.  

---

### ⌨️ **Keyboard Controls**  
- **R:** Restart the game.  
- **Space:** Pause or resume gameplay.  
- **O:** Instantly trigger the **Game Over** screen.  
- **Q:** Quit the game and return to the **Start Screen**.  

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
- LERP 
	- Within the `updateCamera()` method, we use LERP to smoothly move the camera.
	- We also use LERP during enemy patrol, in the `step()` method in `/src/physics_system.cpp`
