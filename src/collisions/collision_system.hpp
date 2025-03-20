#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include <map>
#include <optional>

// enum for the 4 types of wall edges
//
//         ------- Horizontal Top -------
//        |                             |
//        |                             |
//    Vertical Left                Vertical Right
//        |                             |
//        |                             |
//        |_____ Horizontal Bottom _____|
enum class EDGE_TYPE
{
	HORIZONTAL_TOP = 0,
	VERTICAL_RIGHT = HORIZONTAL_TOP + 1,
	HORIZONTAL_BOTTOM = VERTICAL_RIGHT + 1,
	VERTICAL_LEFT = HORIZONTAL_BOTTOM + 1,
	NONE = VERTICAL_LEFT + 1
};

// enum for the different quadrants an angle can be in
enum class QUADRANT
{
	QUADRANT_1 = 0,
	QUADRANT_2 = QUADRANT_1 + 1,
	QUADRANT_3 = QUADRANT_2 + 1,
	QUADRANT_4 = QUADRANT_3 + 1
};

// Map of [Angle Quadrant][Index of Vertex inside a wall] to the edge of collision
const EDGE_TYPE COLLISION_TO_EDGE[4][4] = {
	{EDGE_TYPE::HORIZONTAL_BOTTOM, EDGE_TYPE::VERTICAL_LEFT, EDGE_TYPE::HORIZONTAL_TOP, EDGE_TYPE::VERTICAL_RIGHT},
	{EDGE_TYPE::VERTICAL_LEFT, EDGE_TYPE::HORIZONTAL_TOP, EDGE_TYPE::VERTICAL_RIGHT, EDGE_TYPE::HORIZONTAL_BOTTOM},
	{EDGE_TYPE::HORIZONTAL_TOP, EDGE_TYPE::VERTICAL_RIGHT, EDGE_TYPE::HORIZONTAL_BOTTOM, EDGE_TYPE::VERTICAL_LEFT},
	{EDGE_TYPE::VERTICAL_RIGHT, EDGE_TYPE::HORIZONTAL_BOTTOM, EDGE_TYPE::VERTICAL_LEFT, EDGE_TYPE::HORIZONTAL_TOP}
};

class CollisionSystem
{
private:
	/*
		Checks if the given point is within the given rectangle
		Rectangle is defined by 4 points in the order Top Left, Top Right, Bottom Right, Bottom Left (clockwise order)

		param point: the point to check
		param vertices: the 4 vertices of the rectangle
	*/
	bool isPointInRectangle(vec2 point, std::vector<vec2> vertices);
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
		Polygon vertices need to be in clockwise order with no repeats, but can start at any vertex

		param poly1_v: the vertices of the first polygon
		param poly1_v: the vertices of the second polygon
	*/
	bool checkPolygonCollisionWithVertices(const std::vector<vec2>& poly1_v, const std::vector<vec2>& poly2_v);
	/*
		Check if 2 polygon with the given vertices and edges have collided. Return true if they have, false otherwise

		Implements the Seperating Axis Theorem, which etects collisions between any 2 convex polygons, no matter how they are rotated
		Video explaining it: https://www.youtube.com/watch?v=MvlhMEE9zuc
		Polygon vertices need to be in clockwise order with no repeats, but can start at any vertex

		param poly1_v: the vertices of the first polygon
		param poly1_e: the edges of the first polygon
		param poly1_v: the vertices of the second polygon
		param poly1_e: the edges of the second polygon
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
	/*
		Returns the quadrant that the given angle is in
	*/
	QUADRANT getAngleQuadrant(float angle);
	/*
		Gets the direction vector for the given angle
	*/
	vec2 getDirectionVecFromAngle(float angle);
	/*
		Clamps the given angle to between 0 and 360, so adds 360 to negative angles
	*/
	float clampAngle(float angle);
	/*
		Check collisions between the given motion and wall

		param motion: the motion component which may be colliding
		param wall: the entity of the wall to check

		Returns True if there is a collision, False if not
	*/
	bool checkWallCollision(Motion& motion, Entity& wall);
	/*
		Assuming a collision has been detected between a motion component and a wall, gets the edge of collision as an enum type
		Also resolves the collision using a helper

		param motion: the motion component which has collided
		param wall: the entity of the wall that it collided with

		returns the edge of collision as an enum type
	*/
	EDGE_TYPE getEdgeOfCollisionAndResolve(Motion& motion, Entity& wall);
	/*
		Resolves a collision after it has been detected
		Uses the edge of collision to determine how to move the given motion so that it is no longer colliding

		param motion: the motion that has collided, which should be moved
		param edge_of_collision: the type of edge that the motion collided on
		param wall_vertex: the position of one of the vertices of the edge of collision
	*/
	void resolveWallCollision(Motion& motion, EDGE_TYPE edge_of_collision, vec2 wall_vertex);

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
		Return true if colliding along with the edge on which the player collided, and false if not

		param player_motion: the motion component of the plaer with position, angle and size
		param player_motion: the current motion of the player
		param wall: the wall entity that we want to test collision for

		return: True + wall edge of collision if collided, False + empty edge if not
	*/
	EDGE_TYPE checkAndHandleWallCollision(Motion& player_motion, Entity& wall);
	/*
		Modifies the given dash so that it doesn't collide into the given wall edge

		param wall_edge: the type of edge of a wall that the player is dashing into
		param dash: the dash to modify
	*/
	void handleDashOnWallEdge(EDGE_TYPE wall_edge, Dashing& dash);

};