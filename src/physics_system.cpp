// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include "animation_system.hpp"
#include <iostream>

#include <glm/gtx/normalize_dot.hpp>
// include lerp
#include <glm/gtx/compatibility.hpp>

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion &motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return {abs(motion.scale.x), abs(motion.scale.y)};
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion &motion1, const Motion &motion2)
{
	// object 1, set left right
	vec2 bb1 = get_bounding_box(motion1);
	vec2 bb2 = get_bounding_box(motion2);

	return glm::distance(motion1.position, motion2.position) < (bb1.x + bb2.x) / 2;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move each entity that has motion (invaders, projectiles, and even towers [they have 0 for velocity])
	// based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.

	float step_seconds = elapsed_ms / 1000.f;

	// precompute boundaries for player motion
	float leftBound = MAP_LEFT * GRID_CELL_WIDTH_PX;
	float rightBound = MAP_RIGHT * GRID_CELL_WIDTH_PX - 1;
	float topBound = MAP_TOP * GRID_CELL_HEIGHT_PX;
	float bottomBound = MAP_BOTTOM * GRID_CELL_HEIGHT_PX - 1;

	Entity& player_entity = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player_entity);
	Player& player = registry.players.get(player_entity);
	
	// Handle player dashing
	// this needs to be done first since the collision resolution updates the player dash angle
	// so we need the updated velocities before we update player position
	for (uint i = 0; i < registry.dashes.size(); i++)// Is alwats a loop of 1 as we remove other dashes when we start a dash
	{
		Entity dash_entity = registry.dashes.entities[i];
		Dashing& dash = registry.dashes.get(dash_entity);
		Motion& motion = registry.motions.get(player_entity);

		dash.timer_ms -= elapsed_ms;

		if (dash.timer_ms <= 0)
		{
			registry.dashes.remove(dash_entity);
			motion.velocity = { 0, 0 };
			toggleDashAnimation(player_entity, false); // goes back to idle animation
		}
		else
		{
			// update player velocity
			motion.velocity = dash.velocity;

			// Gradually change the velocity
			dash.velocity *= player.dashDecay; // FLAG MIGHT NEED TO SWAP THIS ONE OUT TOO.
		}
	}

	auto &motion_registry = registry.motions;
	for (uint i = 0; i < motion_registry.size(); i++)
	{
		Entity entity = motion_registry.entities[i];
		Motion &motion = motion_registry.components[i];


		motion.position += motion.velocity * step_seconds;

		if (registry.denderiteAIs.has(entity)) {
			
			DenderiteAI& denderiteAI = registry.denderiteAIs.get(entity);

			if (denderiteAI.state == DenderiteState::HUNT) {
				denderiteAI.timeSinceLastRecalc += elapsed_ms;

				bool needsRecalc = denderiteAI.path.empty() ||
                           denderiteAI.timeSinceLastRecalc > denderiteAI.recalcTimeThreshold;

				if (needsRecalc) {
					denderiteAI.path.clear();
            		denderiteAI.currentNodeIndex = 0;

					if(find_path(denderiteAI.path, motion.position, player_motion.position)) {
						denderiteAI.timeSinceLastRecalc = 0;
					} else {
						motion.velocity = {0.f, 0.f};
						motion.angle = 0.f;
					}
				}

				if (!denderiteAI.path.empty()) {
					if (denderiteAI.currentNodeIndex >= (int)denderiteAI.path.size()) {
						motion.velocity = {0.f, 0.f};
						motion.angle = 0.f;
						denderiteAI.path.clear();
						denderiteAI.currentNodeIndex = 0;
					} else {
						ivec2 currentTargetTile = denderiteAI.path[denderiteAI.currentNodeIndex];
						vec2 targetWorldPos = gridCellToPosition(currentTargetTile);
						vec2 offset = targetWorldPos - motion.position;
						float dist = glm::length(offset);
						
						if (dist > 0.0001f) {
							vec2 dir = offset / dist;
							motion.velocity = dir * 300.f;
							motion.angle = atan2f(dir.y, dir.x) * 180.f / M_PI + 90.f;
						} else {
							motion.velocity = {0.f, 0.f};
							motion.angle = 0.f;
						}
		
						float reachThreshold = 5.f;
						if (dist < reachThreshold) {
							denderiteAI.currentNodeIndex++;
						}
					}
				}
			}
		}

		if (registry.spiralProjectiles.has(entity)) {
			float spiral_speed = 0.5f;
			float angle = spiral_speed * step_seconds;

			// 2D Rotation Matrix
			float new_x = motion.velocity.x * cos(angle) - motion.velocity.y * sin(angle);
			float new_y = motion.velocity.x * sin(angle) + motion.velocity.y * cos(angle);
			motion.velocity = { new_x, new_y };
		}

		if (registry.followingProjectiles.has(entity)) {
			float speed = glm::length(motion.velocity);
			vec2 direction = glm::normalize(player_motion.position - motion.position);
			motion.velocity = direction * speed;
			motion.angle = atan2f(direction.y, direction.x) * 180 / M_PI + 90.f;
		}

		if (registry.keys.has(entity)) {
			float dampingFactor = 0.8f;
			motion.velocity *= dampingFactor;
			if (glm::length(motion.velocity) < 0.01f) {
				motion.velocity = vec2(0.0f, 0.0f);
			}
			if (motion.position.x >= rightBound + 1 || motion.position.x <= leftBound ||
				motion.position.y <= topBound || motion.position.y >= bottomBound + 1)  {
					// temporary hexagon motion
					motion.position.x = glm::clamp(motion.position.x, leftBound, rightBound);
					motion.position.y = glm::clamp(motion.position.y, topBound, bottomBound);
					
					if (motion.velocity != vec2(0.0f, 0.0f)) {
						motion.velocity = -1.f * motion.velocity;
					}
				}
		}

		// knockback
		if (registry.players.has(entity)) {
			Player& player = registry.players.get(entity);
            if (player.knockback_duration > 0.0f) {
			    player.knockback_duration -= elapsed_ms;
            }
 
			if (player.knockback_duration < 0.f) {
				motion.velocity = vec2(0.0f, 0.0f);
				player.knockback_duration = 0.f;
			}
		}

		// map boundary checking
		if (registry.players.has(entity) || registry.enemies.has(entity))
		{
			if (motion.position.x >= rightBound + 1 || motion.position.x <= leftBound ||
				motion.position.y <= topBound || motion.position.y >= bottomBound + 1)
			{
				motion.position.x = glm::clamp(motion.position.x, leftBound, rightBound);
				motion.position.y = glm::clamp(motion.position.y, topBound, bottomBound);

				if (registry.players.has(entity))
				{
					motion.velocity *= -0.5f;
				}
				else if (registry.enemies.has(entity))
				{
					if (registry.rbcEnemyAIs.has(entity))
					{
						motion.velocity *= -1;
						motion.angle -= 180;
					} else if (registry.bossAIs.has(entity) || registry.denderiteAIs.has(entity)) {	
                        motion.velocity = vec2(0.f, 0.f);
                    } 
				}
			}

            // precompute boundaries for player motion
            float newLeftBound = gridCellToPosition({0, 0}).x + GRID_CELL_WIDTH_PX / 2.f;
            float newRightBound = gridCellToPosition({19, 0}).x - GRID_CELL_WIDTH_PX / 2.f;
            float newTopBound = gridCellToPosition({0, 0}).y + GRID_CELL_HEIGHT_PX / 2.f;
            float newBottomBound = gridCellToPosition({0, 19}).y - GRID_CELL_HEIGHT_PX / 2.f;

            if (motion.position.x >= newRightBound + 1 || motion.position.x <= newLeftBound ||
				motion.position.y <= newTopBound || motion.position.y >= newBottomBound + 1)
			{

                if (registry.bossAIs.has(entity) && registry.denderiteAIs.has(entity)) {
                    motion.position.x = glm::clamp(motion.position.x, newLeftBound, newRightBound);
                    motion.position.y = glm::clamp(motion.position.y, newTopBound, newBottomBound);

                    motion.velocity *= -0.5f;
                } 
            }
		}

		// buff drops and slides
		if (registry.buffs.has(entity))
		{
			float friction = 0.95f;
			motion.velocity *= friction;
			if (glm::length(motion.velocity) < 5.0f)
			{
				motion.velocity = {0, 0};
			}
		}

	}

	// PLAYER DASH ACTION COOLDOWN
	player.dash_cooldown_timer_ms -= elapsed_ms;
	if (player.dash_cooldown_timer_ms <= 0)
	{
		if (player.dash_count < player.max_dash_count)
		{
			player.dash_count++;

			if (player.dash_count == player.max_dash_count)
			{
				player.dash_cooldown_timer_ms = 0;
			}
			else
			{
				player.dash_cooldown_timer_ms = player.dash_cooldown_ms;
			}
		}
	}
	
	
	// update player grid position
	player.grid_position = positionToGridCell(registry.motions.get(registry.players.entities[0]).position);

	// update dash recharge bubbles
	if (!registry.dashRecharges.entities.empty()) 
	{
		Player &player = registry.players.get(registry.players.entities[0]);
		vec2 playerPos = registry.motions.get(registry.players.entities[0]).position;
		static float counter = 0.0f;
		counter += step_seconds;
		static vec2 previousPlayerPos = playerPos;
		float lerp_factor = 0.3f;
		
		vec2 currentPlayerVelocity = (playerPos - previousPlayerPos) / step_seconds;
		vec2 velocity = glm::lerp(vec2(0.0f), currentPlayerVelocity, lerp_factor);
		previousPlayerPos += velocity * step_seconds;

		int i = 0;
		for (Entity entity : registry.dashRecharges.entities) 
		{
			if (!registry.motions.has(entity)) 
				continue;
			
			Motion &motion = registry.motions.get(entity);
			float angle = (2 * M_PI / DASH_RECHARGE_COUNT) * i;
			
			float offset_x = sin(counter * 2.5f + i * 1.5f) * 5.0f;
			float offset_y = cos(counter * 2.5f + i * 1.5f) * 5.0f;
			float random_radius_offset = sin(counter * 0.8f + i * 0.7f) * 12.0f;
			
			vec2 target_pos = previousPlayerPos + vec2(
				(DASH_RADIUS + random_radius_offset) * cos(angle) + offset_x,
				(DASH_RADIUS + random_radius_offset) * sin(angle) + offset_y
			);
			
			motion.position = glm::lerp(motion.position, target_pos, lerp_factor);
			
			if (i >= player.dash_count) 
				motion.scale = {0, 0};
			else 
				motion.scale = {DASH_WIDTH, DASH_HEIGHT};
			
			i++;
		}
	}

	// Collisions are always in this order: (Player | Projectiles | Chest, Key | Enemy | Wall | Buff)
	for (auto& e_entity : registry.enemies.entities)
	{
		// Handle enemy-projectile or enemy-player collisions
		Motion& e_motion = registry.motions.get(e_entity);
		
		for (auto& proj_entity : registry.projectiles.entities)
		{
			Projectile& projectile = registry.projectiles.get(proj_entity);
			
			// to prevent projectile (from enemy) to enemy collision
			if (projectile.from_enemy) continue;

			Motion& proj_motion = registry.motions.get(proj_entity);

			// ensure the projectile is the "first" entity
			if (detector.hasCollided(proj_motion, e_motion))
			{
				registry.collisions.emplace_with_duplicates(proj_entity, e_entity);
			}
		}

		// ensure the player is the "first" entity
		if (detector.hasCollided(player_motion, e_motion))
		{
			// to make sure the player doesn't get locked to the enemy 
			if ((registry.bossAIs.has(e_entity) || registry.finalBossAIs.has(e_entity)) && glm::length(e_motion.velocity) > 0.1f) {
				player.knockback_duration = 500.f;
			} 

			registry.collisions.emplace_with_duplicates(player_entity, e_entity);
		}

		 handleWallCollision(e_entity);
	}

	for (auto& proj_entity : registry.projectiles.entities)
	{
		Motion& proj_motion = registry.motions.get(proj_entity);
		// ensure the projectile is the "second" entity
		if (detector.hasCollided(proj_motion, player_motion))
		{
			registry.collisions.emplace_with_duplicates(player_entity, proj_entity);
		}
	}

	for (auto& buff_entity : registry.buffs.entities)
	{
		// Handle player-buff collisions
		Motion& buff_motion = registry.motions.get(buff_entity);

		if (detector.hasCollided(player_motion, buff_motion))
		{
			registry.collisions.emplace_with_duplicates(player_entity, buff_entity);
		}

		handleWallCollision(buff_entity);
	}

	for (auto& key_entity : registry.keys.entities)
	{
		// Handle player-key collisions
		Motion& key_motion = registry.motions.get(key_entity);

		if (detector.hasCollided(player_motion, key_motion))
		{
			registry.collisions.emplace_with_duplicates(player_entity, key_entity);
		}

		for (auto& chest_entity : registry.chests.entities)
		{
			// Handle chest-key collisions
			Motion& chest_motion = registry.motions.get(chest_entity);

			if (detector.hasCollided(chest_motion, key_motion))
			{
				registry.collisions.emplace_with_duplicates(chest_entity, key_entity);
			}
		}

		 handleWallCollision(key_entity);
	}

	handleWallCollision(player_entity);
}

