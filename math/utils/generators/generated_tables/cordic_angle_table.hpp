#pragma once

namespace hls{
namespace math{
const int TABLE_SIZE = 13;
hls::ap_ufixpt<20, 0> cordicTable[TABLE_SIZE] = {
0.785398,
0.463648,
0.244979,
0.124355,
0.0624188,
0.0312398,
0.0156237,
0.00781234,
0.00390623,
0.00195312,
0.000976562,
0.000488281,
0.000244141};

}}
