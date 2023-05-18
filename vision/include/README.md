This directory contains the SmartHLS C++ Vision library.

- The [`common`](common/) folder contains the definition of the `vision::Img` class that represents an image frame, as well as the additional parameterization and utility functions.
- The [`imgproc`](imgproc/) folder contains all the image processing functions, such as Sobel edge detection, DeBayer and etc.
- The [`interface`](interface/) folder contains the C++ definition of the AXI-stream-based video protocol interface (`vision::AxisVideoT`), as well as the HLS implementation of DDR data movers from/to a variety of supported interfaces.
