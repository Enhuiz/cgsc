#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include "csv.hpp"
#include "gtest/gtest.h"
#include "cgsc/geometry/region.hpp"

using namespace cgsc::model;
// using namespace cgsc::solver;

template <class T>
std::vector<T> roll(const std::vector<T> &v, int n)
{
	std::vector<T> ret;
	n = n < 0 ? v.size() + n : n;

	for (int i = v.size() - n; i < v.size(); ++i)
	{
		ret.push_back(v[i]);
	}

	for (int i = 0; i < v.size() - n; ++i)
	{
		ret.push_back(v[i]);
	}

	return ret;
}

std::string to_string(const std::vector<Point> &v)
{
	std::string ret;
	std::ostringstream oss(ret);

	oss << "[";
	for (int i = 0; i < v.size(); ++i)
	{
		const auto &p = v[i];
		if (i)
		{
			oss << ", ";
		}
		oss << "[" << p.x() << ", " << p.y() << "]";
	}
	oss << "]";

	return oss.str();
}

TEST(Region, area1)
{
	auto vertices = parsePoints("[[0, 0], [1, 0], [1, 1], [0, 1]]");
	for (int i = 0; i < vertices.size(); i += 1)
	{
		vertices = roll(vertices, i);
		std::cout << to_string(vertices) << std::endl;
		Region region(vertices);
		EXPECT_EQ(region.getArea(), 1);
	}
}

TEST(Region, area2)
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
		std::cout << to_string(vertices) << std::endl;
		Region region(vertices);
		EXPECT_DOUBLE_EQ(region.getArea(), 2.59807621135332);
	}
}

TEST(Region, ostream)
{
	std::string s = "[[0, 0], [1, 0], [1, 1], [0, 1]]";
	Region region(s);
	EXPECT_EQ(region.to_string(), s);
}
