#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface *> registry_list;

public:
	ComponentContainer<Progression> progressions;

	// Manually created list of all components this game has
	// TODO: A1 add a LightUp component
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh *> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Deadly> deadlys;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	// IMPORTANT: Add any new CC's below to the registry_list
	ComponentContainer<GridLine> gridLines;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<BacteriophageProjectile> bacteriophageProjectiles;
	ComponentContainer<BossProjectile> bossProjectiles;
	ComponentContainer<FinalBossProjectile> finalBossProjectiles;
    ComponentContainer<Portal> portals;

	// mine
	ComponentContainer<VignetteTimer> vignetteTimers;
	ComponentContainer<Animation> animations;
	ComponentContainer<Buff> buffs;
	ComponentContainer<SpriteSize> spritesSizes;
	ComponentContainer<Dashing> dashes;
	ComponentContainer<Map> maps;
	ComponentContainer<Tile> tiles;
	ComponentContainer<Wall> walls;
	ComponentContainer<Camera> cameras;
	ComponentContainer<SpriteSheetImage> spriteSheetImages;

	// mercury
	ComponentContainer<screenButton> buttons;
	ComponentContainer<GameScreen> gameScreens;
	ComponentContainer<Pause> pauses;
	ComponentContainer<Over> overs;
	ComponentContainer<Start> starts;
	ComponentContainer<Shop> shops;
	ComponentContainer<Info> infos;
	ComponentContainer<GameplayCutScene> cutscenes;
	ComponentContainer<MiniMap> miniMaps;
	ComponentContainer<Key> keys;
	ComponentContainer<Chest> chests;
	ComponentContainer<ProceduralMap> proceduralMaps;
	ComponentContainer<InfoBox> infoBoxes;

	// debaounce for damage cooldwn
	ComponentContainer<DamageCooldown> damageCooldowns;

	// enemy state and behavior

	// hazel
	ComponentContainer<UIElement> uiElements;
	ComponentContainer<HealthBar> healthBars;
	ComponentContainer<DashRecharge> dashRecharges;
	ComponentContainer<BuffUI> buffUIs;
	ComponentContainer<ClickableBuff> clickableBuffs;

	// enemy behaviors
	ComponentContainer<SpikeEnemyAI> spikeEnemyAIs;
	ComponentContainer<RBCEnemyAI> rbcEnemyAIs;
	ComponentContainer<BacteriophageAI> bacteriophageAIs;
	ComponentContainer<BossAI> bossAIs;
	ComponentContainer<FinalBossAI> finalBossAIs;
	ComponentContainer<BossArrow> bossArrows;

	// particle
	ComponentContainer<Particle> particles;

    ComponentContainer<Gun> guns;
	// NUCLEUS MENU SLOT
	ComponentContainer<Slot> slots;

	// constructor that adds all containers for looping over them
	ECSRegistry()
	{
		// TODO: A1 add a LightUp component
		registry_list.push_back(&progressions);
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&deadlys);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&gridLines);
		registry_list.push_back(&enemies);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&bacteriophageProjectiles);
        registry_list.push_back(&portals);
		registry_list.push_back(&vignetteTimers);
		registry_list.push_back(&animations);
		registry_list.push_back(&buffs);
		registry_list.push_back(&spritesSizes);
		registry_list.push_back(&dashes);
		registry_list.push_back(&maps);
		registry_list.push_back(&tiles);
		registry_list.push_back(&walls);
		registry_list.push_back(&cameras);
		registry_list.push_back(&spriteSheetImages);
		registry_list.push_back(&buttons);
		registry_list.push_back(&gameScreens);
		registry_list.push_back(&pauses);
		registry_list.push_back(&overs);
		registry_list.push_back(&starts);
		registry_list.push_back(&shops);
		registry_list.push_back(&infos);
		registry_list.push_back(&keys);
		registry_list.push_back(&chests);
		registry_list.push_back(&cutscenes);
		registry_list.push_back(&miniMaps);
		registry_list.push_back(&proceduralMaps);
		registry_list.push_back(&damageCooldowns);
		registry_list.push_back(&uiElements);
		registry_list.push_back(&healthBars);
		registry_list.push_back(&dashRecharges);
		registry_list.push_back(&infoBoxes);
		registry_list.push_back(&buffUIs);
		registry_list.push_back(&spikeEnemyAIs);
		registry_list.push_back(&rbcEnemyAIs);
		registry_list.push_back(&bacteriophageAIs);
		registry_list.push_back(&particles);
		registry_list.push_back(&bossAIs);
		registry_list.push_back(&bossProjectiles);
        registry_list.push_back(&guns);
		registry_list.push_back(&slots);
        registry_list.push_back(&clickableBuffs);
		registry_list.push_back(&bossArrows);
		registry_list.push_back(&finalBossAIs);
		registry_list.push_back(&finalBossProjectiles);
	}

	void clear_all_components()
	{
		for (ContainerInterface *reg : registry_list)
			reg->clear();
	}

	void list_all_components()
	{
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface *reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e)
	{
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface *reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e)
	{
		for (ContainerInterface *reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;