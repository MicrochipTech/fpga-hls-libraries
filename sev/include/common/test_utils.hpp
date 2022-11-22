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

#ifndef __SHLS_SEV_TEST_UTILS_HPP__
#define __SHLS_SEV_TEST_UTILS_HPP__

#include "common.hpp"
#include <hls/ap_int.hpp>
#include <stdio.h>

namespace hls {
namespace sev {

/**
 * Bmp pixel type, containing r, g, and b components.
 */
struct BmpPixelT {
    unsigned char b, g, r;
};

/**
 * Bmp header file type.
 */
struct BmpHeaderT {
    uint16_t type;     // Magic identifier: 0x4d42
    uint32_t size;     // File size in bytes
    uint32_t reserved; // Not used
    uint32_t offset; // Offset to image data in bytes from beginning of file (54
                     // bytes)
    uint32_t dib_header_size;  // DIB Header size in bytes (40 bytes)
    int32_t width;             // Width of the image
    int32_t height;            // Height of image
    uint16_t num_planes;       // Number of color planes
    uint16_t bits_per_pixel;   // Bits per pixel
    uint32_t compression;      // Compression type
    uint32_t image_size;       // Image size in bytes
    int32_t x_resolution_ppm;  // Pixels per meter
    int32_t y_resolution_ppm;  // Pixels per meter
    uint32_t num_colors;       // Number of colors
    uint32_t important_colors; // Important colors
} __attribute__((__packed__));

/**
 * This function reads a bmp image into an array of BmpPixelT.
 *   Input:
 *     - filename is the file name of the bmp image.
 *     - img_width and img_height define the expected dimension of the image.
 *   Output:
 *     - header_buf: holds the bmp header parsed from the bmp file.
 *     - image_buf: holds the image pixels parsed from the bmp file.
 *     Note: header_buf and image_buf should point to pre-allocated memories.
 *
 *   Returns true on success and false otherwise.
 */
bool readBmp(const char *filename, unsigned img_width, unsigned img_height,
             BmpHeaderT *header_buf, BmpPixelT *img_buf) {
    FILE *file = fopen(filename, "rb");
    if (!file)
        return false;

    // Read BMP header.
    if (fread(header_buf, sizeof(BmpHeaderT), 1, file) != 1)
        return false;
    // Verify header offset, width and height.
    if (header_buf->offset != sizeof(BmpHeaderT) ||
        header_buf->width != img_width || header_buf->height != img_height)
        return false;

    // Parse BMP image.
    unsigned image_size = img_width * img_height;
    if (fread(img_buf, sizeof(BmpPixelT), image_size, file) != image_size)
        return false;

    fclose(file);
    return true;
}

/**
 * This function writes an array of BmpPixelT to a bmp image file.
 *   Input:
 *     - filename is the file name of the bmp image.
 *     - header_buf: holds the bmp header.
 *     - image_buf: holds the image pixels.
 */
void writeBmp(const char *filename, const BmpHeaderT *header_buf,
              const BmpPixelT *img_buf) {
    FILE *file = fopen(filename, "wb");
    if (!file)
        return;
    fwrite(header_buf, sizeof(BmpHeaderT), 1, file);
    fwrite(img_buf, sizeof(BmpPixelT), header_buf->width * header_buf->height,
           file);
    fclose(file);
}

/**
 * This function converts a grayscale sev::Img to an array of BmpPixelT
 */
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
BmpPixelT *grayscaleImg2Bmp(sev::Img<PIXEL_T, H, W, STORAGE, NPPC> &ImgIn) {
    // This function assumes a grayscale img, with an 8 bit component
    int img_height = ImgIn.get_height();
    int img_width = ImgIn.get_width();
    int img_columns = img_width / NPPC;
    int image_index = 0;
    typename DT<PIXEL_T, NPPC>::T Imgdata;
    // if storage type is fifo, we need to make a copy because reading from the
    // image empties the fifo
    BmpPixelT *image =
        (BmpPixelT *)malloc((img_height * img_width) * sizeof(BmpPixelT));

    for (int i = 0; i < img_height; i++) {
        for (int j = 0; j < img_columns; j++) {
            Imgdata = ImgIn.read(image_index);
            // extract each separate pixel taking in mind when we have more than
            // 1 pixel per clock cycle
            for (int k = 0; k < NPPC; k++) {
                // to get the exact location of the pixel in the image, we use
                // (img_width * i) + j * NPPC + k, or image_index * NPPC + k,
                // which takes in mind image width (not image columns)
                // and number of pixels per cycle in the input Img
                //
                image[image_index * NPPC + k].r = (Imgdata.byte(k));
                image[image_index * NPPC + k].g = (Imgdata.byte(k));
                image[image_index * NPPC + k].b = (Imgdata.byte(k));
            }
            image_index++;
        }
    }
    return image;
}

/**
 * This function converts an rgb sev::Img to an array of BmpPixelT
 */
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
BmpPixelT *rgbImg2Bmp(sev::Img<PIXEL_T, H, W, STORAGE, NPPC> &ImgIn) {
    // This function assumes 8 bit components (r, g, b), hence each (rgb) pixel
    // having 3 * 8 = 24 bits of data
    // unsigned height = ImgIn.get_height();
    int img_height = ImgIn.get_height();
    int img_width = ImgIn.get_width();
    int img_columns = img_width / NPPC;
    int image_index = 0;
    typename DT<PIXEL_T, NPPC>::T Imgdata;
    // if storage type is fifo, we need to make a copy because reading from the
    // image empties the fifo
    BmpPixelT *image =
        (BmpPixelT *)malloc((img_height * img_width) * sizeof(BmpPixelT));
    for (int i = 0; i < img_height; i++) {
        for (int j = 0; j < img_columns; j++) {
            Imgdata = ImgIn.read(image_index);
            // extract each separate pixel taking in mind when we have more than
            // 1 pixel per clock cycle
            for (int k = 0; k < NPPC; k++) {
                // to get the exact location of the pixel in the image, we use
                // (img_width * i) + j * NPPC + k, which takes in mind image
                // width (not image columns)
                image[image_index * NPPC + k].r = Imgdata.byte(3 * k);
                image[image_index * NPPC + k].g = Imgdata.byte(3 * k + 1);
                image[image_index * NPPC + k].b = Imgdata.byte(3 * k + 2);
            }
            image_index++;
        }
    }
    return image;
}

/**
 * Converts a bmp image stored in a BmpPixelT array to sev::Img format
 */
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void bmp2Img(BmpPixelT *BmpImage,
             sev::Img<PIXEL_T, H, W, STORAGE, NPPC> &ImgOut) {
    int img_height = ImgOut.get_height();
    int img_width = ImgOut.get_width();
    int img_columns = img_width / NPPC;
    int image_index = 0;
    typename DT<PIXEL_T, NPPC>::T Imgdata;

    for (int i = 0; i < img_height; i++) {
        for (int j = 0; j < img_columns; j++) {

            Imgdata = 0;
            for (int k = 0; k < NPPC; k++) {
                Imgdata.byte(3 * k) = BmpImage[image_index * NPPC + k].r;
                Imgdata.byte(3 * k + 1) = BmpImage[image_index * NPPC + k].g;
                Imgdata.byte(3 * k + 2) = BmpImage[image_index * NPPC + k].b;
            }
            ImgOut.write(image_index++, Imgdata);
        }
    }
}

/**
 * Creates RGB image frames containing vertical colored lines
 */
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void patternGenerator(sev::Img<PIXEL_T, H, W, STORAGE, NPPC> &ImgOut) {
    int img_height = ImgOut.get_height();
    int img_width = ImgOut.get_width();
    int img_columns = img_width / NPPC;
    int image_index = 0;
    int color_column_width = img_columns / 8;

    typename DT<PIXEL_T, NPPC>::T Imgdata;
    typename DT<PIXEL_T, NPPC_1>::T Pixeldata;

    for (int i = 0; i < img_height; i++) {
#ifdef __SYNTHESIS__
#pragma HLS loop pipeline
#endif
        for (int j = 0; j < img_columns; j++) {
            if (j < 4 * color_column_width) {
                if (j < 2 * color_column_width) {
                    if (j < color_column_width)
                        // black
                        Pixeldata = 0x000000;
                    else
                        // blue
                        Pixeldata = 0xFF0000;
                } else {
                    if (j < 3 * color_column_width)
                        // green
                        Pixeldata = 0x00FF00;
                    else
                        // cyan
                        Pixeldata = 0xFFFF00;
                }
            } else {
                if (j < 6 * color_column_width) {
                    if (j < 5 * color_column_width)
                        // red
                        Pixeldata = 0x0000FF;
                    else
                        // pink
                        Pixeldata = 0xFF00FF;
                } else {
                    if (j < 7 * color_column_width)
                        // yellow
                        Pixeldata = 0x00FFFF;
                    else
                        // white
                        Pixeldata = 0xFFFFFF;
                }
            }

            Imgdata = 0;
            for (int k = 0; k < NPPC; k++) {
                Imgdata.byte(k, 24) = Pixeldata;
            }
            ImgOut.write(image_index++, Imgdata);
        }
    }
}

} // end of namespace sev
} // end of namespace hls

#endif
