# Copyright (c) 2016-2019, NVIDIA CORPORATION.  All rights reserved.
# 
# Licensed under the Apache License, Version 2.0 (the "License")
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

source ../../nvhls_exec.tcl

proc nvhls::usercmd_post_assembly {} {
    upvar TOP_NAME TOP_NAME
#    directive set /$TOP_NAME/run/while -PIPELINE_INIT_INTERVAL 1
#    directive set /$TOP_NAME/run/while -PIPELINE_STALL_MODE flush

# Refer to memory blocks
# $PROJECT_HOME/../../../cmod/InputBuffer/InputBuffer.h(43): Memory Resource '/Top/InputBuffer/run/mem:rsc' (from var: mem) mapped to 'ram_sample-065nm-singleport_beh_dc.RAM' (size: 8192 x 64). (MEM-4)
# $PROJECT_HOME/../../../cmod/OutputBuffer/OutputBuffer.h(45): Memory Resource '/Top/OutputBuffer/run/mem:rsc' (from var: mem) mapped to 'ram_sample-065nm-singleport_beh_dc.RAM' (size: 16384 x 64). (MEM-4)
    directive set /Top/OutputBuffer/run/mem:rsc -MAP_TO_MODULE {BLOCK_1R1W_RBW}
    directive set /Top/InputBuffer/run/mem:rsc -MAP_TO_MODULE {BLOCK_1R1W_RBW}
 
    #directive set /Top/OutputBuffer/run/mem:rsc -BLOCK_SIZE 8192
    #directive set /Top/InputBuffer/run/mem:rsc/MAP_TO_MODULE ram_sample-065nm-singleport_beh_dc.RAM
    #directive set /Top/OutputBuffer/run/mem:rsc/MAP_TO_MODULE ram_sample-065nm-singleport_beh_dc.RAM
    #/OutputBuffer/run/mem:rsc(0)(0)
    #/OutputBuffer/run/mem:rsc(1)(0)
    
}

nvhls::run
