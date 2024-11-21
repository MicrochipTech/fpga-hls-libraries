

## Prerequisites:
You should install the following software:
   1. The latest version of Libero & SmartHLS
   2. The device tree compiler (`dtc` command)
   
      ```bash
      sudo apt-get install device-tree-compiler
      ```
   3. The following python packages:
      
      ```bash
      python -m pip install GitPython
      python -m pip install requests
      python -m pip install PyYAML
      ```

4. You will need the following hardware: [BeagleV-Fire board](https://www.beagleboard.org/boards/beaglev-fire).  
   * Make sure Ubuntu boots-up properly and that the board is connected to a local network.
   * You can use the serial terminal program (e.g. `tio`) to find your board's IP address. You can type:

   ```bash
   $host> tio /dev/ttyACM0 -b 115200
   $beagle> ifconfig eth0
   eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
      inet 192.168.0.173  netmask 255.255.255.0  broadcast 192.168.0.255
      inet6 fe80::204:a3ff:fefb:406f  prefixlen 64  scopeid 0x20<link>
      ether 00:04:a3:fb:40:6f  txqueuelen 1000  (Ethernet)
      RX packets 17882  bytes 3231759 (3.0 MiB)
      RX errors 0  dropped 4843  overruns 0  frame 0
      TX packets 4963  bytes 1478718 (1.4 MiB)
      TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
      device interrupt 18
   ```

   In this example, 192.168.0.173 is the board's IP address. This may be different for your board.

## Environment Setup

1. First, make sure that the [`support/beaglev-fire/gateware`](../../../../support/beaglev-fire/gateware) directory contains the Beagleboard gateware repo. If this directory is empty, run the following command to fetch and check out the Beagleboard gateware submodule:

   ```
   git submodule update --init --recursive 
   ```

2. You will need to set the following environment variables as required by the 
   beagleboard compilation flow:

   ```bash
   export LIBERO_INSTALL_DIR=/path/to/Libero_SoC_v2024.2
   export FPGENPROG=$LIBERO_INSTALL_DIR/Libero/bin64/fpgenprog
   export SC_INSTALL_DIR=/path/to/SoftConsole-v2022.2-RISC-V-747
   export LD_LIBRARY_PATH=/usr/lib/i386-linux-gnu:$LD_LIBRARY_PATH
   export PATH=$LIBERO_INSTALL_DIR/Libero/bin:$LIBERO_INSTALL_DIR/Libero/bin64:$PATH
   export PATH=$SC_INSTALL_DIR/riscv-unknown-elf-gcc/bin:$PATH
   ```

## Running the Example

1. First step is to generate the hardware required to run the design

   ```bash
   cd support/beaglev-fire/gateware
   python build-bitstream.py ../../../math/examples/riscv_tests/sin_performance/beaglev_fire/sin_performance.yaml
   ```

   This `.yaml` configuration file makes it so that SmartHLS's open-source library, [fpga-hls-libraries](https://github.com/MicrochipTech/fpga-hls-libraries), will be cloned byt the python script in order to get the design files before starting to run the Libero flow. Then, as part of the project generation step, it will:
   * generate the SmartHLS hardware module (compile the C++ source code to Verilog)
   * integrate the hardware module into the Libero design
   * compile the RISC-V binary executable (.elf file) to run on board
   * generate the bitstream

2. Once you have generated the bitstream, copy the `bitstream` folder over to the board, and program the generated bitstream and associated .dtbo files to the board using the [reprogramming](https://docs.beagleboard.org/latest/boards/beaglev/fire/demos-and-tutorials/gateware/upgrade-gateware.html#launch-reprogramming-of-beaglev-fire-s-fpga) script, `/usr/share/beagleboard/gateware/change-gateware.sh`. 
   
   E.g., run:
   ```
   scp -r <PATH TO GATEWARE REPO>/bitstream beagle@192.168.0.173

   ssh beagle@192.168.0.173
   sudo su root
   /usr/share/beagleboard/gateware/change-gateware.sh ~/bitstream

   ```
3. Now, copy the RISC-V binary (`sin_performance.accel.elf` file) to your board:

   ```console
   scp ../../../math/examples/riscv_tests/sin_performance/beaglev_fire/hls_output/sin_performance.accel.elf beagle@192.168.0.173:
   ```

4. To run the executable, you will need to be running as `sudo`. Go into your board and run the binary you just copied over as sudo.
   ```
   ssh beagle@192.168.0.173

   sudo ./sin_performance.accel.elf
   ```

   You should see something like this:
   ```
   Passed! Times: cmath: 0.004770 s, hls_math: 0.000616 s
   ```
   The exact times it takes to run will vary.

