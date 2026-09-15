# Exporting Component Description of mipicsi2rxdecoderPF_C1 to TCL
# Family: PolarFire
# Part Number: MPF300TS-1FCG1152I
# Create and Configure the core component mipicsi2rxdecoderPF_C0
create_and_configure_core -core_vlnv "Microchip:SolutionCore:mipicsi2rxdecoderPF:${mipicsi2rxdecoderPF_version}" -component_name {mipicsi2rxdecoderPF_C0} -params {\
"g_DATAWIDTH:10"  \
"g_FIFO_SIZE:12"  \
"g_FORMAT:0"  \
"g_INPUT_DATA_INVERT:0"  \
"g_LANE_WIDTH:4"  \
"g_NO_OF_VC:1"  \
"g_NUM_OF_PIXELS:4"   }
# Exporting Component Description of mipicsi2rxdecoderPF_C1 to TCL done