void PhysicsSystem::handleWallCollision(Entity& entity)
{
	Motion& motion = registry.motions.get(entity);

	std::vector<Entity> nearby_walls;

	for (auto& wall_entity : registry.walls.entities)
	{
		Motion& wall = registry.motions.get(wall_entity);
		float distance_to_motion = glm::distance(motion.position, wall.position);

		// skip the far away walls
		if (distance_to_motion > GRID_CELL_WIDTH_PX) continue;

		nearby_walls.push_back(wall_entity);
	}

	for (auto& wall_entity : nearby_walls)
	{
		auto edge_of_collision = detector.checkAndHandleWallCollision(motion, wall_entity);
		if (registry.players.has(entity) && registry.dashes.components.size() > 0 && edge_of_collision != EDGE_TYPE::NONE)
		{
			// if player is dashing, modify the dash to have a sliding effect along the wall
			Dashing& dash = registry.dashes.components[0];
			detector.handleDashOnWallEdge(edge_of_collision, dash);
		}
	}
}

// move to collision_detect.cpp
bool PhysicsSystem::willMeshCollideSoon(const Entity& player, const Entity& hexagon, float predictionTime)
{
    Mesh& hexagonMesh = *registry.meshPtrs.get(hexagon);

    Motion& playerMotion = registry.motions.get(player);
    Motion& hexagonMotion = registry.motions.get(hexagon);

    vec2 playerFuturePos = playerMotion.position + playerMotion.velocity * predictionTime;
    vec2 hexagonFuturePos = hexagonMotion.position + hexagonMotion.velocity * predictionTime;

    vec2 diff = playerFuturePos - hexagonFuturePos;
    float distance = length(diff);

    // mesh based collision won't happen 
    if (distance > (playerMotion.scale.x / 2 + hexagonMotion.scale.x / 2))
    {
        return false;
    }

    // mesh based collision will happen

    vec2 playerCenter = playerFuturePos;
    float playerRadius = playerMotion.scale.x / 2;

    std::vector<vec2> hexagonWorldVertices = getWorldVertices(hexagonMesh.textured_vertices, hexagonFuturePos, hexagonMotion.scale);
    int numVertices = hexagonWorldVertices.size();

    bool inside = pointInPolygon(playerCenter, hexagonWorldVertices);

    // check the distance from circle (player) to edge
    for (int i = 0; i < numVertices; i++) {
        int next = (i + 1) % numVertices;

        vec2 A = hexagonWorldVertices[i];
        vec2 B = hexagonWorldVertices[next];

        vec2 AB = A - B;

        float t = dot(playerCenter - A, AB) / dot(AB, AB); // ratio of projections

        t = std::max(0.0f, std::min(1.0f, t)); // clamping

        vec2 projectionPoint = A + AB * t;
        
        float distance = length(playerCenter - projectionPoint);

        if (distance < playerRadius) {
            return true;
        }
    }

    return false;
}

