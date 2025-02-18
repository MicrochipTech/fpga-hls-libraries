

set synplify_exe synplify_pro
set identify_exe identify_instrumentor_shell

#-----------------------------------------------
# Generate instrumentation file for HLS modules
#-----------------------------------------------

# From SmartHLS installation dir
set instrumentationScript [string cat $::env(SHLS_ROOT_DIR) "/examples/scripts/utils/instrument/generate_identify_instrumentation_files.tcl"]

# HLS project info
set hlsPrjDir ../hls
set hlsPrjName pf_demo

# Libero project info 
set LiberoSynthDir vision_pipeline/synthesis

# Generate a new instrumentation TCL file
exec $synplify_exe -licensetype synplifypro_actel -batch $instrumentationScript $hlsPrjDir $LiberoSynthDir


#------------------------
# Actual instrumentation
#------------------------
exec $identify_exe -licensetype identinstrumentor_actel hls_instrument.tcl

