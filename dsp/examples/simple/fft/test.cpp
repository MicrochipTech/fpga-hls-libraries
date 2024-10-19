// ©2022 Microchip Technology Inc. and its subsidiaries
//
// Subject to your compliance with these terms, you may use this Microchip
// software and any derivatives exclusively with Microchip products. You are
// responsible for complying with third party license terms applicable to your
// use of third party software (including open source software) that may
// accompany this Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES,
// WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING
// ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR
// A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY
// INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST
// OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED,
// EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#include "hls/ap_int.hpp"
#include "hls/ap_fixpt.hpp"
#include "hls/hls_alloc.h"
#include <stdio.h>
#include <cstdlib>
#include "hls_fft.hpp"
#include "test_utils.hpp"

using namespace hls;
using namespace hls::dsp;

#include "test_vectors/test1_vector.h"
#include "test_vectors/test2_vector.h"
#include "test_vectors/test3_vector.h"
#include "test_vectors/test4_vector.h"
#include "test_vectors/test5_vector.h"
#include "test_vectors/test6_vector.h"
#include "test_vectors/test7_vector.h"
#include "test_vectors/test8_vector.h"
#include "test_vectors/test9_vector.h"
#include "test_vectors/test10_vector.h"

#define FFT_SIZE                    256
#define FIFO_DEPTH                  FFT_SIZE
#define ERR_RATE                    0.05
#define SIGNAL_SCALE                1000
#define VAL_MISMATCH_THRESHHOLD     (ERR_RATE * SIGNAL_SCALE)
#define RE_IDX                      0
#define IM_IDX                      1


/***
 * @function read_fifo
 * Read data from fifo to the destination buffer with a given size.
 * 
 * @param {FIFO<fft_data_in_t>} in_fifo: input fifo
 * @param {fft_data_t} buf: the destination buffer
 * @return void
 */
template <unsigned SIZE> 
void read_fifo(FIFO<fft_data_t> *in_fifo, fft_data_t *buf) {
    for(unsigned i=0; i<SIZE; i++) {
        fft_data_t tmp = in_fifo->read();
        buf[i].re = tmp.re;
        buf[i].im = tmp.im;
    }
}

/***
 * @function write_fifo
 * write data from the source buffer to the destination fifo with a given size.
 * 
 * @param {fft_data_t} buf: the source buffer
 * @param {FIFO<fft_data_in_t>} out_fifo: output fifo
 * @return void
 */
template <unsigned SIZE>
void write_fifo(fft_data_t *buf, FIFO<fft_data_t> *out_fifo) {
    for(unsigned i=0; i<SIZE; i++) {
        fft_data_t tmp;
        tmp.re = buf[i].re;
        tmp.im = buf[i].im;
        out_fifo->write(tmp);
    }
}

/***
 * @function inplace_fft_wrapper
 * The Inplace-FFT wrapper that calls the FFT library in the SoC flow. The top-level
 * arguments are buffers configured as either AXI target or AXI intiator types.
 * 
 * @template {unsigned int} SIZE: the FFT transform size
 * @param {uint32_t} buf_in: the input buffer of size 256
 * @param {uint32_t} buf_out: the output buffer of size 256
 * @return void
 */
template <unsigned SIZE>
void inplace_fft_wrapper(FIFO<fft_data_t> &fifo_in, FIFO<fft_data_t> &fifo_out) {
  #pragma HLS function top
  fft<SIZE>(fifo_in, fifo_out);
}

template <unsigned SIZE> 
void init_buf_in(fft_data_t *buf_in, const uint16_t *testPatternRe, const uint16_t *testPatternIm) {
    for (unsigned i=0; i<SIZE; i++){
        buf_in[i].im = testPatternIm[i];
        buf_in[i].re = testPatternRe[i];
    }
}