// move to collision_detect.cpp
bool PhysicsSystem::pointInPolygon(const vec2& point, const std::vector<vec2> &polygon)
{
    bool inside = false;

    int numVertices = polygon.size();

    // using ray cast algorithm
    for (int i = 0, j = numVertices - 1; i < numVertices; j = i++) {
        const vec2& vi = polygon[i];
        const vec2& vj = polygon[j];

        if (((vi.y > point.y) != (vj.y > point.y)) &&
            (point.x < (vj.x - vi.x) * (point.y - vi.y) / (vj.y - vi.y) + vi.x))
        {
            inside = !inside;
        }
    }

    return inside;
}

// move to collision_detect.cpp
std::vector<vec2> PhysicsSystem::getWorldVertices(const std::vector<TexturedVertex>& vertices, const vec2 &position, const vec2 &scale) {
    std::vector<vec2> worldVertices;
    for (const auto& vertex : vertices) {
        vec2 worldVertex = {
            vertex.position.x * scale.x + position.x,
            vertex.position.y * scale.y + position.y
        };

        worldVertices.push_back(worldVertex);
    }
    return worldVertices;
}

bool PhysicsSystem::find_path(std::vector<ivec2> & path, vec2 start_world, vec2 end_world)
{	
	const auto& map = registry.proceduralMaps.get(registry.proceduralMaps.entities[0]).map;
	int map_height = map.size();

	ivec2 start_pos = positionToGridCell(start_world);
	ivec2 end_pos = positionToGridCell(end_world);
	
	struct Node {
		ivec2 position;
		int g_cost;
		int h_cost;
		Node* parent;
		int f_cost() const { return g_cost + h_cost; }
	};

	struct CompareNode {
		bool operator()(const Node* a, const Node* b) const {
			return a->f_cost() >= b->f_cost(); 
		}
	};
	
	struct CompareVec2 {
		bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
			if (a.x == b.x) return a.y < b.y;
			return a.x < b.x;
		}
	};

	std::priority_queue<Node*, std::vector<Node*>, CompareNode> open;
    std::set<ivec2, CompareVec2> closed;
    std::map<ivec2, Node*, CompareVec2> all_nodes;

	auto heuristic = [](ivec2 a, ivec2 b) {
        return abs(a.x - b.x) + abs(a.y - b.y); 
    };

	
	Node* start = new Node{ start_pos, 0, heuristic(start_pos, end_pos), nullptr };
    open.push(start);
    all_nodes[start_pos] = start;

    const std::vector<ivec2> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };

	while(!open.empty()) {
		Node* current = open.top();
		open.pop();
		
		// std::cout << "All known nodes:" << std::endl;
		// for (auto& [pos, node] : all_nodes) {
		// 	std::cout << "  (" << pos.x << ", " << pos.y << ") - f: " << node->f_cost() << std::endl;
		// }

		if (current->position == end_pos) {
			while (current) {
				path.push_back(ivec2(current->position));
				current = current->parent;	
			}
			std::reverse(path.begin(), path.end());
			return true;
		}

		closed.insert(current->position);
		
		for (auto& dir : directions) {
			ivec2 neighbor_pos = current->position + dir;

            if (closed.find(neighbor_pos) != closed.end()) continue;
            if (!isTraversable(neighbor_pos)) continue;

            int g_cost = current->g_cost + 1;
            int h_cost = heuristic(neighbor_pos, end_pos);
            int f_cost = g_cost + h_cost;

            if (all_nodes.find(neighbor_pos) == all_nodes.end() || f_cost < all_nodes[neighbor_pos]->f_cost()) {
                Node* neighbor = new Node{ neighbor_pos, g_cost, h_cost, current };
                open.push(neighbor);
                all_nodes[neighbor_pos] = neighbor;
            }
		}
	}

	return false;
}

bool PhysicsSystem::isTraversable(ivec2 pos) {
	const auto& map = registry.proceduralMaps.get(registry.proceduralMaps.entities[0]).map;

	int map_width = map.size();
    if (map_width == 0)
        return false; // map is empty

    int map_height = map[0].size();

	int x = pos.x;
    int y = pos.y;

    // Check bounds
    if (x < 0 || x >= map_width ||
        y < 0 || y >= map_height)
    {
        return false;
    }

	return map[x][y] != tileType::WALL;
}