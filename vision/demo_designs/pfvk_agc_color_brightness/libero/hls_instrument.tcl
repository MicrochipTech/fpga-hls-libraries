# HLS project info
set hlsPrjDir ../hls
set hlsPrjName pf_demo

# Other signals
iice new {IICE} -type regular -mode {none}
iice sampler -iice {IICE} -depth 1024 -qualified_sampling 1 -always_armed 1 -compression 0 {behavioral}
iice clock -iice {IICE}  -edge positive {/Video_Pipeline_0/Video_AXIS_Converter_0/i_axis_clk}
iice controller -iice {IICE}  none
# iice -iice {IICE} controller -counterwidth 2 -triggerstates 4 -triggerconditions 2 -importtrigger 0 -exporttrigger 0 -crosstrigger 0 statemachine

### Instrumenting custom signals here:
signals add -iice {IICE} -sample -trigger {/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/o_pushback_detected}
signals add -iice {IICE} -sample -trigger {/IMX334_IF_TOP_0/Camera_To_AXIS_Converter_0/o_frames_skipped}

# Instrumentation of HLS modules
source $hlsPrjDir/hls_output/scripts/instrument/prj_${hlsPrjName}_hls_identify.tcl

