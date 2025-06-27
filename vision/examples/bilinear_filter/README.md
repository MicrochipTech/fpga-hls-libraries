# Introduction

Bilinear filtering is a resampling and interpolation method used in image processing to estimate the intensity of a pixel at a non-integer coordinate by linearly blending the values of its four nearest neighbors in a 2D grid.

# Mechanism

In general, Given a non-integer sampling coordinate `(x,y)` in the source image:

1.Find the 2×2 nearest pixel grid surrounding `(x,y)`:

Top-left: `P00 = I(⌊x⌋,⌊y⌋)`

Top-right: `P01 = I(⌊x⌋+1,⌊y⌋)`

Bottom-left: `P10 = I(⌊x⌋,⌊y⌋+1)`

Bottom-right: `P11 = I(⌊x⌋+1,⌊y⌋+1)`

Compute the fractional offsets:

`Δx=x−⌊x⌋`
`Δy=y−⌊y⌋`

Blend along x-axis (horizontal):

Top: `T=(1−Δx)⋅P00 + Δx⋅P01`

Bottom: `B=(1−Δx)⋅P10 + Δx⋅P11`​

Blend along y-axis (vertical):

Result: `I(x,y)=(1−Δy)⋅T+Δy⋅B`

# Limitation

The current implementation of `hls::vision::BilinearFilter( )` has the following characteristics:

- support only one channel (i.e. grayscale images)
- support only NPPC = 1
- support only 8 bits per channel
- support both FRAME_BUFFER and FIFO as the underlying image storage type

# Upscaling 

The `hls::vision::BilinearFilter()` supports image upscaling. Given the following `toronto_640x480_grayscale.jpg`, we upscale the image by 3 times to obtain an bilinear-interpolated image `hls_output_1920x1440.png` in 1920 x 1440 format.


To configure the scaling factor, we modify `SCALE_X` and `SCALE_Y` macros in `bilinear_filter.cpp` as follows:
```
#define SCALE_X                         3
#define SCALE_Y                         3
```

Input Image:

![toronto_640x480_grayscale.jpg](./../../media_files/test_images/toronto_640x480_grayscale.jpg)

Output Image:

![hls_output_1920x1440.png](./hls_output_1920x1440.png)


# Downscaling

The `hls::vision::BilinearFilter()` supports image upscaling. Given the following `toronto_640x480_grayscale.jpg`, we downscale the image by 4 times to obtain an bilinear-interpolated image `hls_output_320x240.png` in 320 x 240 format.

To configure the scaling factor, we modify `SCALE_X` and `SCALE_Y` macros in `bilinear_filter.cpp` as follows:
```
#define SCALE_X                         0.25
#define SCALE_Y                         0.25
```

Input Image:

![toronto_640x480_grayscale.jpg](./../../media_files/test_images/toronto_640x480_grayscale.jpg)

Output Image:

![hls_output_320x240.png](./hls_output_320x240.png)