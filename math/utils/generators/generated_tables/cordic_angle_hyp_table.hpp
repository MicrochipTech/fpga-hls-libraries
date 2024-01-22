#pragma once

namespace hls{
namespace math{
const int HYP_TABLE_SIZE = 16;
  hls::ap_ufixpt<20, 0> cordicHypTable[HYP_TABLE_SIZE] = {
1,
0.549306,
0.255413,
0.125657,
0.0625816,
0.0312602,
0.0156263,
0.00781266,
0.00390627,
0.00195313,
0.000976563,
0.000488281,
0.000244141,
0.00012207,
6.10352e-05,
3.05176e-05};

}}
