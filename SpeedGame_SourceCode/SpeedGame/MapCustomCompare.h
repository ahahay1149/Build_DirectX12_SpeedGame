#pragma once

#include <sstream>

struct ComponentCustomCompare
{
	bool operator()(const std::string& a, const std::string& b) const
	{
		size_t pos_a = a.find(":");
		size_t pos_b = b.find(":");
		std::string prefix_a = a.substr(0, pos_a);
		std::string prefix_b = b.substr(0, pos_b);
		if (prefix_a == prefix_b && pos_a != std::string::npos && pos_b != std::string::npos)
		{
			std::istringstream iss_a(a.substr(pos_a + 1));
			std::istringstream iss_b(b.substr(pos_b + 1));
			int num_a, num_b;
			iss_a >> num_a;
			iss_b >> num_b;
			return num_a < num_b;
		}
		else
		{
			return a < b;
		}
	}
};