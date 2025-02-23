#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include <map>


class CollisionDetector
{
private:
	/*
		Gets the vertices for the given rectangle motion
		in the folowing order relative to center:
		 
		Top Left, Top Right, Bottom Right, Bottom Left (clockwise order)

		param rectangle: the motion component containing the position and size of the rectangle
	*/
	std::vector<vec2> getRectVertices(const Motion rectangle);
	/*
		Gets the edge vectors for the polygon with the given vertices

		param vertices: the list of vertices that define this polygon
	*/
	std::vector<vec2> getEdges(const std::vector<vec2> vertices);
	/*
		Check if 2 polygon with the given vertices have collided. Return true if they have, false otherwise

		param poly1_v: the vertices of the first polygon
		param poly1_v: the vertices of the second polygon
	*/
	bool checkPolygonCollisionWithVertices(const std::vector<vec2>& poly1_v, const std::vector<vec2>& poly2_v);
	/*
		Check if 2 polygon with the given vertices and edges have collided. Return true if they have, false otherwise

		param poly1_v: the vertices of the first polygon
		param poly1_v: the vertices of the second polygon
	*/
	bool checkPolygonCollision(const std::vector<vec2>& poly1_v, const std::vector<vec2>& poly1_e, const std::vector<vec2>& poly2_v, const std::vector<vec2>& poly2_e);
	/*
		Add the given wall entity and motion component to the cache

		param wall: the wall entity
		param wall_motion: the motion component of the wall
	*/
	void addWallToCache(Entity& wall, const Motion& wall_motion);
	/*
		Checks if the given angle is in the top 2 quadrants (+x/+y and -x/+y)
	*/
	bool isTopQuadrant(float angle);
	/*
		Checks if the given angle is in the right 2 quadrants (+x/+y and +x/-y)
	*/
	bool isRightQuadrant(float angle);

	// the unit vector representing the player's dash direction
	vec2 direction_vector;
	// since walls are static, cache the ones we've already calculated vertices / edges for
	std::map<unsigned int, std::pair<std::vector<vec2>, std::vector<vec2>>> wall_cache;
public:
	/*
		Check if 2 motion components have collided. Treats them both as rectangles using their position and scale
		Return true if they have, false otherwise

		param motion1: the first motion component
		param motion2: the second motion component
	*/
	bool hasCollided(const Motion& motion1, const Motion& motion2);
	/*
		Check if the player with a specific velocity is colliding into the given wall. Then, handle resolving the collision
		Return true if colliding, false if not

		param player_motion: the motion component of the plaer with position, angle and size
		param player_dash: the current velocity of the player
		param wall: the wall entity that we want to test collision for
	*/
	bool checkAndHandleWallCollision(Motion& player_motion, Velocity& player_dash, Entity& wall);
};
