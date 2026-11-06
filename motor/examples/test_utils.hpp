#pragma once
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <string>
#include <cstdio>
#include <iostream>
#include <sys/time.h>

/***
 * @function create_dir
 * Creates directory if it doesn't already exist.
 */
static void create_dir(const char* dir_name){

   struct stat exists = {0};

   if (stat(dir_name, &exists) == -1) {
#ifdef _WIN32
    mkdir(dir_name);
#else
    mkdir(dir_name, 0700);
#endif
   }
}

double timestamp() {
    struct timeval Tp;
    int stat = gettimeofday (&Tp, NULL);
    if (stat != 0)
      printf ("Error return from gettimeofday: %d", stat);
    return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}
std::string find_test_name(const char* function, unsigned int W, int IW, int N_ITER = -1){
  std::string test_name = function;
  test_name.insert(test_name.length(), "_reports/");
  test_name.insert(test_name.length(), function);

  test_name.insert(test_name.length(), "_D");
  test_name.insert(test_name.length(), std::to_string(W));

  test_name.insert(test_name.length(), "_I");
  test_name.insert(test_name.length(), std::to_string(IW));

  if (N_ITER != -1){
    test_name.insert(test_name.length(), "_N");
    test_name.insert(test_name.length(), std::to_string(N_ITER));
  }

  test_name.insert(test_name.length(), ".dat");
  return test_name;
}
