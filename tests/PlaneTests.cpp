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
		glm::vec3{-5,5,0},
		glm::vec3{0,0,-1}
	};

	auto calculatedValue = plane.GetNormal();
	auto actualValue = glm::vec3{0,0,-1};
	auto equals = glm::all(glm::equal(calculatedValue, actualValue, glm::epsilon<float>()));
	CORE_ASSERT(equals, "NormalVectorTest Failed");

	plane = Plane
	{
		{-10,20,-10},
		{10,20,-10},
		{-10,0,-10}
	};


	calculatedValue = plane.GetNormal();
	actualValue = glm::vec3{0,0,-1};
	equals = glm::all(glm::equal(calculatedValue, actualValue, glm::epsilon<float>()));
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
}
