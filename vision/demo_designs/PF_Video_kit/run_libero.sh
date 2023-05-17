cd hls
shls clean
shls hw
cd ../libero
rm -rf vision_pipeline
libero SCRIPT:libero_flow.tcl 2>&1 | tee output.log