template <unsigned SIZE> 
int test(const uint16_t* testResGoldRe, const uint16_t* testResGoldIm, 
        const uint16_t* testPatternRe, const uint16_t* testPatternIm,
        int test_no, unsigned int radix, unsigned int n_points,
        std::string direction, std::string streaming,
        int graph = 0, int report = 0) {

    fft_data_t buf_in[SIZE];
    fft_data_t buf_out[SIZE];

    // 0: re, 1: im
    int max_diff[2] = {0,0};
    float avg_diff[2] = {0,0};
    int errs = 0;
    FIFO<fft_data_t> fifo_out(SIZE), fifo_in(SIZE);
    std::string signal_name = find_test_signal(test_no);
    std::string test_name = find_test_name("fft", signal_name, test_no, n_points, radix, direction, streaming);
    
    assert(!test_name.empty());
    assert(!signal_name.empty());

    FILE* fp = fopen(test_name.c_str(), "w");
    if (fp == NULL) {
        perror("Error: ");
        return -1;
    }

    if (graph)  fprintf(fp, "# PLOT ");
    if (report)  fprintf(fp, "# REPORT ");
    fprintf(fp, "\n");
    fprintf(fp, "# test number: %d\n", test_no);
    fprintf(fp, "# test signal: %s\n", signal_name.c_str());
    fprintf(fp, "# i,sw_re,hls_re,sw_im,hls_im,re_diff,im_diff,normalized_re_diff,normalized_im_diff\n");

    // print to stdout
    printf("Test number: %d\n", test_no);
    printf("Test signal: %s\n", signal_name.c_str());

    // Convert an array of uint16_ts into an array of fft_data_ts
    init_buf_in<SIZE>(buf_in, testPatternRe, testPatternIm);

    // run fft
    write_fifo<SIZE>(buf_in, &fifo_in);
    inplace_fft_wrapper<SIZE>(fifo_in, fifo_out);
    read_fifo<SIZE>(&fifo_out, buf_out);

    // value mismatch check
    for (unsigned i = 0; i < SIZE; i++) {
        // Convert the testResGoldRe/Im values from uint16_t to int, since the types of fft_data_t are 16-bit ints
        int golden_re_int = ((ap_int<16>)testResGoldRe[i]).to_int64();
        int golden_im_int = ((ap_int<16>)testResGoldIm[i]).to_int64();        
        
        // calculate the diff in the real part
        int re_diff = abs(golden_re_int - buf_out[i].re);
        // calculate the diff in the imaginary part
        int im_diff = abs(golden_im_int - buf_out[i].im);

        if (re_diff > VAL_MISMATCH_THRESHHOLD) {
            std::cout << "Mismatch: outRe[" << i << "] expected != actual: " << golden_re_int << " != " << buf_out[i].re.to_int64() << std::endl;
            errs++;
        }
        if (im_diff > VAL_MISMATCH_THRESHHOLD) {
            std::cout << "Mismatch: outIm[" << i << "] expected != actual: " << golden_im_int << " != " << buf_out[i].im.to_int64() << std::endl;
            errs++;
        }

        // since SIGNAL_SCALE is 1000, signal / 1000 * 100 = signal / 10.
        float normalized_re_diff = (float)re_diff / 10;
        avg_diff[RE_IDX] += re_diff;
        max_diff[RE_IDX] = std::max(max_diff[RE_IDX], re_diff);

        float normalized_im_diff = (float)im_diff / 10;
        avg_diff[IM_IDX] += im_diff;
        max_diff[IM_IDX] = std::max(max_diff[IM_IDX], im_diff);

        fprintf(fp, " %d, %d, %d, %d, %d, %d, %d, %.2f, %.2f\n", 
            i, golden_re_int, int(buf_out[i].re), golden_im_int, int(buf_out[i].im), re_diff, im_diff, normalized_re_diff, normalized_im_diff);
    }

    avg_diff[RE_IDX] /= SIZE;
    avg_diff[IM_IDX] /= SIZE;

    fprintf(fp, "# re: Count: %d Max error: %f Avg error: %lf\n", SIZE, (float)max_diff[RE_IDX], avg_diff[RE_IDX]);
    fprintf(fp, "# im: Count: %d Max error: %f Avg error: %lf\n", SIZE, (float)max_diff[IM_IDX], avg_diff[IM_IDX]);

    fclose(fp);

    if(!errs) {
        printf("test passed\n");
    } else {
        printf("test failed\n");
    }

    return errs;
}


