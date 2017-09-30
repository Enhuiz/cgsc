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

bool operator<(const shared_ptr<Grid> &a, const shared_ptr<Grid> &b)
{
    cout << "p < t" << endl;
    return *a < *b;
}


bool operator<(const shared_ptr<const Grid> &a, const shared_ptr<const Grid> &b)
{
    cout << "const p < t" << endl;
    return *a < *b;
}

TEST(Grid, equal)
{
	list<shared_ptr<Grid>> U1;
	list<shared_ptr<Grid>> U2;

	for (int i = 0; i < 10; ++i)
	{
		// U1.push_back(make_shared<Grid>(1, 1, 1));
		if ((i & 1) == 0)
		{
			U1.push_back(make_shared<Grid>(6284, 2307, 0.020));
			U2.push_back(make_shared<Grid>(6284, 2307, 0.020));
			// U2.push_back(make_shared<Grid>(1, 1, 1));
		} else {
			U1.push_back(make_shared<Grid>(2, 5, 0));
		}
	}

	set<shared_ptr<Grid>> U;

	set_difference(
		U1.begin(),
		U1.end(),
		U2.begin(),
		U2.end(),
		inserter(U, U.begin()));

	EXPECT_EQ(U.size(), 1);

	for (const auto &grid : U)
	{
		cout << *grid << endl;
	}
}
