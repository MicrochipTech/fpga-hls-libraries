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

#ifndef __SHLS_VISION_OPENCV_UTILS_HPP__
#define __SHLS_VISION_OPENCV_UTILS_HPP__

#include "common.hpp"
#include <hls/ap_int.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <set>
#include <stdio.h>
#include <vector>

namespace {
using cv::Mat;
using std::cout;
using std::endl;
}

namespace hls {
namespace vision {

// Corresponds to PixelType in params.hpp
static constexpr int CvPixelTypeList[] = {
    CV_8UC1,  CV_8UC2,  CV_8UC3,  CV_8UC4,  CV_16UC1, CV_16UC3,
    CV_16UC4, CV_16SC1, CV_16SC3, CV_32SC1, CV_32SC3};

#define __VISION_SCP static constexpr PixelType
template <int CvType> struct PixelTypeFromCv { ; };

template <> struct PixelTypeFromCv<CV_8UC1> { __VISION_SCP T = HLS_8UC1; };
template <> struct PixelTypeFromCv<CV_8UC2> { __VISION_SCP T = HLS_8UC2; };
template <> struct PixelTypeFromCv<CV_8UC3> { __VISION_SCP T = HLS_8UC3; };
template <> struct PixelTypeFromCv<CV_8UC4> { __VISION_SCP T = HLS_8UC4; };
template <> struct PixelTypeFromCv<CV_16UC1> { __VISION_SCP T = HLS_16UC1; };
template <> struct PixelTypeFromCv<CV_16UC3> { __VISION_SCP T = HLS_16UC3; };
template <> struct PixelTypeFromCv<CV_16UC4> { __VISION_SCP T = HLS_16UC4; };
template <> struct PixelTypeFromCv<CV_16SC1> { __VISION_SCP T = HLS_16SC1; };
template <> struct PixelTypeFromCv<CV_16SC3> { __VISION_SCP T = HLS_16SC3; };
template <> struct PixelTypeFromCv<CV_32SC1> { __VISION_SCP T = HLS_32SC1; };
template <> struct PixelTypeFromCv<CV_32SC3> { __VISION_SCP T = HLS_32SC3; };
#undef __VISION_SCP

/**
 * Converts vision::Img into cv::Mat.
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
            InImg.write(V, idx); // Write back in case of FIFO storage.
        }
    }
}

/**
 * Converts cv::Mat into vision::Img.
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
            OutImg.write(V, idx);
        }
    }
}

/*****************************************************************************/
/******************************** CompareMat *********************************/
/*****************************************************************************/

/**
 * Helper function: PointCmp()
 */
// In order to use a set of `cv::Point`, we need to define a comparison object.
// It doesn't matter what it does as long as it can differentiate the different
// `Point` to prevent duplicates. So let's just define it by first comparing the
// X coordinate, and if they're equal, then compare the Y coordinate.
bool PointCmp(cv::Point P1, cv::Point P2) {
    return (P1.x < P2.x) || (P1.x == P2.x && P1.y < P2.y);
}

/**
 * Helper function: findDiffLocations()
 */
// The output `Location` is a set of Points of the 2 `Mat` `A` and `B` that have
// a difference greater than `Threshold`.
void findDiffLocations(const cv::Mat &A, const cv::Mat &B, double Threshold,
                       std::set<cv::Point, decltype(&PointCmp)> &Locations) {
    // Steps to compare 2 Mat `A` and `B` across all channels:
    // 1. Find the `OverThreshold` Mat. This Mat has pixels that are 1 if the
    //    difference between `A` and `B` exceeds `Threshold`, and 0 otherwise.
    // 2. Split the `OverThreshold` mat into an array of Mat
    //    `OverThresholdAllChannels[]`, each Mat represents a channel.
    //    E.g. OverThresholdAllChannels[0] represents the Mat where the value of
    //    each pixel represents if the difference between `A` and `B` in the
    //    first channel at that pixel exceed `Threshold`.
    // 3. Find all pixels that are different in any 1 channel, then put them in
    //    a set to prevent duplicates.

    // 1. Find the `OverThreshold` Mat.
    Mat Diff;
    cv::absdiff(A, B, Diff);
    Mat OverThreshold = Diff > Threshold;

    // 2. Split the `OverThreshold` mat into an array of Mat
    // `OverThresholdAllChannels`, each represents a channel.
    std::vector<Mat> OverThresholdAllChannels;
    cv::split(OverThreshold, OverThresholdAllChannels);

    // 3. Find all pixels that are different in any channel, then put them in
    // the output set.
    for (auto &OverThresholdEachChannel : OverThresholdAllChannels) {
        std::vector<cv::Point> LocationsEachChannel;
        cv::findNonZero(OverThresholdEachChannel, LocationsEachChannel);
        std::copy(LocationsEachChannel.begin(), LocationsEachChannel.end(),
                  std::inserter(Locations, Locations.end()));
    }
}

/**
 * Main function: compareMat()
 */
// Returns the percentage of pixels having at least 1 channel that have a
// difference greater than threshold.
float compareMat(const cv::Mat &A, const cv::Mat &B, double Threshold) {
    std::set<cv::Point, decltype(&PointCmp)> Locations(&PointCmp);
    findDiffLocations(A, B, Threshold, Locations);
    float NumOverThreshold = Locations.size();
    return 100 * NumOverThreshold / A.total();
}

/**
 * Main function: compareMatAndReport<T>()
 */
// Same as `compareMat()`; in addition the function reports the locations of
// diff that are greater than threshold.
// When printing the large diff pixel values, the values are printed in type T.
// Example usages:
// - 8UC1:  compareMatAndReport<unsigned char>(...)
// - 8UC3:  compareMatAndReport<Vec3b>(...)
// - 16SC1: compareMatAndReport<int16_t>(...)
// Note: for types such as `char`, `cout` will print a character instead of a
// number. We'll need special handling for these cases
template <typename T, typename std::enable_if<sizeof(T) == 1>::type * = nullptr>
float compareMatAndReport(const cv::Mat &A, const cv::Mat &B,
                          double Threshold) {

    std::set<cv::Point, decltype(&PointCmp)> Locations(&PointCmp);
    findDiffLocations(A, B, Threshold, Locations);

    // Print out the value of `A` and `B` at each pixel location that are
    // different.
    cout << "Locations where diff is greater than Threshold (" << Threshold
         << "):" << endl;
    for (auto &P : Locations) {
        // `cout` will print data types like `char` as characters instead of
        // numbers.
        // To print them as numbers, add the unary operator `+` in front.
        cout << "  " << P << ": " << +A.at<T>(P) << " vs " << +B.at<T>(P)
             << endl;
    }

    float NumOverThreshold = Locations.size();
    return 100 * NumOverThreshold / A.total();
}

template <typename T, typename std::enable_if<sizeof(T) != 1>::type * = nullptr>
float compareMatAndReport(const cv::Mat &A, const cv::Mat &B,
                          double Threshold) {

    std::set<cv::Point, decltype(&PointCmp)> Locations(&PointCmp);
    findDiffLocations(A, B, Threshold, Locations);

    // Print out the value of `A` and `B` at each pixel location that are
    // different.
    cout << "Locations where diff is greater than Threshold (" << Threshold
         << "):" << endl;
    for (auto &P : Locations) {
        cout << "  " << P << ": " << A.at<T>(P) << " vs " << B.at<T>(P) << endl;
    }

    float NumOverThreshold = Locations.size();
    return 100 * NumOverThreshold / A.total();
}

} // end of namespace vision
} // end of namespace hls

#endif
