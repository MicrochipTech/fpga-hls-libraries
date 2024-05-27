CXXFLAGS=-std=c++11 -I$(SHLS_ROOT_DIR)/smarthls-library -I../../include

TABLES += gen_gamma_correction_table 

.PHONY: gen_all_tables

gen_all_tables: $(TABLES)

gen_gamma_correction_table:
	LD_LIBRARY_PATH=$(SHLS_ROOT_DIR)/dependencies/lib g++ $(CXXFLAGS) gen_gamma_correction_table.cpp -o gen_gamma_correction_table
	LD_LIBRARY_PATH=$(SHLS_ROOT_DIR)/dependencies/gcc/lib64 ./gen_gamma_correction_table 
	rm gen_gamma_correction_table
