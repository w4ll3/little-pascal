SRC_DIR = src
CXXFLAGS = -std=c++11

all: compiler
	make ll test.ras
	./rascal < test/test.ras

compiler:
	$(MAKE) -C $(SRC_DIR) -j
	mv $(SRC_DIR)/rascal .

%:
	@:

clean:
	$(MAKE) -C $(SRC_DIR) clean
	$(RM) *.ll *.s

clean-all: clean
	$(RM) rascal
	
