#include "collision_system.hpp"
#include "world_init.hpp"
#include <limits>
#include <stack>
#include <iostream>
#include <math.h>

vec2 CollisionSystem::getDirectionVecFromAngle(float angle)
{
	return { cosf((angle - 90) * (M_PI / 180.0f)), sinf((angle - 90) * (M_PI / 180.0f)) };
}

bool CollisionSystem::isPointInRectangle(vec2 point, std::vector<vec2> vertices)
{
	vec2 top_left = vertices[0];
	vec2 top_right = vertices[1];
	vec2 bottom_left = vertices[3];

	return point.y >= top_left.y && point.y <= bottom_left.y && point.x >= top_left.x && point.x <= top_right.x;
}

std::vector<vec2> CollisionSystem::getRectVertices(const Motion rectangle)
{
	vec2 position = rectangle.position;
	float angle = rectangle.angle;

	float half_width = rectangle.scale.x / 2;
	float half_height = rectangle.scale.y / 2;

	vec2 direction_vector = getDirectionVecFromAngle(angle);
	vec2 perpendicular_vector = { -direction_vector.y, direction_vector.x };
	vec2 half_width_rotated = direction_vector * half_width;
	vec2 half_height_rotated = perpendicular_vector * half_height;

	return {
		position - half_height_rotated + half_width_rotated,
		position + half_height_rotated + half_width_rotated,
		position + half_height_rotated - half_width_rotated,
		position - half_height_rotated - half_width_rotated
	};
}

std::vector<vec2> CollisionSystem::getEdges(const std::vector<vec2> vertices)
{
	std::vector<vec2> edges;
	for (int i = 0; i < vertices.size(); i++)
	{
		if (i + 1 >= vertices.size())
		{
			edges.push_back(vertices[0] - vertices[i]);
		}
		else
		{
			edges.push_back(vertices[i + 1] - vertices[i]);
		}

	}
	return edges;
}

bool CollisionSystem::checkPolygonCollision(const std::vector<vec2>& poly1_v, const std::vector<vec2>& poly1_e, const std::vector<vec2>& poly2_v, const std::vector<vec2>& poly2_e)
{
	float poly1_min;
	float poly1_max;
	float poly2_min;
	float poly2_max;

	std::vector<vec2> perpendiculars;

	for (auto& edge : poly1_e)
	{
		perpendiculars.push_back({ -edge.y, edge.x });
	}

	for (auto& edge : poly2_e)
	{
		perpendiculars.push_back({ -edge.y, edge.x });
	}

	for (auto& perpendicular : perpendiculars)
	{
		poly1_min = std::numeric_limits<float>::max();
		poly1_max = std::numeric_limits<float>::min();
		poly2_min = std::numeric_limits<float>::max();
		poly2_max = std::numeric_limits<float>::min();
		float dot;

		for (auto& vertex : poly1_v)
		{
			dot = vertex.x * perpendicular.x + vertex.y * perpendicular.y;

			if (dot < poly1_min)
			{
				poly1_min = dot;
			}
			if (dot > poly1_max)
			{
				poly1_max = dot;
			}
		}

		for (auto& vertex : poly2_v)
		{
			dot = vertex.x * perpendicular.x + vertex.y * perpendicular.y;

			if (dot < poly2_min)
			{
				poly2_min = dot;
			}
			if (dot > poly2_max)
			{
				poly2_max = dot;
			}
		}

		if (!((poly1_min < poly2_max && poly1_min > poly2_min) || (poly2_min < poly1_max && poly2_min > poly1_min)))
		{
			return false;
		}
	}

	return true;
}

bool CollisionSystem::checkPolygonCollisionWithVertices(const std::vector<vec2>& poly1_v, const std::vector<vec2>& poly2_v)
{
	return checkPolygonCollision(poly1_v, getEdges(poly1_v), poly2_v, getEdges(poly2_v));
}

bool CollisionSystem::hasCollided(const Motion& motion1, const Motion& motion2)
{

	return checkPolygonCollisionWithVertices(getRectVertices(motion1), getRectVertices(motion2));
}

vec2 CollisionSystem::getPointOnPlayer(std::vector<vec2> player_vertices, float player_angle, vec2 wall_edge)
{
	float clamped_angle = clampNegativeAngle(player_angle);
	bool top_left = isQuadrant1Or3(clamped_angle);
	top_left = abs(wall_edge.x) <= 0.01f ? !top_left : top_left;

	return top_left ? player_vertices[0] : player_vertices[1];
}

