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

bool CollisionSystem::checkWallCollision(Motion& motion, Entity& wall)
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

	return checkPolygonCollision(vertices, edges, wall_vertices, wall_edges);
}

EDGE_TYPE CollisionSystem::getEdgeOfCollisionAndResolve(Motion& motion, Entity& wall)
{
	// get direction vector of object movement
	float angle = clampAngle(motion.angle);
	
	// cached wall info from before
	Motion wall_motion = registry.motions.get(wall);
	vec2 wall_center = wall_motion.position;
	auto wall_info = wall_cache.at(wall.id());
	auto wall_vertices = wall_info.first;

	auto motion_vertices = getRectVertices(motion);
	float min_dist = std::numeric_limits<float>::max();
	std::vector<int> points_inside_idx;

	// find out which vertices of the motion are currently inside the wall
	for (int i = 0; i < motion_vertices.size(); i++)
	{
		if (isPointInRectangle(motion_vertices[i], wall_vertices))
		{
			points_inside_idx.push_back(i);
		}
	}

	EDGE_TYPE edge_of_collision = EDGE_TYPE::NONE;;

	// if no vertices are in the wall, skip 
	// technically an edge case, but quite rare and hard to deal with
	if (points_inside_idx.empty()) return edge_of_collision;

	if (points_inside_idx.size() > 1)
	{
		// 2 or more points inside the wall 
		// (we only consider the 2 vertices case, as the collision is almost always detected before more vertices intersect)
		// depending on which 2 vertices are in the wall and the angle, we determine the collision edge
		if (abs(angle) < 25.0f || abs(angle - 360) < 25.0f)
		{
			if (points_inside_idx[0] == 0 && points_inside_idx[1] == 1) edge_of_collision = EDGE_TYPE::HORIZONTAL_BOTTOM;
			if (points_inside_idx[0] == 1 && points_inside_idx[1] == 2) edge_of_collision = EDGE_TYPE::VERTICAL_LEFT;
			if (points_inside_idx[0] == 2 && points_inside_idx[1] == 3) edge_of_collision = EDGE_TYPE::HORIZONTAL_TOP;
			if (points_inside_idx[0] == 0 && points_inside_idx[1] == 3) edge_of_collision = EDGE_TYPE::VERTICAL_RIGHT;
		}
		else if (abs(angle - 90) < 25.0f)
		{
			if (points_inside_idx[0] == 0 && points_inside_idx[1] == 1) edge_of_collision = EDGE_TYPE::VERTICAL_LEFT;
			if (points_inside_idx[0] == 1 && points_inside_idx[1] == 2) edge_of_collision = EDGE_TYPE::HORIZONTAL_TOP;
			if (points_inside_idx[0] == 2 && points_inside_idx[1] == 3) edge_of_collision = EDGE_TYPE::VERTICAL_RIGHT;
			if (points_inside_idx[0] == 0 && points_inside_idx[1] == 3) edge_of_collision = EDGE_TYPE::HORIZONTAL_BOTTOM;
		}
		else if (abs(angle - 180) < 25.0f)
		{
			if (points_inside_idx[0] == 0 && points_inside_idx[1] == 1) edge_of_collision = EDGE_TYPE::HORIZONTAL_TOP;
			if (points_inside_idx[0] == 1 && points_inside_idx[1] == 2) edge_of_collision = EDGE_TYPE::VERTICAL_RIGHT;
			if (points_inside_idx[0] == 2 && points_inside_idx[1] == 3) edge_of_collision = EDGE_TYPE::HORIZONTAL_BOTTOM;
			if (points_inside_idx[0] == 0 && points_inside_idx[1] == 3) edge_of_collision = EDGE_TYPE::VERTICAL_LEFT;
		}
		else if (abs(angle - 270) < 25.0f)
		{
			if (points_inside_idx[0] == 0 && points_inside_idx[1] == 1) edge_of_collision = EDGE_TYPE::VERTICAL_RIGHT;
			if (points_inside_idx[0] == 1 && points_inside_idx[1] == 2) edge_of_collision = EDGE_TYPE::HORIZONTAL_BOTTOM;
			if (points_inside_idx[0] == 2 && points_inside_idx[1] == 3) edge_of_collision = EDGE_TYPE::VERTICAL_LEFT;
			if (points_inside_idx[0] == 0 && points_inside_idx[1] == 3) edge_of_collision = EDGE_TYPE::HORIZONTAL_TOP;
		}
	}
	else
	{
		// only 1 point is in the wall, so use the predefine map to figure out the collision edge
		edge_of_collision = COLLISION_TO_EDGE[(int)getAngleQuadrant(angle)][points_inside_idx[0]];
	}

	// if a collision edge is found, resolve the collision
	if (edge_of_collision != EDGE_TYPE::NONE) resolveWallCollision(motion, edge_of_collision, wall_vertices[(int)edge_of_collision]);

	return edge_of_collision;
}

