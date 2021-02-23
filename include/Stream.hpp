#pragma once

/**
 * WARNING: You can only use this benchmark once per program lifetime.
 * There are static varibles that persist between runs, and will aggregate
 * time elapsed for the tests, affectly ruining the results for anything
 * after the first run.
 */
void run_stream(bool use_pmem);