<!-- TOC -->

- [Configure Top-level RTL interface for Img arguments](#configure-top-level-rtl-interface-for-img-arguments)
    - [Memory interface and AXI4 Stream interface](#memory-interface-and-axi4-stream-interface)
    - [AXI4 Initiator interface and AXI4 Target interface](#axi4-initiator-interface-and-axi4-target-interface)
        + [AxiMM2Img()](#aximm2img)
        + [Img2AxiMM()](#img2aximm)
    - [Mixed interfaces](#mixed-interfaces)
- [AXI4-Stream Video Protocol](#axi4-stream-video-protocol)
    - [AXIS-Video vs FIFO-based vision::Img AXI4 Stream interface](#axis-video-vs-fifo-based-visionimg-axi4-stream-interface)
    - [AxisVideo2Img()](#axisvideo2img)
    - [Img2AxisVideo()](#img2axisvideo)
    - [AxiMM2AxisVideo()](#aximm2axisvideo)
    - [AxisVideo2AxiMM()](#axisvideo2aximm)

<!-- /TOC -->
# Configure Top-level RTL interface for `Img` arguments
For general knowledge about the top-level RTL interfaces that are available in SmartHLS, please refer to the SmartHLS User Guide: [Top-Level RTL Interface](https://microchiptech.github.io/fpga-hls-docs/userguide.html#top-level-rtl-interface).

In SmartHLS, the top-level C++ function prototype defines the RTL interface of the top-level module.
Generally speaking, each top-level function argument corresponds to an RTL interface.
The Vision library supports implementing top-level image arguments in one of the four interfaces:
- **Memory interface**
- **AXI4 Stream interface**
- **AXI4 Initiator interface**
- **AXI4 Target interface**

To configure the interface, we apply different `StorageType` template parameter in the argument's `vision::Img` template instantiation,
and additionally apply width conversion logic and HLS `interface` pragmas for the cases of **AXI4 Initiator interface** and **AXI4 Target interface**.

We will use an example to illustrate how to configure the top-level interfaces.
The example contains a top-level function that converts color format from RGB to grayscale using our library's color conversion function [`vision::RGB2GRAY()`](../imgproc/README.md#rgb2gray).
> For all examples in this section, we will follow the following assumptions:
> - The input image's `PixelType` is `HLS_8UC3` (3 channels for RGB with 8 bits per channel).
> - The output image's `PixelType` is `HLS_8UC1` (1 channel for grayscale with 8 bits per channel).
> - Some of the template parameters that are not important for the demonstration for `Img` objects are truncated for clarity purpose.

For more information about different template parameters for the `Img` class, please refer to [Template Parameters](../common/README.md#template-parameters).

## **Memory interface** and **AXI4 Stream interface**
For **Memory interface** and **AXI4 Stream interface**, the inputs / outputs of your top-level function need to be `Img` objects, passed in by reference.

By specifying the `StorageType` template parameter of the input / output `Img` argument, you can configure that argument to follow either **Memory interface** or **AXI4 Stream interface**. In particular:
- If `StorageType` is `FRAME_BUFFER`, then the `Img` argument will use **Memory interface**.
- If `StorageType` is `FIFO`, then the `Img` argument will use **AXI4 Stream interface**.

For example, in the `RGB2GRAYTop()` top-level function below, the input `InImg` uses **Memory interface**, and the output `OutImg` uses **AXI4 Stream interface**.
```cpp
void RGB2GRAYTop(
    Img<vision::PixelType::HLS_8UC3, vision::StorageType::FRAME_BUFFER> &InImg,
    Img<vision::PixelType::HLS_8UC1, vision::StorageType::FIFO> &OutImg) {
#pragma HLS function top
#pragma HLS function dataflow
#pragma HLS memory partition argument(InImg) type(struct_fields)
    vision::RGB2GRAY(InImg, OutImg);
}
```

For a complete example SmartHLS design, including how you can set up the input `InImg` and output `OutImg` outside of the top-level function in the `main()` testbench, please see: [examples/configure_interfaces/rgb2gray_memory_intf_axi_stream_intf](../../examples/configure_interface/rgb2gray_axi_initiator_intf_axi_target_intf/rgb2gray_axi_initiator_intf_axi_target_intf_tb.cpp).

> Note that for **Memory interface**, we recommend you to perform [struct-field memory partition](https://microchiptech.github.io/fpga-hls-docs/userguide.html#struct-fields-partitioning) on the argument by using the associated pragma (as shown in above code), especially when your image is large.
> This is to prevent SmartHLS' default struct-packing optimization from happening, which packs all pixels in entire image frame to a single, very large word.
> For example, let's take a look at the interface report for a 1920x1080 `InImg`:
> - Undesired interface without struct-field memory partition:
> ```
> +----------+----------------+------------------------+------------------+------------------+
> | C++ Name | Interface Type | Signal Name            | Signal Bit-width | Signal Direction |
> +----------+----------------+------------------------+------------------+------------------+
> | InImg    | Memory         | InImg_address_a        | 1                | output           |
> |          |                | InImg_clken            | 1                | output           |
> |          |                | InImg_read_data_a      | 66355264         | input            |
> |          |                | InImg_read_en_a        | 1                | output           |
> +----------+----------------+------------------------+------------------+------------------+
> ```
> - Expected interface with memory partition:
> ```
> +----------+----------------+------------------------+------------------+------------------+
> | C++ Name | Interface Type | Signal Name            | Signal Bit-width | Signal Direction |
> +----------+----------------+------------------------+------------------+------------------+
> | InImg    | Memory         | InImg_data_address_a   | 21               | output           |
> |          |                | InImg_data_clken       | 1                | output           |
> |          |                | InImg_data_read_data_a | 24               | input            |
> |          |                | InImg_data_read_en_a   | 1                | output           |
> +----------+----------------+------------------------+------------------+------------------+
> ```

## **AXI4 Initiator interface** and **AXI4 Target interface**
**AXI4 Initiator interface** and **AXI4 Target interface** require the data width to be a power-of-2 bytes.
However, an image frame can have a more arbitrary pixel bit width (e.g., `HLS_8UC3` type pixel with `NPPC_1` has a bit width of 24, or 3 bytes).
When the `Img`'s pixel width is not a power-of-2 bytes, we cannot directly use the `Img` type for the top-level interface argument to implement **AXI4 Initiator interface** or **AXI4 Target interface**.
Rather, we need to apply extra adapter logic before the input `Img` and after the output `Img` to convert the width such that the top-level interface has a power-of-2 byte size.

The example code below uses width conversion adapters to implement the input `InAxiMM` as an **AXI4 Initiator interface**, and implement the output `OutAxiMM` as an **AXI4 Target interface**.

```cpp
#define NumPixels (WIDTH * HEIGHT)
#define NPPC 1
#define NumPixelWords (NumPixels / NPPC)
#define InPixelWordWidth (24 * NPPC) // Input image is 8UC3
#define InAxiWordWidth 32            // Input AXI memory is 32-bit
#define InNumAxiWords (NumPixelWords * InPixelWordWidth / InAxiWordWidth)
#define OutPixelWordWidth (8 * NPPC) // Output image is 8UC1
#define OutAxiWordWidth 32           // Output AXI memory is 32-bit
#define OutNumAxiWords (NumPixelWords * OutPixelWordWidth / OutAxiWordWidth)

void hlsRGB2GRAY(uint32_t *InAxiMM, uint32_t *OutAxiMM) {
#pragma HLS function top
#pragma HLS function dataflow
#pragma HLS interface argument(InAxiMM) type(axi_initiator)    \
    num_elements(InNumAxiWords) max_burst_len(256)
#pragma HLS interface argument(OutAxiMM) type(axi_target)      \
    num_elements(OutNumAxiWords)
    Img<vision::PixelType::HLS_8UC3, vision::StorageType::FIFO, vision::NPPC_1> InImg;
    Img<vision::PixelType::HLS_8UC1, vision::StorageType::FIFO, vision::NPPC_1> OutImg;
    vision::AxiMM2Img<InAxiWordWidth>(InAxiMM, InImg);
    vision::RGB2GRAY(InImg, OutImg);
    vision::Img2AxiMM<OutAxiWordWidth>(OutImg, OutAxiMM);
}
```
- The high-level idea is that the input and output of the top-level function will be pointers with a power-of-2 byte size data type, e.g., `uint32_t *` pointer.
  And the top-level function will carry out the following steps:
  1) Perform incoming width conversion on the input side to convert the 32-bit input data `InAxiMM` into an intermediate 24-bit `InImg`.
  2) Call `vision::RGB2GRAY()` to convert the 24-bit `InImg` into an intermediate 8-bit `OutImg`.
  3) Perform outgoing width conversion on the output side to pack the 8-bit `OutImg` into the 32-bit output memory `OutAxiMM`.
- The Vision library provides two width-conversion functions for doing step 1) and 3): `AxiMM2Img()`, and `Img2AxiMM()`.
- To configure the `InAxiMM` and `OutAxiMM` as either **AXI4 Initiator interface** or **AXI4 Target interface**, you can use the associated SmartHLS top-level interface pragma
  (also see SmartHLS User Guide on [AXI4 Initiator Interface](https://microchiptech.github.io/fpga-hls-docs/userguide.html#axi4-initiator-interface), and [AXI4 Target Interface](https://microchiptech.github.io/fpga-hls-docs/userguide.html#axi4-target-interface)).
- Note that in the example above, both the `InAxiWordWidth` and `OutAxiWordWidth` template parameters for the two width conversion functions are 32 and denote the AXI interface's data width.
  The pointer argument (`InAxiMM` and `OutAxiMM`) must be of a type that has the same data width as the AXI interface's data width: `uint32_t *` for 32b data.
- For the two intermediate `Img`: `InImg` and `OutImg`, we use `FIFO` `StorageType` whenever possible (when pixels are accessed in sequence) to conserve resource usage.
  For more information about why this is the case, and how to choose a suitable FIFO depth, please refer to the section on [StorageType](../common/README.md#storagetype).

For a full example SmartHLS design, including how you can set up the input `InAxiMM` and output `OutAxiMM` outside of the top-level function in `main()`, please see: [examples/configure_interfaces/rgb2gray_axi_initiator_intf_axi_target_intf](../../examples/configure_interface/rgb2gray_axi_initiator_intf_axi_target_intf).

### `AxiMM2Img()`
```cpp
template <unsigned AxiWordWidth, typename AxiDT, vision::PixelType PIXEL_T,
          unsigned H, unsigned W, vision::StorageType STORAGE,
          vision::NumPixelsPerCycle NPPC>
void AxiMM2Img(AxiDT *InAxiMM, Img<PIXEL_T, H, W, STORAGE, NPPC> &OutImg)
```
This functions converts an input AXI Memory Map `InAxiMM` to an output `Img` object `OutImg`.
It handles the width conversion and the packing/unpacking of data to convert between the AXI interface width (which has to be a power-of-2 bytes) and the more arbitrary pixel width of the `Img`.

**Template parameters:**
- `AxiWordWidth`: The AXI interface's data width (in bits) of the memory `InAxiMM`.
  This has to be a power-of-2 bytes, i.e., `AxiWordWidth / 8` has to be a power-of-2.
  - Examples: `32`, `64`.
- `AxiDT`: The C++ data type for the memory `InAxiMM`.
  This is automatically inferred from `InAxiMM`.
  The width of `AxiDT` has to match`AxiWordWidth`.
  - Examples: `uint32_t`, `uint64_t`.
- The other template parameters are automatically inferred from the `OutImg` argument.

**Example usages:**
```cpp
#define NumPixels (WIDTH * HEIGHT)
#define NPPC 1
#define NumPixelWords (NumPixels / NPPC)
#define PixelWordWidth (24 * NPPC) // Image is 8UC3
#define AxiWordWidth 64            // AXI memory is 64-bit
#define NumAxiWords (NumPixelWords * PixelWordWidth / AxiWordWidth)

uint64_t InAxiMM[NumPixelWords];
Img<vision::PixelType::HLS_8UC3, ...> OutImg;
AxiMM2Img<AxiWordWidth>(InAxiMM, OutImg);
```

### `Img2AxiMM()`
```cpp
template <unsigned AxiWordWidth, typename AxiDT, vision::PixelType PIXEL_T,
          unsigned H, unsigned W, vision::StorageType STORAGE,
          vision::NumPixelsPerCycle NPPC>
void Img2AxiMM(Img<PIXEL_T, H, W, STORAGE, NPPC> &InImg, AxiDT *OutAxiMM)
```
This functions converts an input `Img` object `InImg` to an output AXI Memory Map `OutAxiMM`.
It handles the width conversion and the packing/unpacking of data to convert between the AXI interface width (which has to be a power-of-2 bytes) and the more arbitrary pixel width of the `Img`.

If the input `InImg` uses `FIFO` `StorageType`, the input FIFO becomes empty after calling this function (or 1 frame less of pixels).

**Template parameters:**
- `AxiWordWidth`: The AXI interface's data width (in bits) of the memory `OutAxiMM`.
  This has to be a power-of-2 bytes, i.e., `AxiWordWidth / 8` has to be a power-of-2.
  - Examples: `32`, `64`.
- `AxiDT`: The C++ data type for the memory `OutAxiMM`.
  This is automatically inferred from `OutAxiMM`.
  The width of `AxiDT` has to match `AxiWordWidth`.
  - Examples: `uint32_t`, `uint64_t`.
- The other template parameters are automatically inferred from the `InImg` argument.

**Example usages:**
```cpp
#define NumPixels (WIDTH * HEIGHT)
#define NPPC 1
#define NumPixelWords (NumPixels / NPPC)
#define PixelWordWidth (8 * NPPC) // Image is 8UC1
#define AxiWordWidth 64           // AXI memory is 64-bit
#define NumAxiWords (NumPixelWords * PixelWordWidth / AxiWordWidth)

Img<vision::PixelType::HLS_8UC1, ...> InImg;
uint64_t OutAxiMM[NumPixelWords]
Img2AxiMM<AxiWordWidth>(InImg, OutAxiMM);
```

## Mixed interfaces
It is possible to configure the input and output of the top-level function to use different combinations of the above 4 interfaces using the associated pragma and template parameters.

For example, in the `RGB2GRAYTop()` top-level function below, the input argument `InImg` uses **Memory interface**, and the output `OutAxiMM` uses **AXI4 Initiator interface**.
```cpp
void RGB2GRAYTop(Img<vision::PixelType::HLS_8UC3, FRAME_BUFFER> &InImg,
                 uint32_t *OutAxiMM) {
#pragma HLS function top
#pragma HLS function dataflow
#pragma HLS memory partition argument(InImg) type(struct_fields)
#pragma HLS interface argument(OutAxiMM) type(axi_initiator)    \
    num_elements(OutNumWords) max_burst_len(256)
    Img<vision::PixelType::HLS_8UC1> OutImg;
    vision::RGB2GRAY(InImg, OutImg);
    vision::Img2AxiMM<32>(OutImg, OutAxiMM);
}
```

# AXI4-Stream Video Protocol

The traditional video signal format (`HSYNC`, `VSYNC`, etc.) presents several challenges for designing video processing cores and video interconnects.
These challenges include the need for specific durations of gaps between lines and frames during the blanking period,
and the requirement for consecutive pixel arrivals (without bubbles) during the active period.
In order to support this traditional video signal format, video processing cores must be designed to avoid back-pressure on video input to ensure real-time consumption.
Similarly, for video output, video processing cores must prevent bubbles during the active period and stop data transmission during blanking periods.
Such cycle-accurate requirement also makes integrating video processing cores more difficult.

To overcome the challenges, we propose to use an AXI4-Stream Video Protocol (AXIS-Video) to replace the traditional video signal format, when it comes to integrating video processing cores, especially when integrating with camera and display I/O components.
This protocol is based on the AXI4 Stream (AXIS) interface.
AXI4 Stream's `TValid` and `TReady` handshaking signals can indicate bubble and back-pressure during data transmission.
We use the `TLast` signal to represent End Of Line (EOL) and `TUser[0]` to represent Start Of Frame.
The protocol removes the cycle-accurate restrictions, allowing back-pressure during the active period,
and continuous processing/output during the blanking period.
The protocol also eliminates the need for processing cores to have knowledge of the active/blanking period.

The table below summarizes AXIS-Video signals.
For more information about the AXI protocol visit [AMBA 4 AXI4-Stream Protocol Specification](https://developer.arm.com/documentation/ihi0051/a/).

|Signal name|Description                                                                  |
|-----------|-----------------------------------------------------------------------------|
|TReady     | AXIS ready signal, for downstream to back-pressure                          |
|TValid     | AXIS valid signal, for upstream to indicate valid data vs bubbles           |
|TData      | Contains image data (pixels)                                                |
|TLast      | Represents End of Line (EOL), asserted for the last pixel of a line         |
|TUser      | Represents Start Of Frame (SOF), asserted for the first pixel of a frame    |

The Vision library defines AXIS-Video data type and AXIS-Video FIFO in [axis_video.hpp](./axis_video.hpp)) as follows:

```cpp
/**
 * AXI stream video protocol type -- the underlying data type inside the
 * hls::FIFO template.
 */
template <PixelType PIXEL_T, NumPixelsPerCycle NPPC = NPPC_1>
struct AxisVideoT {
    typename DT<PIXEL_T, NPPC>::T data;
    ap_uint<1> last, user;
};

/**
 * AXI stream video protocol FIFO type.  The FIFO type will have data, last
 * user, plus valid and ready.
 */
template <PixelType PIXEL_T, NumPixelsPerCycle NPPC = NPPC_1>
using AxisVideoFIFO = hls::FIFO<AxisVideoT<PIXEL_T, NPPC>>;
```

Top-level function arguments using the `AxisVideoFIFO` type will result in an AXI4-Stream Video Protocol interface.

## AXIS-Video vs. FIFO-based vision::Img AXI4 Stream interface

Both `AxisVideoFIFO` and [`vision::Img`](../common/README.md#img-class) with `FIFO` `STORAGE_TYPE` can be used for a top-level argument to generate an AXI4 Stream interface.
However, they are intended to be used in different circumstances,
- Both interfaces use the `TData` signal to transfer pixel data, and `TValid & TReady` signals for data transfer handshaking.
  The difference is the AXIS-Video interface generated from the `AxisVideoFIFO` type includes additional `TLast` and `TUser[0]` signals to indicate EOL and SOF.
- Typically we use AXIS-Video interface when interfacing the HLS cores with I/O components like camera and display components that rely on SOF and EOL signals.
  The SOF and EOL signals are useful to detect broken frames and restore after video frames become stable.
- The `Img` on the other hand is used by all the image processing functions in the HLS C++ library.
  The `Img` class permits more interface configurations (e.g., AXI4 Initiator, AXI4 Target, Memory interface) in addition to AXI4 Stream, as well as buffering options between processing modules.
  Typically there won't be broken frames between functions in a HLS pipeline.
  Almost in all cases, `Img` type is used for intermediate frames within a HLS pipeline.
  Moreover, `Img` class type has additional `width` and `height` fields to indicate frame resolution.
  The `FIFO` `STORAGE_TYPE` is preferred whenever possible (when pixels are accessed in sequence) to conserve resource usage (also see [StorageType](../common/README.md#storagetype)).
- In short, `AXIS-Video` is typically used for the HLS core's top-level interface for integrating with I/O components,
  and the `Img` type is more often used within an HLS pipeline.

To convert between `Img` and AXIS-Video interface, we provide the [`AxisVideo2Img()`](#axisvideo2img) and [`Img2AxisVideo()`](#img2axisvideo) functions in [axis_video.hpp](axis_video.hpp).
These functions are not limited to `FIFO` `STORAGE_TYPE` `Img` and support other storage types as well.

Similarly, to convert between AXI Memory Map and AXIS-Video interface, we provide the [`AxiMM2AxisVideo()`](#aximm2axisvideo) and [`AxisVideo2AxiMM()`](#axisvideo2aximm) functions in [axi_mm_intf.hpp](axi_mm_intf.hpp).
- For more information about the AXI Memory Map, please refer to the section [AXI4 Initiator interface and AXI4 Target interface](#axi4-initiator-interface-and-axi4-target-interface).

## `AxisVideo2Img()`

This function converts an incoming AXIS-Video interface `InVideo` to an `Img` object `OutImg`.
The function receives pixel from the AXIS-Video interface, and only starts writing pixels to `Img` when the SOF signal is received; pixels transmitted before SOF are dropped.
The function also returns the following status code:
- 0: OK, no error.
- 1: Early start of frame (SOF is received in the middle of a previous frame)
- 2: Early end of line (EOL is received in the middle of a previous line)
- 4: Late end of line (EOL is missing at the expected last pixel of a line)
The return status can include more than 1 error, for example, we can have both early start of frame and late end of line happening,
which would result in an error code of 5 (1 | 4 = 5).

```cpp
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
int AxisVideo2Img(AxisVideoFIFO<PIXEL_T, NPPC> &InVideo,
                  vision::Img<PIXEL_T, H, W, STORAGE, NPPC> &OutImg)
```

**Template parameters:**
- All template parameters are automatically inferred from the arguments.

## `Img2AxisVideo()`

This function converts an `Img` object `InImg` to an AXIS-Video interface `OutVideo`.
If the input `InImg` uses `FIFO` `StorageType`, the input FIFO becomes empty after calling this function (or 1 frame less of pixels).

```cpp
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE = FIFO,
          NumPixelsPerCycle NPPC = NPPC_1>
void Img2AxisVideo(vision::Img<PIXEL_T, H, W, STORAGE, NPPC> &InImg,
                   AxisVideoFIFO<PIXEL_T, NPPC> &OutVideo)
```

**Template parameters:**
- All template parameters are automatically inferred from the arguments.

## `AxiMM2AxisVideo()`
```cpp
template <unsigned AxiWordWidth, typename AxiDT, unsigned H, unsigned W,
          vision::PixelType PIXEL_T, vision::NumPixelsPerCycle NPPC>
void AxiMM2AxisVideo(AxiDT *InAxiMM, AxisVideoFIFO<PIXEL_T, NPPC> &OutVideo,
                     int HRes, int VRes)
```
This functions converts an input AXI Memory Map `InAxiMM` to an output AXIS-Video interface `OutVideo`.
Under the hood, it simply calls [`AxiMM2Img()`](#aximm2img) and [`Img2AxisVideo()`](#img2axisvideo) back-to-back.

**Template parameters:**
- `AxiWordWidth`: The AXI interface's data width (in bits) of the memory `InAxiMM`.
  This has to be a power-of-2 bytes, i.e., `AxiWordWidth / 8` has to be a power-of-2.
  - Examples: `32`, `64`.
- `AxiDT`: The C++ data type for the memory `InAxiMM`.
  This is automatically inferred from `InAxiMM`.
  The width of `AxiDT` has to match `AxiWordWidth`.
  - Examples: `uint32_t`, `uint64_t`.
- `H` and `W`: The maximum size of the video frame.
  They are conceptually the same as the `Img` class' template parameters [`H` and `W`](../common/README.md#h-and-w) and similarly, can be different from the arguments `VRes` and `HRes`.
- The other template parameters are automatically inferred from the `OutVideo` argument.

**Arguments:**
- `HRes` and `VRes`: The horizontal and vertical resolution of the video frame.
  They are conceptually the same as the `Img` class' member variables [`width` and `height`](../common/README.md#height-and-width) and similarly, can be different from the template parameters `W` and `H`.

**Example usages:**
```cpp
#define NumPixels (WIDTH * HEIGHT)
#define NPPC 4
#define NumPixelWords (NumPixels / NPPC)
#define PixelWordWidth (8 * NPPC) // Video frame is 8UC1
#define AxiWordWidth 64           // AXI memory is 64-bit
#define NumAxiWords (NumPixelWords * PixelWordWidth / AxiWordWidth)

uint64_t InAxiMM[NumPixelWords];
AxisVideoFIFO<HLS_8UC1, NPPC_4> OutVideo;
AxiMM2AxisVideo<AxiWordWidth, uint64_t, HEIGHT, WIDTH>(InAxiMM, OutVideo, WIDTH,
                                                       HEIGHT);
```

## `AxisVideo2AxiMM()`
```cpp
template <unsigned AxiWordWidth, typename AxiDT, unsigned H, unsigned W,
          vision::PixelType PIXEL_T, vision::NumPixelsPerCycle NPPC>
void AxisVideo2AxiMM(AxisVideoFIFO<PIXEL_T, NPPC> &InVideo, AxiDT *OutAxiMM,
                     int HRes, int VRes)
```
This functions converts an input AXIS-Video interface `InVideo` to an output AXI Memory Map `OutAxiMM`.
Under the hood, it simply calls [`AxisVideo2Img()`](#axisvideo2img) and [`Img2AxiMM()`](#img2aximm) back-to-back.

**Template parameters:**
- `AxiWordWidth`: The AXI interface's data width (in bits) of the memory `OutAxiMM`.
  This has to be a power-of-2 bytes, i.e., `AxiWordWidth / 8` has to be a power-of-2.
  - Examples: `32`, `64`.
- `AxiDT`: The C++ data type for the memory `OutAxiMM`.
  This is automatically inferred from `OutAxiMM`.
  The width of `AxiDT` has to match `AxiWordWidth`.
  - Examples: `uint32_t`, `uint64_t`.
- `H` and `W`: The maximum size of the video frame.
  They are conceptually the same as the `Img` class' template parameters [`H` and `W`](../common/README.md#h-and-w) and similarly, can be different from the arguments `VRes` and `HRes`.
- The other template parameters are automatically inferred from the `InVideo` argument.

**Arguments:**
- `HRes` and `VRes`: The horizontal and vertical resolution of the video frame.
  They are conceptually the same as the `Img` class' member variables [`width` and `height`](../common/README.md#height-and-width) and similarly, can be different from the template parameters `W` and `H`.

**Example usages:**
```cpp
#define NumPixels (WIDTH * HEIGHT)
#define NPPC 4
#define NumPixelWords (NumPixels / NPPC)
#define PixelWordWidth (24 * NPPC) // Video frame is 8UC3
#define AxiWordWidth 64            // AXI memory is 64-bit
#define NumAxiWords (NumPixelWords * PixelWordWidth / AxiWordWidth)

AxisVideoFIFO<HLS_8UC3, NPPC_4> InVideo;
uint64_t OutAxiMM[NumPixelWords];
AxisVideo2AxiMM<AxiWordWidth, uint64_t, HEIGHT, WIDTH>(InVideo, OutAxiMM, WIDTH,
                                                       HEIGHT);
```
