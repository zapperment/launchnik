#pragma once

// A minimal check macro shared by the command-line tests.

#include <cstdio>

namespace {

int gFailures = 0;
int gChecks = 0;

#define CHECK(condition) \
	do { \
		++gChecks; \
		if (!(condition)) { \
			++gFailures; \
			std::printf("FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		} \
	} while (false)

int ReportChecks(const char* iName) {
	std::printf("%s: %d checks, %d failed\n", iName, gChecks, gFailures);
	return gFailures == 0 ? 0 : 1;
}

} // namespace
