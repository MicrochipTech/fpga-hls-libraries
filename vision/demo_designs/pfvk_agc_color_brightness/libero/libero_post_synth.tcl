proc update_snvm_to_spi_ram_cfg { ramcfg } {
    set fd [open $ramcfg r]
    set newFilename "[file rootname $ramcfg].new.cfg"
    puts "update_snvm_to_spi_ram_cfg: Creating $newFilename"
    set newfd [open $newFilename w]
    set use_spi 0
    while {[gets $fd line] >= 0} {
        if {[string first "-storage_type" $line] != -1} {
            set prev_line $line
            set prev_line_spi [string map {"SNVM" "SPI"} $line]
        } elseif {[string first "-content_type" $line] != -1} {
            if {[string first "NO_CONTENT" $line] != -1} {
                puts $newfd $prev_line
                puts $newfd $line
            } else {
                puts $newfd $prev_line_spi
                puts $newfd $line
                set use_spi 1
            }
        } else {
            puts $newfd $line
        }
    }
    close $fd
    close $newfd
    configure_ram -cfg_file $newFilename
    if {$use_spi} {
        puts "NOTE: Using SPI instead of SNVM for RAM"
    }
    return $use_spi
}


open_project -file vision_pipeline/vision_pipeline.prjx
run_tool -name {PLACEROUTE}
run_tool -name {VERIFYTIMING}

run_tool -name {GENERATEPROGRAMMINGDATA}
set USE_SPI 1
if {$USE_SPI == 1} {
    puts "Using SPI for memory initialization."
    update_snvm_to_spi_ram_cfg ./vision_pipeline/designer/VIDEO_KIT_TOP/VIDEO_KIT_TOP_RAM.cfg

    generate_design_initialization_data

    configure_ram -cfg_file {./src/cfg_and_mem_files/RAM.spi.cfg}

    configure_design_initialization_data \
        -second_stage_start_address {0x00000000} \
        -third_stage_uprom_start_address {0x00000000} \
        -third_stage_snvm_start_address {0x00000000} \
        -third_stage_spi_start_address {0x00000400} \
        -third_stage_spi_type {SPIFLASH_NO_BINDING_PLAINTEXT} \
        -third_stage_spi_clock_divider {6} \
        -init_timeout 128 \
        -auto_calib_timeout {3000} \
        -broadcast_RAMs {1}
} else {
    configure_ram -cfg_file ./vision_pipeline/designer/VIDEO_KIT_TOP/VIDEO_KIT_TOP_RAM.cfg
}
configure_snvm -cfg_file ./vision_pipeline/designer/VIDEO_KIT_TOP/SNVM.cfg
generate_design_initialization_data

#Generate the Programming job file
export_prog_job \
         -job_file_name {VIDEO_KIT_TOP} \
         -export_dir {.} \
         -bitstream_file_type {TRUSTED_FACILITY} \
         -bitstream_file_components {FABRIC_SNVM} \
         -zeroization_likenew_action 0 \
         -zeroization_unrecoverable_action 0 \
         -program_design 1 \
         -program_spi_flash $USE_SPI \
         -include_plaintext_passkey 0 \
         -design_bitstream_format {PPD} \
         -prog_optional_procedures {} \
         -skip_recommended_procedures {} \
         -sanitize_snvm 0 

save_project
