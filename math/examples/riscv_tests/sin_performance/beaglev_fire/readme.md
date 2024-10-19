

## Prerequisites:
You should install the following software:
1. The latest version of Libero & SmartHLS

You will need the following hardware:
1. A [BeagleV-Fire board](https://www.beagleboard.org/boards/beaglev-fire)

## Environment Setup

1. First, make sure that the [`support/beaglev-fire/gateware`](../../../../support/beaglev-fire/gateware) directory contains the Beagleboard gateware repo. If this directory is empty, run the following command to fetch and check out the Beagleboard gateware submodule:

```
git submodule update --init --recursive 
```

2. You will need to set the following environment variables:
   * BOARD_IP: the IP of the target BeagleV-Fire board
      * If you haven't set an IP address for the board, do it now.
   * LIBERO_INSTALL_DIR: your installation of Libero
   * FPGENPROG: your installation of fpgenprog (it's packaed with Libero)
   * SC_INSTALL_DIR: your installation of SoftConsole
   * LD_LIBRARY_PATH should also have /usr/lib/i386-linux-gnu
   * PATH must include $SC_INSTALL_DIR/riscv-unknown-elf-gcc/bin

3. Next, install the following python packages:
   * GitPython
   * requests
   * PyYAML

:exclamation: You can automate steps 2 and 4 by running the following script:

```
#!/bin/bash

export LIBERO_INSTALL_DIR=/captures/Libero_SoC_v2023.2
export FPGENPROG=$LIBERO_INSTALL_DIR/Libero/bin64/fpgenprog
export SC_INSTALL_DIR=/captures/SoftConsole-v2022.2-RISC-V-747

if ! [ -x "$(which dtc)" ]; then
        echo "dtc not found, you may need to install it."
fi

export LD_LIBRARY_PATH=/usr/lib/i386-linux-gnu:$LD_LIBRARY_PATH
export PATH=/captures/Libero_SoC_v2023.2/Libero/bin:/captures/Libero_SoC_v2023.2/Libero/bin64:$SC_INSTALL_DIR/riscv-unknown-elf-gcc/bin:$PATH

python -m pip install GitPython
python -m pip install requests
python -m pip install PyYAML

```

4. Next, install `dtc` (if not already installed)


## Running the Example

1. First, go to the `support/beaglev-fire/gateware` directory, and run `python build-bitstream.py ../../math/examples/riscv_tests/sin_performance/beaglev_fire.yaml` to generate the hardware required to run the design. This configuration file makes it so that SmartHLS's open-source library, [fpga-hls-libraries](https://github.com/MicrochipTech/fpga-hls-libraries), will be cloned in order to get the design files before starting to run the Libero flow. Then, as part of the project generation step, it will generate the SmartHLS hardware module (compile the C++ source code to Verilog), generate a RISC-V binary executable (.elf file) to run on board, and then integrate the hardware generated module into the Libero design. The rest of the bitstream generation flow proceeds as normal.

2. Find your board's IP address. To check, on your board, in the terminal, type `ifconfig`:
```
$ ifconfig eth0
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

3. Once you have generated the bitstream, copy the `bitstream` folder over to the board, and program the generated bitstream and associated .dtbo files to the board using the [reprogramming](https://docs.beagleboard.org/latest/boards/beaglev/fire/demos-and-tutorials/gateware/upgrade-gateware.html#launch-reprogramming-of-beaglev-fire-s-fpga) script, `/usr/share/beagleboard/gateware/change-gateware.sh`. E.g., run:
```
scp -r <PATH TO GATEWARE REPO>/bitstream beagle@192.168.0.173

ssh beagle@192.168.0.173
sudo su root
/usr/share/beagleboard/gateware/change-gateware.sh ~/bitstream
```

4. Now, go to back to the directory this file is in (`math/examples/riscv_tests/sin_performance/beaglev_fire/build`.) Copy the .accel.elf file to your board:
```
scp hls_output/sin_performance.accel.elf beagle@192.168.0.173:
```

5. To run the executable, you will need to be running as `sudo`. Go into your board and run the binary you just copied over as sudo.
```
ssh beagle@192.168.0.173

sudo ./sin_performance.accel.elf
```

You should see something like this:
```
Passed! Times: cmath: 0.004770 s, hls_math: 0.000616 s
```
The exact times it takes to run will vary.

