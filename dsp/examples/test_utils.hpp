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
#ifdef linux
       mkdir(dir_name, 0700);
#endif

#ifdef _WIN32
	_mkdir(dir_name);
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

std::string find_test_signal(int test_no) {
    switch(test_no) {
       case 1: return "1000 * cos(0.5pi*x)";
       case 2: return "1000 * random.normal(0, 0.1, 256)";
       case 3: return "1000 * (sin(8pi*x) + cos(3pi*x))";
       case 4: return "1000 * sin(2pi*x)";
       case 5: return "1000 * cos(30pi*x) * cos(8pi*x)";
       case 6: return "1000 * (3 * sin(2pi*x) + sin(40pi*x) + 0.5 * sin(20pi*x))";
       case 7: return "0";
       case 8: return "1000 * x";
       case 9: return "1000 * cos(300pi*x)";
       case 10: return "1000 * random.normal(0, 0.5, 256)";
    }
    // should never occur
    return "";
}


std::string find_test_name(const char* function, std::string signal_name, int test_no,
                           unsigned int size, unsigned int radix, 
                           std::string direction, std::string streaming) {
   std::string test_name = function;
   test_name.insert(test_name.length(), "_reports/");
   
   // This is something like "fft"
   test_name.insert(test_name.length(), function);

   test_name.insert(test_name.length(), "_");
   test_name.insert(test_name.length(), signal_name);

   test_name.insert(test_name.length(), "_S");
   test_name.insert(test_name.length(), std::to_string(size));  

   test_name.insert(test_name.length(), "_R");
   test_name.insert(test_name.length(), std::to_string(radix));  

  test_name.insert(test_name.length(), "_");
  test_name.insert(test_name.length(), direction);

    test_name.insert(test_name.length(), "_");
  test_name.insert(test_name.length(), streaming);

  test_name.insert(test_name.length(), ".dat");
  return test_name;
}

