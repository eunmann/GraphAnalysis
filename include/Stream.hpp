#pragma once

#include <vector>

namespace STREAM {

	double mysecond();
	void checkSTREAMresults();
	int checktick();
	std::vector<double> run(bool use_pmem);
}