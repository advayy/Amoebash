#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include <map>
#include <optional>

class CollisionDetector
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
		Gets the direction vector for the given angle
	*/
	vec2 getDirectionVecFromAngle(float angle);
	/*
		Returns true if the given angle is in Quadrant 1 (+x, +y) or Quadrant 3 (-x, -y)
	*/
	bool isQuadrant1Or3(float angle);
	/*
		Clamps the given angle to between 0 and 360, so adds 360 to negative angles
	*/
	float clampNegativeAngle(float angle);
	/*
		For the given wall edge, gets the point from the player vertices that would've intersected that edge
		For every angle of approach, there is one point out of the 4 player vertices that necessarily has to touch the given edge first
		With this, we can verify if the player did actually intersect that edge

		param player_vertices: the 4 vertices of the player rectangle, in this order:
								Top Left, Top Right, Bottom Right, Bottom Left
		param player_angle: angle in degrees that the player is facing
		param direction: the edge we want to test for
	*/
	vec2 getPointOnPlayer(std::vector<vec2> player_vertices, float player_angle, vec2 wall_edge);
	/*
		Check collisions between the given motion and wall

		param motion: the motion component which may be colliding
		param wall: the entity of the wall to check

		Returns a pair containing the vertices and edges of the wall that the motion collided with, if there is a collision
		If not, returns nullopt
	*/
	std::optional<std::pair<std::vector<vec2>, std::vector<vec2>>> checkWallCollision(Motion& motion, Entity& wall);
	/*
		Get the distance to the point of intersection between the 2 lines defined by:
			- Starting at point, and going in direction
			- Starting at point2, and going in direction2

		Returns the absolute distance from the first point to the intersection
	*/
	float getIntersectionDist(vec2 point, vec2 direction, vec2 point2, vec2 direction2);
	/*
		Resolve a collision between a Motion component and a wall. Modifies the motion component to move it out of the wall

		param motion: the motion component which has collided, which should be modified
		param closest_intersection_edge: the direction vector of the edge that the motion intersected with
		param intersection_edge_vertex: the starting vertex of the edge that the motion intersected with
		param movement_angle: the angle that the motion was moving in, relative to the vertical
		param object_angle: the angle that the motion was pointing in, relative to the vertical

	*/
	void resolveWallCollision(Motion& motion, vec2 intersection_edge, vec2 intersection_edge_vertex, float movement_angle, float object_angle_rad = 0);

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
	std::pair<bool, vec2> checkAndHandlePlayerWallCollision(Motion& player_motion, float movement_angle, Entity& wall);

	void checkAndHandleGeneralWallCollision(Motion& motion, Entity& wall);
	/*
		Modifies the given dash so that it doesn't collide into the given wall edge

		param direction2: the edge of a wall that the player is dashing into
		param dash: the dash to modify
	*/
	void handleDashOnWallEdge(vec2 wall_edge, Dashing& dash);

};