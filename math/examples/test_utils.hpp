#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef linux
#include <sys/stat.h>
#endif

#include <string>
#include <cstdio>
#include <iostream>

#include <cmath>
#include "../include/hls_common.hpp"
using hls::ap_fixpt;
using hls::ap_ufixpt;
using hls::ap_int;
using hls::ap_uint;
#include <sys/time.h>

/***
 * @function create_dir
 * Creates directory if it doesn't already exist.
 */
static void create_dir(const char* dir_name){
#ifdef linux
   struct stat exists = {0};

   if (stat(dir_name, &exists) == -1) {
       mkdir(dir_name, 0700);
   }
#endif

#ifdef _WIN32
	CreateDirectory(dir_name, NULL);
#endif
}

double timestamp() {
    struct timeval Tp;
    int stat = gettimeofday (&Tp, NULL);
    if (stat != 0)
      printf ("Error return from gettimeofday: %d", stat);
    return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}

std::string find_test_name(const char* function, unsigned int W, int IW, double start, double limit, int N_ITER = -1, double base = 0){
  std::string test_name = function;
  test_name.insert(test_name.length(), "_reports/");
  test_name.insert(test_name.length(), function);

  test_name.insert(test_name.length(), "_D");
  test_name.insert(test_name.length(), std::to_string(W));

  test_name.insert(test_name.length(), "_I");
  test_name.insert(test_name.length(), std::to_string(IW));

  test_name.insert(test_name.length(), "_S");
  test_name.insert(test_name.length(), std::to_string(start));

  test_name.insert(test_name.length(), "_L");
  test_name.insert(test_name.length(), std::to_string(limit));

  if (N_ITER != -1){
    test_name.insert(test_name.length(), "_N");
    test_name.insert(test_name.length(), std::to_string(N_ITER));
  }

  if (base != 0){
    test_name.insert(test_name.length(), "_B");
    test_name.insert(test_name.length(), std::to_string(base));
  }

  test_name.insert(test_name.length(), ".dat");
  return test_name;
}

