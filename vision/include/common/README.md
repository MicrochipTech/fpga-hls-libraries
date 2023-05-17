<!-- TOC -->

- [Img class](#img-class)
    - [Template Parameters](#template-parameters)
        - [PixelType](#pixeltype)
        - [StorageType](#storagetype)
        - [H and W](#h-and-w)
        - [NPPC](#nppc)
    - [Class member variables](#class-member-variables)
        - [height and width](#height-and-width)
        - [data](#data)
    - [Class member functions](#class-member-functions)
        - [unsigned get_height()](#unsigned-get_height)
        - [unsigned get_width()](#unsigned-get_width)
        - [void set_height(unsigned h)](#void-set_heightunsigned-h)
        - [void set_width(unsigned w)](#void-set_widthunsigned-w)
        - [DATA_T_ read(unsigned idx)](#data_t_-readunsigned-idx)
        - [void write(DATA_T_ val, unsigned idx)](#void-writedata_t_-val-unsigned-idx)
        - [void set_fifo_depth(unsigned fifo_depth)](#void-set_fifo_depthunsigned-fifo_depth)
    - [Constructors](#constructors)
        - [Img()](#img)
        - [Img(unsigned height_, unsigned width_)](#imgunsigned-height_-unsigned-width_)
        - [Img(unsigned height_, unsigned width_, unsigned fifo_depth)](#imgunsigned-height_-unsigned-width_-unsigned-fifo_depth)
- [OpenCV utilities](#opencv-utilities)
- [Test utilities](#test-utilities)
    - [Pattern Generator](#pattern-generator)
- [Line buffer](#line-buffer)

<!-- /TOC -->



# `Img` class
The `Img` class represents a 2-dimensional image frame, and is a core class in the C++ Vision library. It is functionally very similar to the [OpenCV's Mat class](https://docs.opencv.org/4.5.4/d3/d63/classcv_1_1Mat.html).

The `Img` class is defined in [common.hpp](common.hpp).

## Template Parameters
The Img class is highly parameterizable with C++ template parameters. You can configure the pixel type, maximum height and width of the frame, the underlying storage type of the frame data, and the number of pixels per cycle, as seen in the class definition below:
```cpp
template <PixelType PIXEL_T, unsigned H, unsigned W, StorageType STORAGE = FIFO, NPPC = NPPC_1> class Img;
```
For example, you can instantiate an `Img` object like the following:
```cpp
using namespace hls;
...
vision::Img<vision::PixelType::HLS_8UC1, 1920, 1080, vision::StorageType::FIFO, vision::NPPC_1> MyImg;
```

For more information about the different values that each template parameter can take, you can look at [params.hpp](params.hpp).

### `PixelType`
This template parameter is very similar to the type seen in OpenCV. For example, `HLS_8UC1` represents 8-bit, unsigned, 1 channel pixel.

Within the `Img` class, the pixel data type is stored as an `ap_[u]int` of size `channel_width * num_channel * NPPC`, where `NPPC` means the number of pixels per cycle (see [`NPPC`](#nppc)).
For example, a `HLS_8UC1` pixel with `NPPC_4` will be stored as an `ap_uint<32>`.

For more information about the `ap_[u]int` class, please refer to the SmartHLS User Guide: [C++ Arbitrary Precision Data Types Library](https://microchiptech.github.io/fpga-hls-docs/userguide.html#ap-lib).

### `StorageType`
This template parameter specifies the underlying data structure of the frame data. It can have one of the following values:
- `FRAME_BUFFER`: the frame is stored inside a C++ array.
- `FIFO`: the frame is stored inside a [`hls::FIFO`](https://microchiptech.github.io/fpga-hls-docs/userguide.html#streaming-lib) object.
  - If this `Img` object is in a hardware function (i.e. top-level functions and their descendants), the FIFO will have a default depth of 2, but this FIFO depth can be configured by using the constructor [`Img(unsigned height_, unsigned width_, unsigned fifo_depth)`](#imgunsigned-height_-unsigned-width_-unsigned-fifo_depth), or by using the class member function [`set_fifo_depth(unsigned fifo_depth)`](#void-set_fifo_depthunsigned-fifo_depth).
  - If this `Img` object is in a software function (software testbench functions), then the FIFO depth is automatically set to `H * W`, i.e., the FIFO needs to be large enough to hold the entire frame.

  Note:
  - The `FIFO` storage type should only be used if you can ensure that the frame data can be read or written pixel-by-pixel in a sequential order.
  - As a starting point, a FIFO depth of `H * W` is guaranteed to be sufficient for functional correctness.
    - However, for hardware functions, you would want to set the FIFO depth to something smaller to save resource.
      A smaller FIFO depth is still sufficient for functional correctness if you use the SmartHLS [Data Flow Parallelism](https://microchiptech.github.io/fpga-hls-docs/userguide.html#data-flow-parallelism) feature. Under data flow parallelism, data can flow in and out of the top-level function all the time. As a result, the FIFO inside the `Img` object doesn't need to be too large.
      The exact choice for the FIFO depth that guarantees both functional correctness and good resource utilization depends on your specific application. For an example, please see our implementation of the [Canny Edge Detection Algorithm](../imgproc/canny.hpp).
    - For software functions, since there is no dataflow parallelism, the FIFO needs to be large enough to hold the entire frame, so the FIFO depth is automatically set to `H * W`.
  - The `FIFO` storage type generally provides better resource utilization than `FRAME_BUFFER` especially at low FIFO depth. For example, for a 1920x1080 frame:
    - `FRAME_BUFFER` storage type will require enough memory to store 2,073,600 pixels.
    - `FIFO` storage type will only require the memory to store "FIFO_depth" pixels (the default value is 2 for hardware functions)
    - This resource usage description only applies to the `vision::Img` objects instantiated inside the top-level functions, but does not apply to the top-level function arguments (see below point).
- In situations where the `Img` object is used as an argument of a top-level function, the StorageType can be used to configure the top-level RTL interface to be either **AXI4 Stream interface** for `FIFO`, or **Memory interface** for `FRAME_BUFFER`. For more information, please see [Configure Top-level RTL interface for `Img` arguments](../interface/README.md#configure-top-level-rtl-interface-for-img-arguments).

### `H` and `W`
These two template parameters represent the maximum size of the image frame that the `Img` object can hold.
Note that these are not necessarily the same as the actual size of the image frame (for example, in a real-time video-processing application, the size of individual image frame of the video can vary over time).

The only requirement for these two template parameters is that `H` and `W` are larger than or equal to the actual height and width of any given frame throughout the whole video-processing application.

### `NPPC`
This template parameter specifies the number of pixels per clock cycle.
For example, when processing 4K resolution frames at 30 FPS, we will often use an `NPPC` of 4 in order to meet the desired real time data rate.
By using an `NPPC` of 4, every four pixels is packed together as one word, and our library functions working on the `Img` will receive/process/transmit all four pixels at once (typically in each clock cycle).

## Class member variables
### `height` and `width`
These two variables represent the actual size of an image frame at a given moment. As explained above,
they are not necessarily the same as the template parameters [`H` and `W`](#h-and-w), which represent the largest possible image frame.

The values of these two variables can be accessed and modified by using the class member functions [`get_height()`](#unsigned-get_height), [`get_width()`](#unsigned-get_width), [`set_height()`](#void-set_heightunsigned-h), and [`set_width()`](#void-set_widthunsigned-w).

### `data`
This variable holds the pixel values of the current image frame.
The type of this variable varies depending on the template parameter `StorageType`.

For example, considering the case where we are trying to store a 1920x1080 `HLS_8UC1` image frame inside an `Img` object, with `NPPC_1`.
The pixel value will have the type `ap_uint<8>` in this case.
- If `StorageType` is `FRAME_BUFFER`, then `data` is an 1-dimensional array of size `H * W / NPPC`, i.e., `ap_uint<8>[1920 * 1080 / 1]` data.
- If `StorageType` is `FIFO`, then `data` is a `hls::FIFO<ap_uint<8>>`, with a configurable FIFO depth.

Note that you should not access the underlying `data` variable directly.
To access and modify the underlying pixel value of the current image frame, please use the class member functions [`read()`](#data_t_-readunsigned-idx), and [`write()`](#void-writedata_t_-val-unsigned-idx).

## Class member functions
### `unsigned get_height()`
Get the height of the current image frame.

### `unsigned get_width()`
Get the width of the current image frame.

### `void set_height(unsigned h)`
Set the height of the current frame to the value `h`.

### `void set_width(unsigned w)`
Set the width of the current frame to the value `w`.

### `DATA_T_ read(unsigned idx)`
Read the pixel value at index `idx` of the image frame.
Here, `DATA_T_` denotes the underlying `ap_[u]int` data type for the pixel value.
- If `StorageType` is `FRAME_BUFFER`, `idx` denotes the pixel position of row `row` and column `col`.
  The `row` and `col` indices are taken from top-left to bottom right, and `0 <= row <= Img.get_height() - 1`, and `0 <= col <= Img.get_width() - 1`.
  We have `idx = row * Img.get_width() + col`.

  For example, for a 1920x1080 `Img`, where `W = 1920`:

  | Pixel position | row  | col  | idx                |
  |----------------|------|------|--------------------|
  | Top-left       | 0    |    0 |    0 * 1920 +    0 |
  | Top-right      | 0    | 1919 |    0 * 1920 + 1919 |
  | Bottom-left    | 1079 |    0 | 1079 * 1920 +    0 |
  | Bottom-right   | 1079 | 1919 | 1079 * 1920 + 1919 |

- If `StorageType` is `FIFO`, `idx` is ignored.
  In this case, the function can also be run without the `idx` argument, i.e., `read()`.
  - Only the pixel data at the head of the FIFO can be read.
  - Once read, that pixel value is popped out of the FIFO.

### `void write(DATA_T_ val, unsigned idx)`
Write the value of `val` to the pixel at index `idx` of the image frame.
Here, `DATA_T_` denotes the underlying `ap_[u]int` data type for the pixel value.
- If `StorageType` is `FRAME_BUFFER`, `idx` denotes the pixel position of row `row` and column `col`.
  In other words, `idx = row * W + col`, where `W` is the template parameter of the `Img` object.
- If `StorageType` is `FIFO`, `idx` is ignored.
  In this case, the function can also be run without the `idx` argument, i.e., `write(DATA_T_ val)`.
  - Once a pixel value is written, that pixel value is pushed to the tail end of the FIFO.
  - It is your responsibility to ensure the data in the FIFO follows the correct pixel order of the image frame.

### `void set_fifo_depth(unsigned fifo_depth)`
- If the `StorageType` is `FIFO`: If this `Img` object is in a hardware function, set the FIFO depth to `fifo_depth`.
  Otherwise, the FIFO depth is automatically set to `H * W`.
- If the `StorageType` is `FRAME_BUFFER`: This function will do nothing since there is no FIFO object inside the `Img`.

## Constructors
Note that regardless of the constructor you used to instantiate the `Img` object, you still need to provide all the template parameters as specified above.
### `Img()`
This is the default constructor.
It sets the class member variables `height` and `width` to be the same as the template parameter [`H` and `W`](#h-and-w).

If `StorageType` is `FIFO`, the default constructor sets the FIFO depth to 2.

### `Img(unsigned height_, unsigned width_)`
This constructor sets the class member variables `height` and `width` to be the value of the arguments `height_` and `width_`.

If `StorageType` is `FIFO`, this constructor sets the FIFO depth to 2.

### `Img(unsigned height_, unsigned width_, unsigned fifo_depth)`
This constructor sets the class member variables `height` and `width` to be the value of the arguments `height_` and `width_`.

If `StorageType` is `FIFO`, this constructor also sets the FIFO depth to `fifo_depth`.
Otherwise, if `StorageType` is `FRAME_BUFFER`, this constructor will do nothing extra.

# OpenCV utilities
The OpenCV utility functions are defined in [opencv_utils.hpp](opencv_utils.hpp).
These functions are not meant to be synthesized by SmartHLS. Rather, they are intended to be used in software testbench.

In the SmartHLS' software testbench, it is useful to leverage OpenCV APIs to read/write image files (e.g., cv::imread), or to get a reference implementation for the desired algorithm.
For convenience, we provide these two functions to convert image frames between this Vision library's `vision::Img` object and OpenCV's `cv::Mat` object,
- `convertToCvMat(Img<PIXEL_T, H, W, STORAGE, NPPC> &InImg, Mat &OutMat)`
- `convertFromCvMat(Mat &InMat, Img<PIXEL_T, H, W, STORAGE, NPPC> &OutImg)`

To confirm functional correctness in software, we typically compare the HLS C++ implementation output image against a golden image, which may be generated by an OpenCV reference implementation or loaded from an image file.
For the convenience in comparing the images, these two comparison functions are provided:
- `float compareMat(Mat &A, Mat &B, double Threshold)` reports the percentage of pixels having 1 or more channels with greater-than-Threshold differences.
- `template <typename T> float compareMatAndReport(Mat &A, Mat &B, double Threshold)` has the same functionality but in addition reports the location and values of pixels having greater-than-Threshold differences.
  - The template parameter `typename T` specifies the C++ data type to print the pixel value of the image as.
    You should choose a C++ data type that matches the [`PixelType`](#pixeltype) parameter of the `Img`.

    Some examples usages are included below. Note that `cv::Vec3b` is an OpenCV data type that can be used to represent a 3-channel pixel. 
    - `HLS_8UC1`:  `compareMatAndReport<unsigned char>(...)`
    - `HLS_8UC3`:  `compareMatAndReport<cv::Vec3b>(...)`
    - `HLS_16SC1`: `compareMatAndReport<int16_t>(...)`

# Test utilities
The test utility functions are defined in [test_utils.hpp](test_utils.hpp) and target testing and analyzing image/video processing hardware designs.

## Pattern Generator
```cpp
template <int Format = 0, PixelType PIXEL_T, unsigned H, unsigned W,
          StorageType STORAGE = FIFO, NumPixelsPerCycle NPPC = NPPC_1>
void PatternGenerator(vision::Img<PIXEL_T, H, W, STORAGE, NPPC> &ImgOut)
```

The `PatternGenerator` function generates fixed RGB image frames that can serve as input data for other functions.
The `PatternGenerator` is similar to the [Pattern Generator IP core](https://www.microchip.com/en-us/products/fpgas-and-plds/ip-core-tools/pattern-generator)
and can be used for analyzing and troubleshooting display or video processing pipelines. 
Unlike the Pattern generator IP core, the `PatternGenerator` function supports multiple pixels per clock,
making it suitable for analyzing pipelines with a wide range of configurations. 
The output image produced by this function is determined by the `Format` template argument, which can be set to one of the following options:

 * 0: Vertical colored lines
 * 1: Horizontal colored lines
 * 2: Angled orange lines over horizontal colored lines
 * 3: Angled orange lines over vertical  colored lines
 * 4: Checker board pattern

![pattern_gen](./doc_images/pattern_gen.png)

# Line buffer
The LineBuffer class is responsible for implementing the line buffer structure, 
which is frequently used in image convolution (filtering) operations. In these operations,
a filter kernel is moved across an input image, and it is applied to a local window of pixels (often in a square shape) at each new sliding location.
As this process occurs, the line buffer receives a new pixel at each new sliding location,
while retaining the pixels from previous image rows that can be included in the sliding window.
The Vision library's line buffer is an enhancement of the existing [HLS line buffer](https://microchiptech.github.io/fpga-hls-docs/userguide.html#line-buffer-user-guide),
with the added advantage of supporting multiple pixels per clock cycle.
The user can use the `AccessWindow(x, y, k)` function to access x, y coordinates in a window where the centroid is at the k-th pixel out of `Number of Pixels Per Clock (NPPC)`.
 
For a given NPPC,the line buffer maintains NPPC number of `window size` x `window size` windows.
For example, if NPPC is 4 and window size is 3, the windows array will have the following 9 words, each word containing 4 pixels.

  `A0.A1.A2.A3` &nbsp;&nbsp;&nbsp;&nbsp;  `B0.B1.B2.B3` &nbsp;&nbsp;&nbsp;&nbsp;  `C0.C1.C2.C3`

  `D0.D1.D2.D3` &nbsp;&nbsp;&nbsp;&nbsp;  `E0.E1.E2.E3` &nbsp;&nbsp;&nbsp;&nbsp;  `F0.F1.F2.F3`

  `G0.G1.G2.G3` &nbsp;&nbsp;&nbsp;&nbsp;  `H0.H1.H2.H3` &nbsp;&nbsp;&nbsp;&nbsp;  `I0.I1.I2.I3`

The user algorithm should be processing the "receptive" fields for the 4
pixels in word `E`.  So calling
  - `AccessWindow(x, y, 0)` returns pixels in the receptive field for E0.
  - `AccessWindow(0, 0, 0)` gives A3
  - `AccessWindow(1, 1, 0)` gives E0
  - `AccessWindow(2, 2, 0)` gives H1
  - `AccessWindow(x, y, 2)` returns pixels in the receptive field for E2
  - `AccessWindow(2, 0, 2)` gives B3
