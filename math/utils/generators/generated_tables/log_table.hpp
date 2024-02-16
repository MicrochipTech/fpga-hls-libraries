#pragma once

namespace hls{
namespace math{
const int LOG_TABLE_SIZE = 16;
const hls::ap_ufixpt<20, 1> logTable[LOG_TABLE_SIZE] = {
1,
0.584963,
0.321928,
0.169925,
0.0874628,
0.0443941,
0.0223678,
0.0112273,
0.00562455,
0.00281502,
0.00140819,
0.000704269,
0.000352177,
0.000176099,
8.80524e-05,
4.40269e-05};

}}
