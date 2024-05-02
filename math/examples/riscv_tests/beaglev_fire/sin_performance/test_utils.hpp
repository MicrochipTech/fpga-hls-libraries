#pragma once
#include <string>
#include <cstdio>
#include <iostream>
#include <cmath>
#include "include/hls_common.hpp"
using hls::ap_fixpt;
using hls::ap_ufixpt;
using hls::ap_int;
using hls::ap_uint;
#include <sys/time.h>

double timestamp() {
    struct timeval Tp;
    int stat = gettimeofday (&Tp, NULL);
    if (stat != 0)
        printf ("Error return from gettimeofday: %d", stat);
    return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}