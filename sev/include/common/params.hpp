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
// MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.
// TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL LIABILITY ON ALL
// CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF FEES, IF ANY, YOU PAID
// DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP OFFERS NO SUPPORT FOR THE
// SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#ifndef __SHLS_SEV_PARAMS_HPP__
#define __SHLS_SEV_PARAMS_HPP__

namespace hls {
namespace sev {

enum StorageType {
    FIFO,
    FRAME_BUFFER,
    EXTERNAL_FRAME_BUFFER,
    PRIMITIVE_TYPE_EXTERNAL_FRAME_BUFFER
};

enum PixelType {
    SEV_8UC1 = 0,
    SEV_16UC1 = 1,
    SEV_16SC1 = 2,
    SEV_32UC1 = 3,
    SEV_32FC1 = 4,
    SEV_32SC1 = 5,
    SEV_8UC2 = 6,
    SEV_8UC4 = 7,
    SEV_2UC1 = 8,
    SEV_8UC3 = 9,
    SEV_16UC3 = 10,
    SEV_16SC3 = 11,
    SEV_16UC4 = 12,
    SEV_10UC1 = 13,
    SEV_10UC4 = 14,
    SEV_12UC1 = 15,
    SEV_12UC4 = 16,
    SEV_10UC3 = 17,
    SEV_12UC3 = 18,
    SEV_32FC3 = 19
};

enum NumPixelsPerCycle {
    NPPC_1 = 1,
    NPPC_2 = 2,
    NPPC_4 = 4,
    NPPC_8 = 8,
    NPPC_16 = 16,
    NPPC_32 = 32,
    NPPC_64 = 64
};

} // End of namespace sev.
} // End of namespace hls.

#endif

