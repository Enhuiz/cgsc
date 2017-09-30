#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include "gtest/gtest.h"
#include "cgsc/model/polygon.h"
#include "cgsc/model/grid.h"
#include "cgsc/model/aoi.h"
#include "cgsc/model/scene.h"
#include "cgsc/solver/greedy.h"
#include "cgsc/utils/data.h"
#include "cgsc/utils/result.h"
#include "cgsc/utils/timestamp.h"

#include "misc.hpp"

using namespace cgsc::model;
using namespace cgsc::test;
using namespace cgsc::utils;
using namespace cgsc::solver;

using namespace std;

TEST(AOI, area)
{
	auto vertices = parseListOf<Point>("[[0, 0], [1, 0], [1, 1], [0, 1]]");
	for (int i = 0; i < vertices.size(); i += 1)
	{
		vertices = roll(vertices, i);
		AOI aoi(vertices);
		EXPECT_EQ(aoi.getArea(), 1);
	}
}

TEST(Scene, area)
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
	list<shared_ptr<Grid>> U1;
	list<shared_ptr<Grid>> U2;
	for (int i = 0; i < 10; ++i)
	{
		U1.push_back(make_shared<Grid>(i, i, 10.5));
		if ((i & 1) == 0)
		{
			U2.push_back(make_shared<Grid>(i, i, 10.5));
		}
	}

	auto oldU1 = U1;
	U1.clear();

	set_difference(
		oldU1.begin(),
		oldU1.end(),
		U2.begin(),
		U2.end(),
		back_inserter(U1),
		[](const shared_ptr<Grid> &a,
		   const shared_ptr<Grid> &b) {
			return *a < *b;
		});

	for (const auto &grid : U2)
	{
		cout << *grid << endl;
	}

	for (const auto &grid : U1)
	{
		cout << *grid << endl;
	}
}
