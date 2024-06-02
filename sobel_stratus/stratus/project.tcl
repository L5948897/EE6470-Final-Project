#*******************************************************************************
# Copyright 2015 Cadence Design Systems, Inc.
# All Rights Reserved.
#
#*******************************************************************************
#
# Stratus Project File
#
############################################################
# Project Parameters
############################################################
#
# Technology Libraries
#
set LIB_PATH "[get_install_path]/share/stratus/techlibs/GPDK045/gsclib045_svt_v4.4/gsclib045/timing"
set LIB_LEAF "slow_vdd1v2_basicCells.lib"
use_tech_lib    "$LIB_PATH/$LIB_LEAF"

#
# Global synthesis attributes.
#
set_attr clock_period           10.0
set_attr message_detail         3 
#set_attr default_input_delay    0.1
#set_attr sched_aggressive_1 on
#set_attr unroll_loops on
#set_attr flatten_arrays all 
#set_attr timing_aggression 0
#set_attr default_protocol true

#
# Simulation Options
#
### 1. You may add your own options for C++ compilation here.
set_attr cc_options             "-DCLOCK_PERIOD=10.0 -g"
#enable_waveform_logging -vcd
set_attr end_of_sim_command "make saySimPassed"

#
# Testbench or System Level Modules
#
### 2. Add your testbench source files here.
define_system_module ../main.cpp
define_system_module ../Testbench.cpp
define_system_module ../System.cpp

#
# SC_MODULEs to be synthesized
#
### 3. Add your design source files here (to be high-level synthesized).
define_hls_module SobelFilter ../SobelFilter.cpp

### 4. Define your HLS configuration (arbitrary names, BASIC and DPA in this example). 
define_hls_config SobelFilter BASIC
define_hls_config SobelFilter DPA           --dpopt_auto=op,expr
define_hls_config SobelFilter FLAT          --flatten_arrays=all
define_hls_config SobelFilter UNROLL        --unroll_loops=on
define_hls_config SobelFilter FLAT_UNROLL   --flatten_arrays=all --unroll_loops=on
define_hls_config SobelFilter UNROLL_PART   -post_elab_tcl {
    unroll_loops [find -loop "*_loop" ]}
define_hls_config SobelFilter FLAT_UNROLL_PART   --flatten_arrays=all -post_elab_tcl {
    unroll_loops [find -loop "*_loop" ]}
define_hls_config SobelFilter CUSTOMIZED    --flatten_arrays=all --unroll_loops=on --dpopt_auto=op,expr
define_hls_config SobelFilter PIPELINE1     -DII=1
define_hls_config SobelFilter PIPELINE2     -DII=2
define_hls_config SobelFilter PIPELINE3     -DII=3
define_hls_config SobelFilter PIPELINE4     -DII=4
define_hls_config SobelFilter PIPELINE5     -DII=5

#set IMAGE_DIR           ".."
#set IN_FILE_NAME        "${IMAGE_DIR}/lena_std_short.bmp"
#set OUT_FILE_NAME				"out.bmp"

### 5. Define simulation configuration for each HLS configuration
### 5.1 The behavioral simulation (C++ only).
define_sim_config B
### 5.2 The Verilog simulation for HLS config "BASIC". 
define_sim_config V_BASIC "SobelFilter RTL_V BASIC"
### 5.3 The Verilog simulation for HLS config "DPA". 
define_sim_config V_DPA "SobelFilter RTL_V DPA"

define_sim_config V_FLAT "SobelFilter RTL_V FLAT"
define_sim_config V_UNROLL "SobelFilter RTL_V UNROLL"
define_sim_config V_FLAT_UNROLL "SobelFilter RTL_V FLAT_UNROLL"
define_sim_config V_CUSTOMIZED "SobelFilter RTL_V CUSTOMIZED"
define_sim_config V_UNROLL_PART "SobelFilter RTL_V UNROLL_PART"
define_sim_config V_FLAT_UNROLL_PART "SobelFilter RTL_V FLAT_UNROLL_PART"
define_sim_config V_PIPELINE1 "SobelFilter RTL_V PIPELINE1"
define_sim_config V_PIPELINE2 "SobelFilter RTL_V PIPELINE2"
define_sim_config V_PIPELINE3 "SobelFilter RTL_V PIPELINE3"
define_sim_config V_PIPELINE4 "SobelFilter RTL_V PIPELINE4"
define_sim_config V_PIPELINE5 "SobelFilter RTL_V PIPELINE5"