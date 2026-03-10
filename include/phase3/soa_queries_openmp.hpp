#pragma once

#include "common/query_result.hpp"
#include "phase3/soa_types.hpp"

QueryResult query_tips_gt_soa_openmp(const TripTableSoA& t, double threshold);
QueryResult query_fare_gt_soa_openmp(const TripTableSoA& t, double threshold);
QueryResult query_miles_gt_soa_openmp(const TripTableSoA& t, double threshold);
