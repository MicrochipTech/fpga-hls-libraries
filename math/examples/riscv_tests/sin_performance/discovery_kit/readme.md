## Overview
Meant as a sanity check, this example runs through every function in the math library and compares it to the cmath implementation. 

## Prerequisites:
You should install the following software:
1. The latest version of Libero & SmartHLS
2. The [Discovery Kit Linux Image](https://github.com/polarfire-soc/meta-polarfire-soc-yocto-bsp/releases/tag/v2024.06)
3. The [pre-generated FPGA programming job file](https://github.com/polarfire-soc/polarfire-soc-discovery-kit-reference-design/releases/download/v2024.06/MPFS_DISCOVERY.zip)

You will need the following hardware:
1. A [PolarFire SoC Discovery Kit](https://www.microchip.com/en-us/development-tool/mpfs-disco-kit)
2. A microSD card (In our experience, the ones that work best are the 16GB storage cards.)

## Board Setup

Before running this SmartHLS example, the board needs to be prepared by performing the 
following tasks using the files under the `precompiled` directory:

1. Flash the precompiled Linux image (`core-image-minimal-de-mpfs-disco-kit-*.rootfs.wic.gz
`)
2. Program the precompiled bitstream (`MPFS_DISCOVERY_KIT.job`)
3. Have a network IPv4 address assigned to the board
4. (If you are using a Linux host PC) Follow the instructions [here](https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/reference-designs-fpga-and-development-kits/mpfs-discovery-kit-user-guide.md#default-jumper-settings) to ensure the COM ports can be seen by the host.

If you already have set up the Discovery Kit, skip to [Compiling the SmartHLS Module and Libero Project](#compiling-the-smarthls-module-and-libero-project).

### Flashing the Linux Image

1. To flash the Linux image you downloaded in the prerequisites section to the microSD card by following the instructions here: [SD card content update procedure](https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/reference-designs-fpga-and-development-kits/updating-mpfs-kit.md#sd-card-content-update-procedure).

2. Make sure the jumper settings are correct according to [Discovery Kit User Guide](https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/reference-designs-fpga-and-development-kits/mpfs-discovery-kit-user-guide.md#default-jumper-settings).

3. Insert the SD card into the board.

### Program the Precompiled Bitstream

Program the board using the [pre-generated FPGA programming job file](https://github.com/polarfire-soc/polarfire-soc-discovery-kit-reference-design/releases/download/v2024.06/MPFS_DISCOVERY.zip) you downloaded in the prerequisites section using Microchip's FPExpress.

### Setup Static IP Address 

Follow these steps after the Linux image has been flashed onto the board and you've connected the board to the ethernet.

On your board, run:
```
sudo vim /etc/systemd/network/70-static-eth0.network
```

and add the following content to the file (adjust as necessary):
``` 
[Match]
Name=eth0
 
[Network]
Address=192.168.2.1/24
Gateway=192.168.2.100
DCHP=no
```

The gateway can be the IP address of your host PC. Take note of the IP address you have set for your board. Then restart the network by running:

```
sudo systemctl restart systemd-networkd
networkctl status
```

You can test the connection by trying to ssh into the board:

```
ssh root@192.168.2.1
```

## Compiling the SmartHLS Module and Libero Project

1. First, make sure that the [`support/discovery_kit/discovery-kit-reference-design`](../../../../support/discovery_kit/discovery-kit-reference-design) directory contains the Discovery Kit reference design. If this directory is empty, run
the following command to fetch and check out the Discovery Kit Reference Design submodule:

```
git submodule update --init --recursive 
```

2. To compile the example from the command line, make sure Libero and SmartHLS are
in the PATH in your terminal. On Linux, you can confirm this by typing the following commands:

```
which shls
```

```
which libero 
```

On Windows, you can confirm this by typing the following commands:
```
Get-Command shls
```

```
Get-Command libero
```

3. Now that the tools' paths are set, we can compile!

If you are using Linux:

```
./run_libero.sh
```

Or this if you are using Windows:

```
.\run_libero.ps1
```

The compilation process may take about 40 min. depending on how fast the machine is. 
The `run_libero` script calls Libero and passes the `MPFS_DISCOVERY_KIT_REFERENCE_DESIGN.tcl`
script along with some arguments. It is this TCL file that drives the compilation flow. 
One of the steps in the flow is to call SmartHLS to compile the C++ code into 
Verilog and integrate the HLS cores into the overall Libero design. 

3. When this is done you can find the bitstream in the following location and the 
FPGA can be programmed using Microchip's FPExpress tool:
```
work/discovery-kit-reference-design/soc/Discovery_SoC.job
```

## Running the Example On Board
1. After you've programmed the board with the bitstream generated in the previous step, copy over the binary from the `hls_output` folder to the board by running the following command:

```
scp hls_output/discovery_kit.accel.elf <YOUR BOARD IP HERE>:
```

2. You can run the example by ssh-ing into the board and running:

```
./discovery_kit.accel.elf
```

