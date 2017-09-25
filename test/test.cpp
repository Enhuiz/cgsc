#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include "gtest/gtest.h"
#include "cgsc/model/polygon.h"
#include "cgsc/model/grid.h"
#include "cgsc/model/aoi.h"
#include "cgsc/model/scene.h"
#include "cgsc/solver/data.h"
#include "cgsc/solver/greedy.h"

#include "utils.hpp"

using namespace cgsc::model;
using namespace cgsc::test;

using namespace cgsc::solver;

using namespace std;

TEST(Polygon, tostring)
{
	EXPECT_EQ(Polygon("[[0, 0], [1, 0], [1, 1], [0, 1]]").to_string(), "[[0, 0], [1, 0], [1, 1], [0, 1]]");
}

TEST(AOI, getArea)
{
	auto vertices = parseListOf<Point>("[[0, 0], [1, 0], [1, 1], [0, 1]]");
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

	auto vertices = parseListOf<Point>(s);

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

shared_ptr<Data> data;

TEST(Data, load)
{
	data = make_shared<Data>("../../data/scenes_small.csv", "../../data/aois.csv");
	Greedy greedy(data);
}

shared_ptr<Greedy> greedy;
TEST(Data, greedy)
{
	greedy = make_shared<Greedy>(data);
}

TEST(Data, calculate_result)
{
	auto results = greedy->calculateResults();
}