int main() {
    //Currently, we only support the radix-2, 256-point, forward inplace FFT implementation.
    unsigned int radix = 2, n_points = FFT_SIZE;
    std::string direction = "forward";
    std::string streaming = "inplace";

    std::string input_func_name; // Look into if the name of the header file can replace this var

    int RC = 0;
    create_dir("fft_reports");

    int test_no = 1; 
   
    // test 1: y = 1000 * cos(0.5πx)
    RC |= test<FFT_SIZE>(test1ResGoldRe, test1ResGoldIm, 
                        test1PatternRe, test1PatternIm,
                        test_no, radix, n_points, "forward", "inplace", 0, 0);

    // test 2: y = 1000 * random.normal(0, 0.1, 256)
    test_no = 2;
    RC |= test<FFT_SIZE>(test2ResGoldRe, test2ResGoldIm, 
                        test2PatternRe, test2PatternIm,
                        test_no, radix, n_points, "forward", "inplace", 0, 0);

    // test 3: y = 1000 * (sin(8πx) + cos(3πx))
    test_no = 3;
    RC |= test<FFT_SIZE>(test3ResGoldRe, test3ResGoldIm, 
                        test3PatternRe, test3PatternIm,
                        test_no, radix, n_points, "forward", "inplace", 1, 0);

    // test 4: y = 1000 * sin(2πx)
    test_no = 4;
    RC |= test<FFT_SIZE>(test4ResGoldRe, test4ResGoldIm, 
                        test4PatternRe, test4PatternIm,
                        test_no, radix, n_points, "forward", "inplace", 1, 1);

    // test 5: y = 1000 * cos(30πx) * cos(8πx)
    test_no = 5;
    RC |= test<FFT_SIZE>(test5ResGoldRe, test5ResGoldIm, 
                        test5PatternRe, test5PatternIm,
                        test_no, radix, n_points, "forward", "inplace", 0, 0);

    // test 6: y = 1000 * (3 * sin(2πx) + sin(40πx) + 0.5 * sin(20πx))
    test_no = 6;
    RC |= test<FFT_SIZE>(test6ResGoldRe, test6ResGoldIm, 
                        test6PatternRe, test6PatternIm,
                        test_no, radix, n_points, "forward", "inplace", 0, 0);

    // test 7: y = 0
    test_no = 7;
    RC |= test<FFT_SIZE>(test7ResGoldRe, test7ResGoldIm, 
                        test7PatternRe, test7PatternIm,
                        test_no, radix, n_points, "forward", "inplace", 0, 0);

    // test 8: y = 1000 * x
    test_no = 8;
    RC |= test<FFT_SIZE>(test8ResGoldRe, test8ResGoldIm, 
                        test8PatternRe, test8PatternIm,
                        test_no, radix, n_points, "forward", "inplace", 0, 0);

    // test 9: y = 1000 * cos(300πx)
    test_no = 9;
    RC |= test<FFT_SIZE>(test9ResGoldRe, test9ResGoldIm, 
                        test9PatternRe, test9PatternIm,
                        test_no, radix, n_points, "forward", "inplace", 0, 0);

    // test 10: y = 1000 * random.normal(0, 0.5, 256)
    test_no = 10;
    RC |= test<FFT_SIZE>(test10ResGoldRe, test10ResGoldIm, 
                        test10PatternRe, test10PatternIm,
                        test_no, radix, n_points, "forward", "inplace", 1, 0);

    if (RC != 0)  printf("Errors may have occurred. Please double-check usage.\n");
    return RC;
}