#pragma once

#define FOR_START(type, start, finish, counter)\
tbb::parallel_for(tbb::blocked_range<type>(start, finish), [&](const tbb::blocked_range<type>& range) {\
    for (type counter = range.begin(); counter != range.end(); ++counter)

#define FOR_END });
#define ACCELERATED_SORT tbb::parallel_sort