void CollisionSystem::resolveWallCollision(Motion& motion, EDGE_TYPE edge_of_collision, vec2 wall_vertex)
{
	float angle = clampAngle(motion.angle);
	float angle_rad = angle * (180.0f / M_PI);

	// this is the length from the center to any vertex. this is the maximum a motion has to be shifted to resolve a collision (45 deg angle of collision)
	// can technically be made more accurate to the current angle, but a constant distance looks smooth and works well enough
	float dist = sqrt(pow(motion.scale.x / 2, 2) + (motion.scale.y, 2));

	switch (edge_of_collision)
	{
	case EDGE_TYPE::HORIZONTAL_TOP:
		motion.position.y = std::min(motion.position.y, wall_vertex.y - dist);
		break;
	case EDGE_TYPE::VERTICAL_RIGHT:
		motion.position.x = std::max(motion.position.x, wall_vertex.x + dist);
		break;
	case EDGE_TYPE::HORIZONTAL_BOTTOM:
		motion.position.y = std::max(motion.position.y, wall_vertex.y + dist);
		break;
	case EDGE_TYPE::VERTICAL_LEFT:
		motion.position.x = std::min(motion.position.x, wall_vertex.x - dist);
		break;
	}
}

EDGE_TYPE CollisionSystem::checkAndHandleWallCollision(Motion& motion, Entity& wall)
{
	auto collision = checkWallCollision(motion, wall);

	vec2 edge_of_collision;

	if (collision)
	{
		return getEdgeOfCollisionAndResolve(motion, wall);
	}

	return EDGE_TYPE::NONE;
}

void CollisionSystem::handleDashOnWallEdge(EDGE_TYPE wall_edge, Dashing& dash)
{
	float dash_angle = dash.angle_deg < 0 ? 360.0f + dash.angle_deg : dash.angle_deg;
	// if its a vertical edge
	if (wall_edge == EDGE_TYPE::VERTICAL_RIGHT || wall_edge == EDGE_TYPE::VERTICAL_LEFT)
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
	Motion wall_motion_with_buffer = wall_motion;
	wall_motion_with_buffer.scale.x *= 1.01;
	wall_motion_with_buffer.scale.y *= 1.01;
	auto vertices = getRectVertices(wall_motion_with_buffer);
	wall_cache.insert({ wall.id(), std::make_pair(vertices, getEdges(vertices)) });
}

QUADRANT CollisionSystem::getAngleQuadrant(float angle)
{
	if (angle >= 0 && angle < 90) return QUADRANT::QUADRANT_1;
	if (angle >= 90 && angle < 180) return QUADRANT::QUADRANT_2;
	if (angle >= 180 && angle < 270) return QUADRANT::QUADRANT_3;
	if (angle >= 270 && angle < 360) return QUADRANT::QUADRANT_4;
	else return QUADRANT::QUADRANT_1;
}

float CollisionSystem::clampAngle(float angle)
{
	return angle < 0 ? 360.0f + angle : angle >= 360 ? angle - 360 : angle;
}

bool CollisionSystem::isTopQuadrant(float angle)
{
	return (angle >= 270 && angle <= 360) || (angle <= 90 && angle >= 0);
}

bool CollisionSystem::isRightQuadrant(float angle)
{
	return angle >= 0 && angle <= 180;
}