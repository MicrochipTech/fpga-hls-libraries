#include <hls/data_buffer.hpp>
#include "hls/ap_int.hpp"
#include "hls/ap_fixpt.hpp"
#include "hls/streaming.hpp"
#include "hls/hls_alloc.h"
#include <stdio.h>
#include <cstdlib>
#include "hls_fft.hpp"

using namespace hls;
using namespace hls::dsp;

#include "test_vector.h"
#include "test_utils.hpp"

struct fft_buff_t {
  uint16_t im;
  uint16_t re;
};


#define FFT_SIZE                    256
#define ERR_RATE                    0.05
#define SIGNAL_SCALE                1000
#define VAL_MISMATCH_THRESHHOLD     (ERR_RATE * SIGNAL_SCALE)


/***
 * @function read_fifo
 * Read data from fifo to the destination buffer with a given size.
 * 
 * @param {FIFO<fft_data_t>} in_fifo: input fifo
 * @param {fft_data_t} buf: the destination buffer
 * @template {unsigned} size: the size of the destination buffer and the input fifo depth
 * @return void
 */
template <unsigned SIZE> 
void read_fifo(FIFO<fft_data_t> &in_fifo, fft_buff_t *buf) {
	#pragma HLS loop pipeline
    for(unsigned i=0; i<SIZE; i++) {
        fft_data_t tmp = in_fifo.read();
        buf[i].re = int(tmp.re);
        buf[i].im = int(tmp.im);
    }
}

/***
 * @function write_fifo
 * write data from the source buffer to the destination fifo with a given size.
 * 
 * @param {fft_data_t} buf: the source buffer
 * @param {FIFO<fft_data_t>} out_fifo: output fifo
 * @template {unsigned} size: the size of the source buffer and the output fifo depth
 * @return void
 */
template <unsigned SIZE> 
void write_fifo(fft_buff_t *buf, FIFO<fft_data_t> &out_fifo) {
	#pragma HLS loop pipeline
    for(unsigned i=0; i<SIZE; i++) {
        fft_data_t tmp;
        tmp.re = buf[i].re;
        tmp.im = buf[i].im;
    	out_fifo.write(tmp);
    }
}

/***
 * @function fft_wrapper
 * The FFT wrapper that calls the FFT library in the SoC flow. The top-level
 * arguments are buffers configured as either AXI target or AXI intiator types.
 * 
 * @template {unsigned int} SIZE: the FFT transform size
 * @param {uint32_t} buf_in: the input buffer of size 256
 * @param {uint32_t} buf_out: the output buffer of size 256
 * @return void
 */
template <unsigned SIZE> 
void fft_wrapper(uint32_t *buf_in, uint32_t *buf_out) {

  #pragma HLS function top
  #pragma HLS interface default type(axi_target)
  #if defined(AXI_TARGET)
  #pragma HLS interface argument(buf_in) type(axi_target) num_elements(SIZE) dma(false)
  #pragma HLS interface argument(buf_out) type(axi_target) num_elements(SIZE) dma(false)
  #else
  #pragma HLS interface argument(buf_in) type(axi_initiator) num_elements(SIZE) max_burst_len(SIZE) ptr_addr_interface(axi_target)
  #pragma HLS interface argument(buf_out) type(axi_initiator) num_elements(SIZE) max_burst_len(SIZE) ptr_addr_interface(axi_target)
  #endif

    FIFO<fft_data_t> fifo_in(SIZE);
	FIFO<fft_data_t> fifo_out(SIZE);

    fft_buff_t *buf_in_ptr = (fft_buff_t *)buf_in;
    fft_buff_t *buf_out_ptr = (fft_buff_t *)buf_out;
    write_fifo<SIZE>(buf_in_ptr, fifo_in);

    fft<SIZE>(fifo_in, fifo_out);

    read_fifo<SIZE>(fifo_out, buf_out_ptr);

}

int main()
{
    ap_int<16> outRe[FFT_SIZE], outIm[FFT_SIZE];
    ap_int<16> refRe[FFT_SIZE], refIm[FFT_SIZE];

    fft_buff_t *buf_in  = (fft_buff_t *)hls_malloc(FFT_SIZE * sizeof(fft_buff_t), DDR_REGION);
    fft_buff_t *buf_out  = (fft_buff_t *)hls_malloc(FFT_SIZE * sizeof(fft_buff_t), DDR_REGION);
    int err = 0;

     for (int i=0; i<FFT_SIZE; i++){
        buf_in[i].im = (int)testPatternIm[i];
        buf_in[i].re =  (int)testPatternRe[i];
    }

    uint32_t *buf_in_ptr = (uint32_t*)buf_in;
    uint32_t *buf_out_ptr = (uint32_t*)buf_out;

    double fft_start = timestamp();
    fft_wrapper<FFT_SIZE>(buf_in_ptr, buf_out_ptr);
    double fft_end = timestamp();

    // read from buf_out
    for (int i=0; i<FFT_SIZE; i++){
        outRe[i] = buf_out[i].re;
        outIm[i] = buf_out[i].im;
        refRe[i] = testResGoldRe[i];
        refIm[i] = testResGoldIm[i];
    }

    // value mismatch check
    for (int i = 0; i < FFT_SIZE; i++) {
        if (abs((int)(refRe[i] - outRe[i])) > VAL_MISMATCH_THRESHHOLD) {
            err++;
            printf("Mismatch: outRe[%d] expected != actual: %d != %d\n", i, int(refRe[i]), int(outRe[i]));
        }
        if (abs((int)(refIm[i] - outIm[i])) > VAL_MISMATCH_THRESHHOLD) {
            err++;
            printf("Mismatch outIm[%d] expected != actual: %d != %d\n", i, int(refIm[i]), int(outIm[i]));
        }
    }
    
    hls_free(buf_in);
    hls_free(buf_out);

    double fft_duration = fft_end - fft_start;
    printf("Time: hls_fft: %lf s\n", fft_duration);

    if(!err) {
        printf("PASS\n");
        return 0;
    } else {
        printf("FAIL\n");
        return 1;
    }
}