std::optional<std::pair<std::vector<vec2>, std::vector<vec2>>> CollisionSystem::checkWallCollision(Motion& motion, Entity& wall)
{
	std::vector<vec2> vertices = getRectVertices(motion);
	std::vector<vec2> edges = getEdges(vertices);
	if (wall_cache.find(wall.id()) == wall_cache.end())
	{
		addWallToCache(wall, registry.motions.get(wall));
	}
	auto wall_info = wall_cache.at(wall.id());
	auto wall_vertices = wall_info.first;
	auto wall_edges = wall_info.second;

	if (checkPolygonCollision(vertices, edges, wall_vertices, wall_edges))
	{
		return { wall_info };
	}
	else
	{
		return std::nullopt;
	}
}

float CollisionSystem::getIntersectionDist(vec2 point, vec2 direction, vec2 point2, vec2 direction2)
{
	float multiplier;

	// vertical edge that player is not parallel to
	if (abs(direction2.x) < 0.01f && abs(direction.x) >= 0.01f)
	{
		multiplier = (point2.x - point.x) / direction.x;
	}
	// horizontal edge that player is not parallel to
	else if (abs(direction2.y) < 0.01f && abs(direction.y) >= 0.01f)
	{
		multiplier = (point2.y - point.y) / direction.y;
	}
	// player is parallel to edge, skip
	else return -1;

	vec2 intersection = point + (direction * multiplier);

	return glm::distance(point, intersection);
}

void CollisionSystem::resolveWallCollision(Motion& motion, vec2 intersection_edge, vec2 intersection_edge_vertex, float movement_angle, float object_angle_rad)
{
	// if its a vertical edge
	if (abs(intersection_edge.x) < 0.01f)
	{
		float dist = (abs(cosf(object_angle_rad)) * (motion.scale.y / 2)) + (abs(sinf(object_angle_rad)) * (motion.scale.x / 2));
		// determine if the player was moving to the right or left when colliding
		if (isRightQuadrant(movement_angle))
		{
			// moving to the right, so we have to reduce player x coord
			motion.position.x = std::min(motion.position.x, intersection_edge_vertex.x - dist - 2);
		}
		else
		{
			// moving to the left, so we have to increase player x coord
			motion.position.x = std::max(motion.position.x, intersection_edge_vertex.x + dist + 2);
		}
	}
	// if its a horizontal edge
	else
	{
		float dist = abs(cosf(object_angle_rad)) * (motion.scale.x / 2) + abs(sinf(object_angle_rad)) * (motion.scale.y / 2);
		// determine if the player was moving up or down when colliding
		if (isTopQuadrant(movement_angle))
		{
			// moving up, so we have to increase player y coord
			motion.position.y = std::max(motion.position.y, intersection_edge_vertex.y + dist + 2);
		}
		else
		{
			// moving to down, so we have to reduce player y coord
			motion.position.y = std::min(motion.position.y, intersection_edge_vertex.y - dist - 2);
		}
	}
}

void CollisionSystem::checkAndHandleGeneralWallCollision(Motion& motion, Entity& wall)
{
	auto collision = checkWallCollision(motion, wall);

	if (collision.has_value())
	{
		auto wall_vertices = collision.value().first;
		auto wall_edges = collision.value().second;

		// get direction vector of object movement
		vec2 direction_vector = glm::normalize(motion.velocity);

		vec2 closest_intersection_edge;
		vec2 closest_intersection_edge_vertex;
		float closest_intersection_dist = std::numeric_limits<float>::max();
		bool edge_found = false;

		// find which of the 4 wall edges the object collided with
		// find the intersection of the vector from object center (in the direction of object movement) to every edge
		// use the closest intersection
		for (int i = 0; i < wall_vertices.size(); i++)
		{
			vec2 edge_start = wall_vertices[i];
			vec2 edge_vector = wall_edges[i];
			vec2 edge_end = edge_start + edge_vector;

			float dist_to_intersection = getIntersectionDist(motion.position, direction_vector, edge_start, edge_vector);

			if (dist_to_intersection >= 0 && dist_to_intersection < closest_intersection_dist)
			{
				edge_found = true;
				closest_intersection_dist = dist_to_intersection;
				closest_intersection_edge = edge_vector;
				closest_intersection_edge_vertex = edge_start;
			}
		}

		if (!edge_found) return;

		float velocity_angle = acosf(glm::dot({ 0, -1 }, direction_vector)) * (180.0f / M_PI);
		if (direction_vector.x < 0) velocity_angle = 360.0f - velocity_angle;

		resolveWallCollision(motion, closest_intersection_edge, closest_intersection_edge_vertex, velocity_angle);
	}
}

