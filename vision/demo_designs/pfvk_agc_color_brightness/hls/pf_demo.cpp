#include "vision.hpp"
#include "hls/utils.hpp"
#include <assert.h>
#include <opencv2/opencv.hpp>

using namespace hls;

// For testing with cosim
// #define SMALL

#ifdef SMALL
#warning "Runnning with SMALL image"
#define WIDTH 100
#define HEIGHT 56
#define FILENAME "../../../media_files/toronto_100x56.bmp"
#define GOLDEN_FILENAME "hls_out_golden_small.png"
#define REF_SUM 1077558
#else
#define WIDTH 3840
#define HEIGHT 2160
#define FILENAME "../../../media_files/toronto_4k.jpg"
#define GOLDEN_FILENAME "hls_out_golden_large.png"
#define REF_SUM 1590725037
#endif

using vision::NPPC_4;
using vision::StorageType;
using vision::PixelType::HLS_8UC1;
using vision::PixelType::HLS_8UC3;

#define NumPixels (WIDTH * HEIGHT)
#define NumPixelWords (NumPixels / NPPC_4)
#define PixelWordWidth (8 * NPPC_4) // Image is 8UC1 for Bayer Img
#define AxiWordWidth 64           // AXI memory is 64-bit
#define NumAxiWords (NumPixelWords * PixelWordWidth / AxiWordWidth)

using BayerAxisVideoT = vision::AxisVideoFIFO<HLS_8UC1, NPPC_4>;
using RGBAxisVideoT = vision::AxisVideoFIFO<HLS_8UC3, NPPC_4>;
using BayerImgT = vision::Img<HLS_8UC1, HEIGHT, WIDTH, StorageType::FIFO, NPPC_4>;
using BGRImgT = vision::Img<HLS_8UC3, HEIGHT, WIDTH, StorageType::FIFO, NPPC_4>;

//------------------------------------------------------------------------------
// Convert the frame to grayscale  using an approximation of the NTSC formula
// to save some resources:
//  Original:   Y  = 0.299 * R + 0.587 * G + 0.114 * B
//  Approx  :   Y  = 0.25  * R + 0.5   * G + 0.125 * B 
// Multiply by 2
//              2Y = 0.5   * R +         G + 0.25  * B
// Later on, the CPU can divide the average by 2 to eliminate the extra 2 factor
//
void Sum(BGRImgT &InImg, BGRImgT &OutImg, unsigned &sum ) {
    #pragma HLS memory partition argument(InImg) type(struct_fields)
    #pragma HLS memory partition argument(OutImg) type(struct_fields)

    const unsigned PixelWidth = vision::DT<HLS_8UC3, NPPC_4>::W / NPPC_4;
    const unsigned ChannelWidth = vision::DT<HLS_8UC3, NPPC_4>::PerChannelPixelWidth;
    unsigned tmpSum = 0;

    OutImg.set_height(InImg.get_height());
    OutImg.set_width(InImg.get_width());

    vision::TransformPixel_ref(InImg, OutImg, tmpSum, 
        [&](ap_uint<PixelWidth> in, unsigned &tmpSum) {
            ap_uint<ChannelWidth> b = in.byte(2,ChannelWidth);
            ap_uint<ChannelWidth> g = in.byte(1,ChannelWidth);
            ap_uint<ChannelWidth> r = in.byte(0,ChannelWidth);
            tmpSum = tmpSum + r/2 + g + b/4;
            return in;
        }
    );

    // This extra register stage is needed to prevent "sum" to be updated every 
    // cycle because this function is called inside a "dataflow" function.
    sum = hls::hls_reg(tmpSum);
}

//------------------------------------------------------------------------------
void DDR_Write_wrapper(BayerAxisVideoT &VideoIn, uint64_t *Buf, int HRes, int VRes) {
    #pragma HLS function top
    #pragma HLS interface argument(Buf) type(axi_initiator) num_elements(NumAxiWords) max_burst_len(256)
    vision::AxisVideo2AxiMM<AxiWordWidth, uint64_t, HEIGHT, WIDTH>(VideoIn, Buf, HRes, VRes);
}

//------------------------------------------------------------------------------
void VideoPipelineTop(
    uint64_t *Buf, 
    RGBAxisVideoT &VideoOut, 
    ap_uint<8> enable_gamma,
    ap_uint<8> b_const,
    ap_uint<8> g_const,
    ap_uint<8> r_const,
    ap_uint<16> brightness,
    unsigned &sum,
    vision::BayerFormat BayerFormat) {
    #pragma HLS function top
    #pragma HLS function dataflow
    #pragma HLS interface argument(Buf) type(axi_initiator) num_elements(NumAxiWords) max_burst_len(256)
    #pragma HLS interface argument(enable_gamma) type(axi_target)
    #pragma HLS interface argument(b_const)      type(axi_target)
    #pragma HLS interface argument(g_const)      type(axi_target)
    #pragma HLS interface argument(r_const)      type(axi_target)
    #pragma HLS interface argument(brightness)   type(axi_target)
    #pragma HLS interface argument(sum)          type(axi_target)

    BayerImgT BayerImg;
    BGRImgT BGRImg, GammaCorrected, AfterSumImg, AfterGrayImg, ImgEnhanced;

    vision::AxiMM2Img<AxiWordWidth>(Buf, BayerImg);
    vision::DeBayer(BayerImg, BGRImg, BayerFormat);
    Sum(BGRImg, AfterSumImg, sum);
    vision::ImageEnhance(AfterSumImg, ImgEnhanced, b_const, g_const, r_const, brightness);
    vision::GammaCorrection(ImgEnhanced, GammaCorrected, enable_gamma);
    vision::Img2AxisVideo(GammaCorrected, VideoOut);
}

//------------------------------------------------------------------------------
int main() {
    Mat BGRInMat = cv::imread(FILENAME, cv::IMREAD_COLOR);
    BGRImgT InImg;
    BayerImgT BayerInImg;
    BayerAxisVideoT InStream(NumPixelWords), DDRReadFIFO(NumPixelWords);
    convertFromCvMat(BGRInMat, InImg);
    vision::BGR2Bayer(InImg, BayerInImg, vision::BayerFormat::BGGR);
    vision::Img2AxisVideo(BayerInImg, InStream);

    static uint64_t Buf[NumPixelWords];
    RGBAxisVideoT OutputStream(NumPixelWords);
    DDR_Write_wrapper(InStream, Buf, WIDTH, HEIGHT);

    ap_uint<8> b_const = 62;
    ap_uint<8> g_const = 42;
    ap_uint<8> r_const = 52;
    ap_uint<10> brightness = 46;
    ap_uint<1> enable_gamma = 1;
    unsigned sum = 0;
    VideoPipelineTop(
        Buf, 
        OutputStream, 
        enable_gamma, 
        b_const, 
        g_const, 
        r_const, 
        brightness, 
        sum, 
        vision::BayerFormat::BGGR
    );
    printf("sum:%u, avg:%f\n", sum, (float)sum / (2 * HEIGHT * WIDTH));

    BGRImgT OutImg;
    cv::Mat HlsOutMat;
    vision::AxisVideo2Img(OutputStream, OutImg);
    convertToCvMat(OutImg, HlsOutMat);
    cv::imwrite("hls_out.png", HlsOutMat);

    // Check if the output image is identical to the reference.
    auto similarity = cv::norm(
        cv::imread("hls_out.png"), cv::imread(GOLDEN_FILENAME), cv::NORM_INF);
    int error = (similarity != 0);
    error += (sum != REF_SUM);
    printf("%s\n", error ? "FAILED" : "PASSED" );
    return error;
}