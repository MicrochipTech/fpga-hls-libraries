CXXFLAGS=-std=c++11 -I$(SHLS_ROOT_DIR)/smarthls-library -I../../include

.PHONY: gen_log_table
.PHONY: gen_cordic_table 
.PHONY: gen_cordic_hyp_table	
.PHONY: gen_sin_lut
.PHONY: gen_all_tables

gen_all_tables: gen_log_table gen_cordic_table gen_cordic_hyp_table gen_sin_lut
gen_log_table:
	LD_LIBRARY_PATH=$(SHLS_ROOT_DIR)/dependencies/lib g++ $(CXXFLAGS) log_gentable.cpp -o log_gentable
	LD_LIBRARY_PATH=$(SHLS_ROOT_DIR)/dependencies/gcc/lib64 ./log_gentable
	rm log_gentable
gen_cordic_table: 
	LD_LIBRARY_PATH=$(SHLS_ROOT_DIR)/dependencies/lib g++ $(CXXFLAGS) cordic_angle_gentable.cpp -o cordic_angle_gentable
	LD_LIBRARY_PATH=$(SHLS_ROOT_DIR)/dependencies/gcc/lib64 ./cordic_angle_gentable
	rm cordic_angle_gentable
gen_cordic_hyp_table:
	LD_LIBRARY_PATH=$(SHLS_ROOT_DIR)/dependencies/lib g++ $(CXXFLAGS) cordic_hyperbolic_gentable.cpp -o cordic_hyperbolic_gentable
	LD_LIBRARY_PATH=$(SHLS_ROOT_DIR)/dependencies/gcc/lib64 ./cordic_hyperbolic_gentable
	rm cordic_hyperbolic_gentable	
gen_sin_lut:
	LD_LIBRARY_PATH=$(SHLS_ROOT_DIR)/dependencies/lib g++ $(CXXFLAGS) sin_lut_gentable.cpp -o sin_lut_gentable
	LD_LIBRARY_PATH=$(SHLS_ROOT_DIR)/dependencies/gcc/lib64 ./sin_lut_gentable
	rm sin_lut_gentable
