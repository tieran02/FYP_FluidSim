#include <math/Plane.h>
#include "PlaneTests.h"
#include <cassert>
#include <util/Log.h>
#include <math/Transform.h>
#include <gtx/string_cast.hpp>

void PlaneTests::NormalVectorTest()
{
	Plane plane
	{
		glm::vec3{0,0,0},
		glm::vec3{0,0,-1}
	};

	auto calculatedValue = plane.GetNormal();
	auto actualValue = glm::vec3{0,0,-1};
	auto equals = glm::all(glm::equal(calculatedValue, actualValue, glm::epsilon<float>()));
	CORE_ASSERT(equals, "NormalVectorTest Failed");

	Transform transform;
	transform.SetRotation(glm::vec3(1,0,0), glm::radians(90.0f));
	auto transformedPlane = plane.TransformedPlane(transform);
	calculatedValue = transformedPlane.GetNormal();
	actualValue = glm::vec3{0,1,0};
	equals = glm::all(glm::equal(calculatedValue, actualValue, glm::epsilon<float>()));
	CORE_ASSERT(equals, "Rotated NormalVectorTest Failed");


	transform = Transform();
	transform.SetRotation(glm::vec3(0,1,0), glm::radians(180.0f));
	transformedPlane = plane.TransformedPlane(transform);
	calculatedValue = transformedPlane.GetNormal();
	actualValue = glm::vec3{0,0,1};
	equals = glm::all(glm::equal(calculatedValue, actualValue, glm::epsilon<float>()));
	CORE_ASSERT(equals, "Rotated NormalVectorTest Failed");

	transform = Transform();
	transform.SetRotation(glm::vec3(0,1,0), glm::radians(270.0f));
	transformedPlane = plane.TransformedPlane(transform);
	calculatedValue = transformedPlane.GetNormal();
	actualValue = glm::vec3{1,0,0};
	equals = glm::all(glm::equal(calculatedValue, actualValue, 0.000001f));
	CORE_ASSERT(equals, "Rotated NormalVectorTest Failed");

	transform = Transform();
	transform.SetRotation(glm::vec3(0,1,0), glm::radians(90.0f));
	transformedPlane = plane.TransformedPlane(transform);
	calculatedValue = transformedPlane.GetNormal();
	actualValue = glm::vec3{-1,0,0};
	equals = glm::all(glm::equal(calculatedValue, actualValue, 0.000001f));
	CORE_ASSERT(equals, "Rotated NormalVectorTest Failed");


	transform = Transform();
	transform.SetPosition(glm::vec3(0,5,-5));
	transform.SetRotation(glm::vec3(1,0,0), glm::radians(90.0f));
	transformedPlane = plane.TransformedPlane(transform);
	calculatedValue = transformedPlane.GetNormal();
	actualValue = glm::vec3{0,1,0};
	equals = glm::all(glm::equal(calculatedValue, actualValue, glm::epsilon<float>()));
	CORE_ASSERT(equals, "Rotated NormalVectorTest Failed");


	transform = Transform();
	transform.SetPosition(glm::vec3(0,5,-5));
	transform.SetRotation(glm::vec3(0,1,0), glm::radians(180.0f));
	transformedPlane = plane.TransformedPlane(transform);

	glm::vec3 linePoint(0,5,1);
	glm::vec3 lineEndPoint(0,0,2);
	float distance = 0.0f;
	bool intersected = transformedPlane.LineIntersection(linePoint,lineEndPoint,distance);
	CORE_ASSERT(intersected == false, "Rotated NormalVectorTest Failed");


	transform = Transform();
	transform.SetPosition(glm::vec3(0,5,-5));
	transform.SetRotation(glm::vec3(0,1,0), glm::radians(180.0f));
	transformedPlane = plane.TransformedPlane(transform);

	linePoint = glm::vec3(0,5,0);
	lineEndPoint = glm::vec3(0,0,-2.5f);
	distance = 0.0f;
	intersected = transformedPlane.LineIntersection(linePoint,lineEndPoint,distance);
	CORE_ASSERT(intersected == false, "Rotated NormalVectorTest Failed");

	transform = Transform();
	transform.SetPosition(glm::vec3(0,0,-5));
	transform.SetRotation(glm::vec3(0,1,0), glm::radians(180.0f));
	transformedPlane = plane.TransformedPlane(transform);

	linePoint = glm::vec3(0,0,0);
	lineEndPoint = glm::vec3(0,0,-5.5f);
	distance = 0.0f;
	intersected = transformedPlane.LineIntersection(linePoint,lineEndPoint,distance);
	CORE_ASSERT(intersected == true, "Rotated NormalVectorTest Failed");

	transform = Transform();
	transform.SetPosition(glm::vec3(0,5,-5));
	transform.SetRotation(glm::vec3(0,1,0), glm::radians(180.0f));
	transformedPlane = plane.TransformedPlane(transform);

	linePoint = glm::vec3(0,5,-1);
	lineEndPoint = glm::vec3(0,0,-3.0f);
	distance = 0.0f;
	intersected = transformedPlane.LineIntersection(linePoint,lineEndPoint,distance);
	CORE_ASSERT(intersected == false, "Rotated NormalVectorTest Failed");

	transform = Transform();
	transform.SetPosition(glm::vec3(0,5,-5));
	transform.SetRotation(glm::vec3(0,1,0), glm::radians(180.0f));
	transformedPlane = plane.TransformedPlane(transform);

	linePoint = glm::vec3(0,5,-1);
	lineEndPoint = glm::vec3(0,0,-5.0f);
	distance = 0.0f;
	intersected = transformedPlane.LineIntersection(linePoint,lineEndPoint,distance);
	CORE_ASSERT(intersected == true, "Rotated NormalVectorTest Failed");

	transform = Transform();
	transform.SetPosition(glm::vec3(0,5,-5));
	transform.SetRotation(glm::vec3(0,1,0), glm::radians(180.0f));
	transformedPlane = plane.TransformedPlane(transform);

	linePoint = glm::vec3(0,5,2);
	lineEndPoint = glm::vec3(0,0,-5.0f);
	distance = 0.0f;
	intersected = transformedPlane.LineIntersection(linePoint,lineEndPoint,distance);
	CORE_ASSERT(intersected == false, "Rotated NormalVectorTest Failed");

	transform = Transform();
	transform.SetPosition(glm::vec3(0,5,-5));
	transform.SetRotation(glm::vec3(0,1,0), glm::radians(180.0f));
	transformedPlane = plane.TransformedPlane(transform);

	linePoint = glm::vec3(0,5,2);
	lineEndPoint = glm::vec3(0,0,-8.0f);
	distance = 0.0f;
	intersected = transformedPlane.LineIntersection(linePoint,lineEndPoint,distance);
	CORE_ASSERT(intersected == true, "Rotated NormalVectorTest Failed");

	transform = Transform();
	transform.SetPosition(glm::vec3(0,5,-5));
	transform.SetRotation(glm::vec3(0,1,0), glm::radians(180.0f));
	transformedPlane = plane.TransformedPlane(transform);

	linePoint = glm::vec3(0,5,-1);
	lineEndPoint = glm::vec3(0,-9.81,-2.0f);
	distance = 0.0f;
	intersected = transformedPlane.LineIntersection(linePoint,lineEndPoint,distance);
	CORE_ASSERT(intersected == false, "Rotated NormalVectorTest Failed");

}
