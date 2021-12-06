SRC_DIR = src

all: compiler
	 make ll progDecVars.ras

# Build the pascal compiler using the subdir makefile
compiler:
	$(MAKE) -C $(SRC_DIR) -j
	mv $(SRC_DIR)/rascal .

# Use our pascal compiler to compile a test file in `test` directory
# This will generate an IR file in the root directory
ll:
	./rascal < test/$(filter-out ll asm,$(MAKECMDGOALS))

%:
	@:

clean:
	$(MAKE) -C $(SRC_DIR) clean
	$(RM) *.ll *.s

clean-all: clean
	$(RM) rascal
	
