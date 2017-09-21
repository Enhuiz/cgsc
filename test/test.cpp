#include <iostream>
#include <string>
#include <vector>
#include <functional>

// #include "csv.hpp"
#include "gtest/gtest.h"
#include "cgsc/geometry/polygon.hpp"
#include "cgsc/geometry/grid.hpp"
#include "cgsc/geometry/aoi.hpp"
#include "cgsc/geometry/scene.hpp"
#include "utils.hpp"

using namespace cgsc::model;
using namespace cgsc::test;


TEST(Polygon, tostring)
{
	EXPECT_EQ(Polygon("[[0, 0], [1, 0], [1, 1], [0, 1]]").to_string(), "[[0, 0], [1, 0], [1, 1], [0, 1]]");
}

TEST(AOI, getArea)
{
	auto vertices = parsePoints("[[0, 0], [1, 0], [1, 1], [0, 1]]");
	for (int i = 0; i < vertices.size(); i += 1)
	{
		vertices = roll(vertices, i);
		AOI aoi(vertices);
		EXPECT_EQ(aoi.getArea(), 1);
	}
}

TEST(Scene, getArea)
{
	std::string s = "[[1, 0], \
					[0.5, 0.8660254037844386],  \
					[-0.5, 0.8660254037844386], \
					[-1, 0], \
					[-0.5, -0.8660254037844386], \
					[0.5, -0.8660254037844386]]";

	auto vertices = parsePoints(s);

	for (int i = 0; i < vertices.size(); i += 1)
	{
		vertices = roll(vertices, i);
		Scene scene(vertices);
		EXPECT_NEAR(scene.getArea(), 2.59807621135332, 1e-10);
	}
}


TEST(Grid, equal)
{
	EXPECT_EQ(Grid(0, 0, 10), Grid(0, 0, 10));
}
