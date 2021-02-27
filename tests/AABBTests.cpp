#include <glm.hpp>
#include <math/AABB.h>
#include <util/Log.h>
#include "AABBTests.h"

void AABBTests::IntersectionTests()
{
	glm::vec3 min{-5,-5,-5};
	glm::vec3 max{5,5,5};
	glm::vec3 centerPoint{0,0,0};
	glm::vec3 insidePoint{3,1.5f,2};
	glm::vec3 insidePoint1{4,4.9f,0.0f};
	glm::vec3 insidePoint2{4,3.0f,0.0f};
	glm::vec3 outsidePoint{6,1.5f,3.0f};
	glm::vec3 outsidePoint1{0,6.5f,0};
	glm::vec3 minCorner = min + 0.1f;

	AABB box{min,max};

	//inside/outsie AABB test
	CORE_ASSERT(box.IsPointInside(centerPoint) == true, "point inside AABB  Failed");
	CORE_ASSERT(box.IsPointOutside(centerPoint) == false, "point outsude AABB  Failed");
	CORE_ASSERT(box.IsPointInside(insidePoint) == true, "point inside AABB  Failed");
	CORE_ASSERT(box.IsPointOutside(insidePoint) == false, "point outsude AABB  Failed");
	CORE_ASSERT(box.IsPointInside(outsidePoint) == false, "point inside AABB  Failed");
	CORE_ASSERT(box.IsPointOutside(outsidePoint) == true, "point outsude AABB  Failed");

	//inside intersections tests
	auto points = box.Intersection(centerPoint, glm::vec3(0,0,4));
	CORE_ASSERT(points.empty(), "point intersection AABB  Failed");

	points = box.Intersection(centerPoint, glm::vec3(0,0,6));
	CORE_ASSERT(points.size() == 1, "point intersection AABB  Failed");

	points = box.Intersection(insidePoint, glm::vec3(0,0,6));
	CORE_ASSERT(points.size() == 1, "point intersection AABB  Failed");

	//outside intersections
	points = box.Intersection(outsidePoint, glm::vec3(0,0,6));
	CORE_ASSERT(points.size() == 0, "point intersection AABB  Failed");

	points = box.Intersection(outsidePoint, glm::vec3(-2,2.25f,1));
	CORE_ASSERT(points.size() == 1, "point intersection AABB  Failed");

	points = box.Intersection(outsidePoint, glm::vec3(-0.5,0,0));
	CORE_ASSERT(points.size() == 0, "point intersection AABB  Failed");

	points = box.Intersection(outsidePoint, glm::vec3(-20,0,0));
	CORE_ASSERT(points.size() == 2, "point intersection AABB  Failed");

	points = box.Intersection(outsidePoint1, glm::vec3(0.0f,-1.5f,0.0f));
	CORE_ASSERT(points.size() == 1, "point intersection AABB  Failed");

	//closest point
	auto closestPoint = box.GetClosestPoint(outsidePoint, true);
	auto actualValue = glm::vec3{5.0f,1.5f,3.0f};
	auto equals = glm::all(glm::equal(closestPoint.first, actualValue, glm::epsilon<float>()));
	CORE_ASSERT(equals, "closest point AABB  Failed");

	closestPoint = box.GetClosestPoint(insidePoint, true);
	actualValue = glm::vec3{5.0f,1.5f,2.0f};
	equals = glm::all(glm::equal(closestPoint.first, actualValue, glm::epsilon<float>()));
	CORE_ASSERT(equals, "closest point AABB  Failed");

	closestPoint = box.GetClosestPoint(insidePoint1, true);
	actualValue = glm::vec3{4.0f,5.0f,0.0f};
	equals = glm::all(glm::equal(closestPoint.first, actualValue, glm::epsilon<float>()));
	CORE_ASSERT(equals, "closest point AABB  Failed");

	//sphere tests
	CORE_ASSERT(box.IsSphereOutside(insidePoint1, 0.01) == false, "sphere point AABB  Failed");
	CORE_ASSERT(box.IsSphereOutside(insidePoint1, 0.1) == true, "sphere point AABB  Failed");

	CORE_ASSERT(box.IsSphereOutside(outsidePoint, 0.1) == true, "sphere point AABB  Failed");
	CORE_ASSERT(box.IsSphereOutside(outsidePoint, 0.1) == true, "sphere point AABB  Failed");

	CORE_ASSERT(box.IsSphereInside(minCorner, 0.1) == true, "sphere point AABB  Failed");
	CORE_ASSERT(box.IsSphereInside(minCorner, 0.15) == false, "sphere point AABB  Failed");
}