std::pair<bool, vec2> CollisionSystem::checkAndHandlePlayerWallCollision(Motion& player_motion, float movement_angle, Entity& wall)
{
	auto collision = checkWallCollision(player_motion, wall);

	vec2 edge_of_collision;

	if (collision.has_value())
	{
		auto wall_vertices = collision.value().first;
		auto wall_edges = collision.value().second;

		// collision detected. we now have to deal with moving the player out of the wall
		vec2 direction_vector = getDirectionVecFromAngle(player_motion.angle);
		vec2 player_center = player_motion.position;
		vec2 player_top_middle = player_center + (direction_vector * (PLAYER_BB_WIDTH / 2));
		vec2 player_bottom_middle = player_center - (direction_vector * (PLAYER_BB_WIDTH / 2));
	
		vec2 closest_intersection_edge;
		vec2 closest_intersection_edge_vertex;
		float closest_intersection_dist = std::numeric_limits<float>::max();
		bool edge_found = false;

		// find which of the 4 wall edges the player collided with
		// find the intersection of the vector from player center (in the direction of player movement) to every edge
		// use the closest intersection
		for (int i = 0; i < wall_vertices.size(); i++)
		{
			vec2 edge_start = wall_vertices[i];
			vec2 edge_vector = wall_edges[i];

			// explained in method docs
			vec2 point_on_player = getPointOnPlayer(getRectVertices(player_motion), player_motion.angle, edge_vector);

			if (!isPointInRectangle(point_on_player, wall_vertices)) continue;

			vec2 point_on_player_center = point_on_player + (direction_vector * (player_motion.scale.x / 2));
			vec2 point_on_player_back = point_on_player_center + (direction_vector * (player_motion.scale.x / 2));
			// use points on the center line of the player if possible. however, this point can sometime be inside the wall already when the collision is detected
			// this messes up getting the right edge. if so, we use points on the bottom edge. 
			// basically, the more the player is within the wall, we choose a point further back 
			point_on_player = !isPointInRectangle(point_on_player_center, wall_vertices) ? point_on_player_center : point_on_player_back;

			float dist_to_intersection = getIntersectionDist(point_on_player, direction_vector, edge_start, edge_vector);

			if (dist_to_intersection >= 0 && dist_to_intersection < closest_intersection_dist)
			{
				edge_found = true;
				closest_intersection_dist = dist_to_intersection;
				closest_intersection_edge = edge_vector;
				closest_intersection_edge_vertex = edge_start;
			}
		}

		if (!edge_found) return { false, {0, 0} };
		
		float player_angle_rad = clampNegativeAngle(player_motion.angle) * (M_PI / 180.0f);
		float clamped_movement_angle = clampNegativeAngle(movement_angle);
		float player_width = player_motion.scale.x;
		float player_height = player_motion.scale.y;

		resolveWallCollision(player_motion, closest_intersection_edge, closest_intersection_edge_vertex, clamped_movement_angle, player_angle_rad);

		edge_of_collision = closest_intersection_edge;
	}

	return { collision.has_value(), edge_of_collision};
}

void CollisionSystem::handleDashOnWallEdge(vec2 wall_edge, Dashing& dash)
{
	float dash_angle = dash.angle_deg < 0 ? 360.0f + dash.angle_deg : dash.angle_deg;
	// if its a vertical edge
	if (abs(wall_edge.x) < 0.01f)
	{
		dash.velocity = { 0, dash.velocity.y };
		dash.angle_deg = dash_angle == 90.0f || dash_angle == 270.0f ? dash_angle : isTopQuadrant(dash_angle) ? 0.0f : 180.0f;
	}
	else
	{
		dash.velocity = { dash.velocity.x, 0 };
		dash.angle_deg = dash_angle == 0.0f || dash_angle == 360.0f ? dash_angle : isRightQuadrant(dash_angle) ? 90.0f : 270.0f;
	}
}

void CollisionSystem::addWallToCache(Entity& wall, const Motion& wall_motion)
{
	auto vertices = getRectVertices(wall_motion);
	wall_cache.insert({ wall.id(), std::make_pair(vertices, getEdges(vertices)) });
}

bool CollisionSystem::isTopQuadrant(float angle)
{
	return (angle >= 270 && angle <= 360) || (angle <= 90 && angle >= 0);
}

bool CollisionSystem::isRightQuadrant(float angle)
{
	return angle >= 0 && angle <= 180;
}

bool CollisionSystem::isQuadrant1Or3(float angle)
{
	return (angle >= 0 && angle <= 90) || (angle >= 180 && angle <= 270);
}

float CollisionSystem::clampNegativeAngle(float angle)
{
	return angle < 0 ? 360.0f + angle : angle;
}

