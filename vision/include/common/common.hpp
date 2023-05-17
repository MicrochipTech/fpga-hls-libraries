// ©2022 Microchip Technology Inc. and its subsidiaries
//
// Subject to your compliance with these terms, you may use this Microchip
// software and any derivatives exclusively with Microchip products. You are
// responsible for complying with third party license terms applicable to your
// use of third party software (including open source software) that may
// accompany this Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES,
// WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
// IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A
// PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
// SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
// OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#ifndef __SHLS_VISION_COMMON_HPP__
#define __SHLS_VISION_COMMON_HPP__

#include "params.hpp"
#include <hls/streaming.hpp>

#ifndef __SYNTHESIS__
#define LEGUP_SW_IMPL(code) code
#else
// Removes the 'code' when __SYNTHESIS__ is defined.
#define LEGUP_SW_IMPL(code)
#endif

#ifndef __SYNTHESIS__
#include <assert.h>
#endif

namespace hls {
namespace vision {

/**
 * This class is used to represent and store images. It supports various pixels
 * per clock (NPPC, up to 64), pixel types (like RGB (3 components, each 8 bits
 * )), and different storage types:
 *      FIFO
 *      FRAME_BUFFER
 *      EXTERNAL_FRAME_BUFFER
 */
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
class Img {
  private:
    using DATA_T_ = typename DT<PIXEL_T, NPPC>::T;
    using STORAGE_T_ = typename std::conditional<
        STORAGE == StorageType::FIFO, hls::FIFO<DATA_T_>,
        typename std::conditional<
            STORAGE == StorageType::FRAME_BUFFER,
#ifndef __SYNTHESIS__
            DATA_T_ *, /* the large image size can cause stack overflow in
                          software, so we will only dynamically allocate the
                          memory in the constructor. */
#else
            DATA_T_[H * W / NPPC],
#endif
            typename std::conditional<
                STORAGE == StorageType::EXTERNAL_FRAME_BUFFER,
                DATA_T_ *, /* external memory, expected to already be allocated
                             somewhere else */
                PrimT<DT<PIXEL_T, NPPC>::W> *>::type>::type>::type;

    unsigned height, width;
    STORAGE_T_ data;

    // Note: If we need to include any new class member, we should include them
    // before `data`. This is to make memory partition easier for FRAME_BUFFER
    // case. By not having any class member after the `data` array, we don't
    // need to worry about any overflow during memory partition.

  public:
    /**************************************************************************/
    /********************** Constructors and destructors **********************/
    /**************************************************************************/

    /************************** StorageType is FIFO ***************************/
    /**
     * General constructor. Do nothing unless we're running software, then we
     * need to set the FIFO depth to be able to hold the whole frame because
     * there's no dataflow.
     */
    template <
        StorageType st = STORAGE,
        typename std::enable_if<st == StorageType::FIFO>::type * = nullptr>
    Img(unsigned height_, unsigned width_) : height(height_), width(width_) {
#ifndef __SYNTHESIS__
        data.setDepth(H * W / NPPC);
#endif
    };

    /**
     * Constructor that also sets the FIFO depth.
     */
    template <
        StorageType st = STORAGE,
        typename std::enable_if<st == StorageType::FIFO>::type * = nullptr>
    Img(unsigned height_, unsigned width_, unsigned fifo_depth)
        : height(height_), width(width_), data(fifo_depth) {
#ifndef __SYNTHESIS__
        // For software, the fifo depth has to be `H * W / NPPC` regardless.
        data.setDepth(H * W / NPPC);
#endif
    };

    /********************** StorageType is FRAME_BUFFER **********************/
    /**
     * General constructor. Do nothing unless we're running software, then we
     * need to dynamically allocate memory for `data`.
     */
    template <StorageType st = STORAGE,
              typename std::enable_if<st == StorageType::FRAME_BUFFER>::type * =
                  nullptr>
    Img(unsigned height_, unsigned width_) : height(height_), width(width_) {
#ifndef __SYNTHESIS__
        data = new DATA_T_[H * W / NPPC];
#endif
    };

    /*************** StorageType is not FIFO nor FRAME_BUFFER ****************/
    /**
     * General constructor.
     */
    template <StorageType st = STORAGE,
              typename std::enable_if<st != StorageType::FIFO &&
                                      st != StorageType::FRAME_BUFFER>::type * =
                  nullptr>
    Img(unsigned height_, unsigned width_) : height(height_), width(width_){};

    /**
     * General constructor for EXTERNAL_FRAME_BUFFER type. Set the pointer to an
     * external memory.
     */
    template <StorageType st = STORAGE,
              typename std::enable_if<
                  st == StorageType::EXTERNAL_FRAME_BUFFER ||
                  st == StorageType::PRIMITIVE_TYPE_EXTERNAL_FRAME_BUFFER>::type
                  * = nullptr>
    Img(unsigned height_, unsigned width_, STORAGE_T_ external_frame_buffer)
        : height(height_), width(width_), data(external_frame_buffer){};

    /******************************** Common *********************************/
    /**
     * Constructor
     */
    Img() : Img(H, W){};

