# SmartHLS DSP Library


<!-- TOC -->

- [SmartHLS DSP Library](#smarthls-dsp-library)
  - [Overview](#overview)
  - [Library Components](#library-components)
    - [Documentation Notes](#documentation-notes)

<!-- /TOC -->
## Overview

The SmartHLS DSP library provides HLS C++ library functions for algorithm prototyping of DSP cores.
The SmartHLS DSP library is designed to simplify the development of DSP application development on Microchip FPGA devices. The 
library provides pre-optimized C++ library functions for fast algorithm prototyping of common DSP applications such as FFT computation.
The DSP applications built with the DSP library functions can be easily ported onto multiple Microchip FPGA platforms with a faster time
to market thanks to the DSP functions in this DSP library.

By open sourcing the Vision library, we hope to deliver new features and fixes under a faster release cycle, and share the source code to serve as example for developers to implement other HLS designs. We welcome all developers to contribute by reporting issues, feedback, and feature requests (by raising "Issues"); and even better, contribute back new algorithms and fixes into this open-source repository.

## Library Components

Below is the directory structure of the DSP library and each sub-directory's respective content.
Please see the readme files in sub-directories for more details.

- [docs](./docs/): Documentation for the HLS C++ Library. See [documentation notes](#documentation-notes).
  - [graphs](./docs/graphs/): Graphs that are used in .md files
  - [md](./docs/md/): folder containing .md files for each DSP module
- [include](include/): The HLS C++ library.
- [utils](utils/): Python scripts that are used for various purposes.
  - [generators](./utils/generators/): Directory containing python scripts for auto-generation of test vectors and the twiddle factors
  - [doc_utils](./utils/doc_utils/): Documentation scripts
    - [python](./utils/doc_utils/python/): Directory holding python scripts for auto-doc generation
      - [graphing](./utils/doc_utils/python/graphing/): Directory containing scripts to generate graphs of a given test
      - [reporting](./utils/doc_utils/python/reporting/): Directory containing scripts to generate tables of a given test
      - [script_utils](./utils/doc_utils/python/script_utils/): Directory containing common python functions used by other modules
      - [upload_to_confluence](./utils/doc_utils/python/upload_to_confluence/): Directory containing scripts to update Confluence page
- [examples](examples/): Examples showcasing the usage of the HLS C++ library
  - [on_board](./examples/on_board/): Directory containing examples for each supported board
    - [icicle_kit](./examples/on_board/icicle_kit/): Directory containing examples for the Icicle Kit
      - [riscv_fft](./examples/on_board/icicle_kit/riscv_fft/): FFT demo designs working on Icicle Kit
  - [simple](./examples/simple/): Directory containing test programs for each DSP module
    - [fft](./examples/simple/fft/): Simple test program for FFT

### Documentation Notes

Documentation for each function in the HLS DSP Library can be found in [docs/md](./docs/md/).