
rm output.log, run.log -ErrorAction SilentlyContinue -Force
rm vision_pipeline -ErrorAction SilentlyContinue -Force -Recurse

libero.exe script:libero_flow.tcl logfile:output.log  |  Tee-Object -FilePath run.log