    /**
     * Copy constructor.  This function is not synthesizable to HW.
     * This function is mainly to be used in software testbenches.
     */
    template <StorageType STORAGE_1>
    Img(Img<PIXEL_T, H, W, STORAGE_1, NPPC> &img1) LEGUP_SW_IMPL(
        : Img(H, W) {
            this->height = img1.get_height();
            this->width = img1.get_width();
            for (unsigned i = 0, read_index = 0; i < height; i++) {
                for (unsigned j = 0; j < width / NPPC; j++, read_index++) {
                    DATA_T_ pixel_temp = img1.read(read_index);
                    this->write(pixel_temp, read_index);
                    if (STORAGE_1 == FIFO) {
                        // idx is not correct but it doesn't actually matter
                        // when writing to a fifo which doesn't need the index.
                        img1.write(pixel_temp, read_index);
                    }
                }
            }
        });

    /**
     * Destructor. Do nothing unless we're running software for FRAME_BUFFER
     * type, then we need to free `data` which was dynamically allocated in the
     * constructor before.
     */
#ifndef __SYNTHESIS__
    template <StorageType st = STORAGE,
              typename std::enable_if<st == StorageType::FRAME_BUFFER>::type * =
                  nullptr>
    void DestructorHelper() {
        delete[] data;
    };
    template <StorageType st = STORAGE,
              typename std::enable_if<st != StorageType::FRAME_BUFFER>::type * =
                  nullptr>
    void DestructorHelper(){};

    ~Img() { DestructorHelper(); }
#endif

    /**************************************************************************/
    /**************************************************************************/
    /**************************************************************************/

    unsigned get_height() const { return height; }
    unsigned get_width() const { return width; }

    void set_height(unsigned h) {
        LEGUP_SW_IMPL(assert(h <= H));
        height = h;
    }
    void set_width(unsigned w) {
        LEGUP_SW_IMPL(assert(w <= W));
        width = w;
    }

    /**
     * set_fifo_depth()
     * Set the FIFO depth if the StorageType is FIFO, otherwise it's a no-op.
     */
    template <
        StorageType st = STORAGE,
        typename std::enable_if<st == StorageType::FIFO>::type * = nullptr>
    void set_fifo_depth(unsigned fifo_depth) {
#ifndef __SYNTHESIS__
        // For software, the fifo depth has to be `H * W / NPPC` regardless.
        data.setDepth(H * W / NPPC);
#else
        data.setDepth(fifo_depth);
#endif
    };
    template <
        StorageType st = STORAGE,
        typename std::enable_if<st != StorageType::FIFO>::type * = nullptr>
    void set_fifo_depth(unsigned fifo_depth){};

    /**
     * read() & write()
     * Enable if this is a FRAME_BUFFER or EXTERNAL_FRAME_BUFFER storage.
     */
    template <
        StorageType st = STORAGE,
        typename std::enable_if<st != StorageType::FIFO>::type * = nullptr>
    DATA_T_ read(unsigned idx) {
        return data[idx];
    }

    template <
        StorageType st = STORAGE,
        typename std::enable_if<st != StorageType::FIFO>::type * = nullptr>
    void write(DATA_T_ val, unsigned idx) {
        data[idx] = val;
    }

    /**
     * read() & write()
     * Enable if this is a FIFO.
     */
    template <
        StorageType st = STORAGE,
        typename std::enable_if<st == StorageType::FIFO>::type * = nullptr>
    DATA_T_ read(unsigned idx = 0) {
        return data.read();
    }

    template <
        StorageType st = STORAGE,
        typename std::enable_if<st == StorageType::FIFO>::type * = nullptr>
    void write(DATA_T_ val, unsigned idx = 0) {
        data.write(val);
    }

    /**
     * Returns the number of data elements in the FIFO (if storage is FIFO),
     * otherwise return the image size.
     */
    template <
        StorageType st = STORAGE,
        typename std::enable_if<st == StorageType::FIFO>::type * = nullptr>
    unsigned get_usedw() LEGUP_SW_IMPL({ return data.get_usedw(); });
    template <
        StorageType st = STORAGE,
        typename std::enable_if<st != StorageType::FIFO>::type * = nullptr>
    unsigned get_usedw() LEGUP_SW_IMPL({ return height * width; });

    /**
     * Returns true if this image is identical to rhs image.
     * This function is not synthesizable.
     */
    template <PixelType PIXEL_I_T, StorageType RHS_STORAGE>
    bool IsEqualTo(Img<PIXEL_I_T, H, W, RHS_STORAGE, NPPC> &rhs) LEGUP_SW_IMPL({
        // Return false if size do not match (only happens if storage is FIFO)
        if (this->get_usedw() != rhs.get_usedw())
            return false;

        // If storage type is fifo, we write back read-out pixels.
        // When mismatch detected, we cannot skip remaining pixels because we
        // need to write back all remaining pixels to FIFO to completely restore
        // the Img.
        bool is_equal = true;
        for (unsigned i = 0, idx = 0; i < rhs.get_height(); i++) {
            for (unsigned j = 0; j < rhs.get_width() / NPPC; j++, idx++) {
                DATA_T_ this_pixel = this->read(idx);
                DATA_T_ rhs_pixel = rhs.read(idx);

                if (STORAGE == FIFO)
                    this->write(this_pixel, idx);

                if (RHS_STORAGE == FIFO)
                    rhs.write(rhs_pixel, idx);

                if (this_pixel != rhs_pixel)
                    is_equal = false;
            }
        }
        return is_equal;
    });
};

} // End of namespace vision.
} // End of namespace hls.

#endif
