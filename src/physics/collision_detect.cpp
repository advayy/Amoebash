#include "collision_detect.hpp"
#include "world_init.hpp"
#include <limits>
#include <stack>
#include <iostream>
#include <math.h>

std::vector<vec2> CollisionDetector::getRectVertices(const Motion rectangle)
{
	vec2 position = rectangle.position;
	float angle = rectangle.angle;

	float half_width = rectangle.scale.x / 2;
	float half_height = rectangle.scale.y / 2;

	direction_vector = { cosf((angle - 90) * (M_PI / 180.0f)), sinf((angle - 90) * (M_PI / 180.0f)) };
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

std::vector<vec2> CollisionDetector::getEdges(const std::vector<vec2> vertices)
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

// Implementation of the Seperating Axis Theorem
// Detects collisions between any 2 convex polygons, no matter how they are rotated
// Video explaining it: https://www.youtube.com/watch?v=MvlhMEE9zuc
// This function takes in polygons as a vector of vertices. So you'll have to convert your polygon 
// to the correctly rotated points before using this
bool CollisionDetector::checkPolygonCollision(const std::vector<vec2>& poly1_v, const std::vector<vec2>& poly1_e, const std::vector<vec2>& poly2_v, const std::vector<vec2>& poly2_e)
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

bool CollisionDetector::checkPolygonCollisionWithVertices(const std::vector<vec2>& poly1_v, const std::vector<vec2>& poly2_v)
{
	return checkPolygonCollision(poly1_v, getEdges(poly1_v), poly2_v, getEdges(poly2_v));
}

bool CollisionDetector::hasCollided(const Motion& motion1, const Motion& motion2)
{

	return checkPolygonCollisionWithVertices(getRectVertices(motion1), getRectVertices(motion2));
}

bool CollisionDetector::checkAndHandleWallCollision(Motion& player_motion, Velocity& player_dash, Entity& wall)
{
	std::vector<vec2> player_vertices = getRectVertices(player_motion);
	std::vector<vec2> player_edges = getEdges(player_vertices);
	if (wall_cache.find(wall.id()) == wall_cache.end())
	{
		addWallToCache(wall, registry.motions.get(wall));
	}
	auto wall_info = wall_cache.at(wall.id());
	auto wall_vertices = wall_info.first;
	auto wall_edges = wall_info.second;

	bool collision = checkPolygonCollision(player_vertices, player_edges, wall_vertices, wall_edges);

	if (collision)
	{
		// collision detected. we now have to deal with moving the player out of the wall
		vec2 player_center = player_motion.position;

		vec2 closest_intersection_edge;
		vec2 closest_intersection_edge_vertex;
		float closest_intersection_dist = std::numeric_limits<float>::max();

		// find which of the 4 wall edges the player collided with
		// find the intersection of the vector from player center (in the direction of player movement) to every edge
		// use the closest intersection
		for (int i = 0; i < wall_vertices.size(); i++)
		{
			vec2 edge_start = wall_vertices[i];
			vec2 edge_vector = wall_edges[i];

			float denominator = direction_vector.x * -edge_vector.y + edge_vector.x * direction_vector.y;

			float multiplier = -(player_center.y - edge_start.y) * (1 / (denominator));
			vec2 intersection = player_center + (direction_vector * multiplier);
			float dist_to_intersection = glm::distance(player_center, intersection);
			if (dist_to_intersection < closest_intersection_dist)
			{
				closest_intersection_dist = dist_to_intersection;
				closest_intersection_edge = edge_vector;
				closest_intersection_edge_vertex = edge_start;
			}
		}

		float angle_rad = (player_motion.angle - 90) * (M_PI / 180.0f);
		float player_width = player_motion.scale.x;
		float player_height = player_motion.scale.y;

		// if its a vertical edge
		if (closest_intersection_edge.x == 0)
		{
			// determine if the player was moving to the right or left when colliding
			float dist = (abs(cosf(angle_rad)) * (player_height / 2)) + (abs(sinf(angle_rad)) * (player_width / 2));
			if (isRightQuadrant(player_dash.angle))
			{
				// moving to the right, so we have to reduce player x coord
				player_motion.position.x = std::min(player_motion.position.x, closest_intersection_edge_vertex.x - dist - 10);
			}
			else
			{
				// moving to the left, so we have to increase player x coord
				player_motion.position.x = std::max(player_motion.position.x, closest_intersection_edge_vertex.x + dist + 10);
			}
			player_dash.speed = abs(player_dash.speed * sinf((player_dash.angle - 90) * (M_PI / 180.0f)));
			player_dash.angle = isTopQuadrant(player_dash.angle) ? 0 : 180;
		}
		// if its a horizontal edge
		else
		{
			// determine if the player was moving up or down when colliding
			float dist = abs(cosf(angle_rad)) * (player_width / 2) + abs(sinf(angle_rad)) * (player_height / 2);
			if (isTopQuadrant(player_dash.angle))
			{
				// moving up, so we have to increase player y coord
				player_motion.position.y = std::max(player_motion.position.y, closest_intersection_edge_vertex.y + dist + 10);
			}
			else
			{
				// moving to down, so we have to reduce player y coord
				player_motion.position.y = std::min(player_motion.position.y, closest_intersection_edge_vertex.y - dist - 10);
			}
			player_dash.speed = abs(player_dash.speed * cosf((player_dash.angle - 90) * (M_PI / 180.0f)));
			player_dash.angle = isRightQuadrant(player_dash.angle) ? 90 : 270;
		}
	}

	return collision;
}

void CollisionDetector::addWallToCache(Entity& wall, const Motion& wall_motion)
{
	auto vertices = getRectVertices(wall_motion);
	wall_cache.insert({ wall.id(), std::make_pair(vertices, getEdges(vertices))});
}

bool CollisionDetector::isTopQuadrant(float angle)
{
	return (angle >= 270 && angle <= 360) || (angle <= 90 && angle >= 0);
}

bool CollisionDetector::isRightQuadrant(float angle)
{
	return angle >= 0 && angle <= 180;
}