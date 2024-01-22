# SmartHLS Vision Library

<!-- TOC -->

  - [Overview](#overview)
  - [Library Components](#library-components)
  - [Setup](#setup)
  - [Getting Started](#getting-started)

<!-- /TOC -->
## Overview

The SmartHLS Vision Library is designed to simplify the development of video processing solutions on Microchip FPGA devices.
The library provides pre-optimized HLS C++ library functions for fast algorithm prototyping of video applications.
OpenCV-based designs can be ported onto FPGAs with a faster time to market thanks to the equivalent functions in this Vision library.

Configuring the RTL interface of the HLS-generated Video IP is easy with the Vision library, requiring no changes to the C++ algorithm code.
The developer can switch the RTL interface between AXI initiator (M), AXI target (S), AXI stream, and a generic RAM interface with just 1 or 2 lines of changes.

The Vision library also aims to make the verification process easy.
Developers can use OpenCV and the provided test utility functions to create the software testbench,
making it convenient to verify the HLS C++ implementation against OpenCV reference in software.
Once the software implementation is verified,
SmartHLS' Software/Hardware Co-Simulation feature can automatically generate an RTL testbench and verify the correctness of HLS-generated IP in RTL simulation.

The library also offers RTL modules for interfacing with I/O, specifically the camera and display modules.
The RTL modules use an AXI-stream-based video interface protocol, allowing easy system integration with HLS IPs.

By open sourcing the Vision library, we hope to deliver new features and fixes under a faster release cycle,
and share the source code to serve as example for developers to implement other HLS designs.
We welcome all developers to contribute by reporting issues, feedback, and feature requests (by raising "Issues");
and even better, contribute back new algorithms and fixes into this open-source repository.

## Library Components

Below is the directory structure of the Vision library and each sub-directory's respective content.
Please see the readme files in sub-directories for more details.

- [include](include/): The HLS C++ library.
- [rtl](rtl/): RTL components of the library, specifically the AXI-stream based video protocol interface adapters for camera and display modules, as well as pre-canned SmartDesign components for camera and display.
- [demo_designs](demo_designs/): Complete demo designs working on FPGA boards (e.g., [PolarFire Video Kit](https://www.microchip.com/en-us/development-tool/MPF300-VIDEO-KIT-NS)), making uses of the HLS C++ library and RTL components.
- [examples](examples/): Examples showcasing the usage of the HLS C++ library.
- [tests](tests/): Tests to verify the functionality of HLS library under different configurations.
- [media_files](media_files/): All media files such as images and videos, tracked under LFS.

## Setup

- The SmartHLS Vision library supports using OpenCV functionality in the software testbench of a SmartHLS project.
- We have included a precompiled OpenCV library at [precompiled_sw_libraries](precompiled_sw_libraries) (see [here](https://onlinedocs.microchip.com/oxy/GUID-AFCB5DCC-964F-4BE7-AA46-C756FA87ED7B-en-US-11/GUID-319B3DF4-74AD-49B5-A101-BE2D33FFA900.html) for more details.)

## Getting Started

After you installed the SmartHLS tool and Libero SoC design suite and set up the repository as described above, you can take a look at the [demo design](demo_designs/PF_Video_kit/README.md) targeting the [PolarFire Video Kit](https://www.microchip.com/en-us/development-tool/MPF300-VIDEO-KIT-NS).
