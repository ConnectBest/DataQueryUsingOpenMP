#pragma once

#include "common/query_result.hpp"
#include "phase1/aos_types.hpp"

QueryResult query_tips_gt_aos_openmp(const TripTableAoS& t, double threshold);
QueryResult query_fare_gt_aos_openmp(const TripTableAoS& t, double threshold);
QueryResult query_miles_gt_aos_openmp(const TripTableAoS& t, double threshold);
