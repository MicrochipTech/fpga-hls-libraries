// ©2022 Microchip Technology Inc. and its subsidiaries
//
// Subject to your compliance with these terms, you may use this
// Microchip software and any derivatives exclusively with Microchip
// products. You are responsible for complying with third party
// license terms applicable to your use of third party software
// (including open source software) that may accompany this
// Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES, WHETHER
// EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING
// ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR
// FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
// LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR
// CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
// WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S
// TOTAL LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT
// EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR
// THIS SOFTWARE. MICROCHIP OFFERS NO SUPPORT FOR THE SOFTWARE. YOU
// MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF
// AVAILABLE.

#ifndef __SHLS_SEV_OPENCV_UTILS_HPP__
#define __SHLS_SEV_OPENCV_UTILS_HPP__

#include "common.hpp"
#include <hls/ap_int.hpp>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>

namespace {
using cv::Mat;
using std::cout;
using std::endl;
}

namespace hls {
namespace sev {

// Corresponds to PixelType in params.hpp
static constexpr int CvPixelTypeList[] = {
    CV_8UC1,  CV_8UC2,  CV_8UC3,  CV_8UC4,  CV_16UC1, CV_16UC3,
    CV_16UC4, CV_16SC1, CV_16SC3, CV_32SC1, CV_32SC3};

#define __SEV_SCP static constexpr PixelType
template <int CvType> struct PixelTypeFromCv { ; };

template <> struct PixelTypeFromCv<CV_8UC1> { __SEV_SCP T = SEV_8UC1; };
template <> struct PixelTypeFromCv<CV_8UC2> { __SEV_SCP T = SEV_8UC2; };
template <> struct PixelTypeFromCv<CV_8UC3> { __SEV_SCP T = SEV_8UC3; };
template <> struct PixelTypeFromCv<CV_8UC4> { __SEV_SCP T = SEV_8UC4; };
template <> struct PixelTypeFromCv<CV_16UC1> { __SEV_SCP T = SEV_16UC1; };
template <> struct PixelTypeFromCv<CV_16UC3> { __SEV_SCP T = SEV_16UC3; };
template <> struct PixelTypeFromCv<CV_16UC4> { __SEV_SCP T = SEV_16UC4; };
template <> struct PixelTypeFromCv<CV_16SC1> { __SEV_SCP T = SEV_16SC1; };
template <> struct PixelTypeFromCv<CV_16SC3> { __SEV_SCP T = SEV_16SC3; };
template <> struct PixelTypeFromCv<CV_32SC1> { __SEV_SCP T = SEV_32SC1; };
template <> struct PixelTypeFromCv<CV_32SC3> { __SEV_SCP T = SEV_32SC3; };
#undef __SEV_SCP

/**
 * Converts sev::Img into cv::Mat.
 */
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE,
          NumPixelsPerCycle NPPC>
void convertToCvMat(Img<PIXEL_T, H, W, STORAGE, NPPC> &InImg, Mat &OutMat) {
    const unsigned PerChannelPixelWidth =
        DT<PIXEL_T, NPPC>::PerChannelPixelWidth;

    const unsigned NumChannels = DT<PIXEL_T, NPPC>::NumChannels;
    using CvType = cv::Vec<typename DT<PIXEL_T, NPPC>::PixelPrimT,
                           DT<PIXEL_T, NPPC>::NumChannels>;
    OutMat =
        Mat(InImg.get_height(), InImg.get_width(), CvPixelTypeList[PIXEL_T]);
    for (int i = 0, idx = 0; i < InImg.get_height(); i++) {
        for (int j = 0; j < InImg.get_width() / NPPC; j++, idx++) {
            auto V = InImg.read(idx);
            for (int n = 0, k = 0; n < NPPC; n++) {
                auto &MPixel = OutMat.at<CvType>(i, j * NPPC + n);
                for (int c = 0; c < NumChannels; c++, k++) {
                    MPixel[c] = V.byte(k, PerChannelPixelWidth);
                }
            }
            InImg.write(idx, V); // Write back in case of FIFO storage.
        }
    }
}

/**
 * Converts cv::Mat into sev::Img.
 */
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE,
          NumPixelsPerCycle NPPC>
void convertFromCvMat(Mat &InMat, Img<PIXEL_T, H, W, STORAGE, NPPC> &OutImg) {
    // Convert CV's InMat to OutImage's format type.
    cv::Mat MatCopy;
    InMat.convertTo(MatCopy, CvPixelTypeList[PIXEL_T]);

    // Set OutImg to the same resolution.
    OutImg.set_height(InMat.rows);
    OutImg.set_width(InMat.cols);

    const unsigned PerChannelPixelWidth =
        DT<PIXEL_T, NPPC>::PerChannelPixelWidth;

    const unsigned NumChannels = DT<PIXEL_T, NPPC>::NumChannels;
    using CvType = cv::Vec<typename DT<PIXEL_T, NPPC>::PixelPrimT,
                           DT<PIXEL_T, NPPC>::NumChannels>;
    for (int i = 0, idx = 0; i < OutImg.get_height(); i++) {
        for (int j = 0; j < OutImg.get_width() / NPPC; j++, idx++) {
            // One word in Img, concatenating all channels and multiple NPPC.
            typename DT<PIXEL_T, NPPC>::T V;
            for (int n = 0, k = 0; n < NPPC; n++) {
                auto &MPixel = MatCopy.at<CvType>(i, j * NPPC + n);
                for (int c = 0; c < NumChannels; c++, k++) {
                    V.byte(k, PerChannelPixelWidth) = MPixel[c];
                }
            }
            OutImg.write(idx, V);
        }
    }
}

/**
 * Returns the percentage of pixels having differences greater than threshold.
 */
float compareMat(const cv::Mat &A, const cv::Mat &B, double Threshold) {
    Mat Diff;
    cv::absdiff(A, B, Diff);
    Mat OverThreshold = Diff > Threshold;

    float NumOverThreshold = cv::countNonZero(OverThreshold);
    return 100 * NumOverThreshold / Diff.total();
}

/**
 * Same as above, in addition the function reports the locations of diff that
 * are greater than threshold.
 * When printing the large diff pixel values, the values are printed in type T.
 */
template <typename T>
float compareMatAndReport(const cv::Mat &A, const cv::Mat &B,
                          double Threshold) {
    Mat Diff;
    cv::absdiff(A, B, Diff);
    Mat OverThreshold = Diff > Threshold;

    cout << "Locations where diff is greater than Threshold (" << Threshold
         << "):" << endl;
    Mat Locations;
    cv::findNonZero(OverThreshold, Locations);
    for (int i = 0; i < Locations.total(); i++) {
        cv::Point P = Locations.at<cv::Point>(i);
        cout << "  " << P << ": " << A.at<T>(P) << " vs " << B.at<T>(P) << endl;
    }

    float NumOverThreshold = cv::countNonZero(OverThreshold);
    return 100 * NumOverThreshold / Diff.total();
}

} // end of namespace sev
} // end of namespace hls

#endif

