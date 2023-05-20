# Commands:
# ---------------------------------------------------------------
# 	build 		: build lib objects and test file for testing 
# 	release 	: build lib objects, archive and organize the lib files for use in the 'dist/' folder
# 	dist 		: dist just organizes the lib files for use in the 'dist/' folder
# 	clear 		: clear compiled executables
# 	install  	: installs binaries, includes and libs to the specified "INSTALL_" path variables
# 	label  		: update the author, year and version using sed in the specified files 
# 	mem  		: runs valgrind on the test binary 
# 	doc  		: runs doxygen and generate output on /docs folder 

# Options -------------------------------------------------------

CC=gcc
C_FLAGS=-fpic -Wall
C_FLAGS_RELEASE=-O2
C_FLAGS_DEBUG=-g
I_FLAGS=-Isrc
L_FLAGS=-lpthread

AR=ar
AR_FLAGS=-rcs
SRC_DIR=src
BUILD_DIR=build
DIST_DIR=dist
DOC_DIR=docs
INSTALL_BIN_DIR := /usr/local/bin
INSTALL_LIB_DIR := /usr/lib
INSTALL_INC_DIR := /usr/local/include

AUTHOR = João Peterson Scheffer
YEAR = 2023
VERSION = 1.1.0

# File recipes --------------------------------------------------

.PHONY : build clear build_dir dist_dir

build : C_FLAGS += $(C_FLAGS_DEBUG)
build : build_dir tests

release : C_FLAGS += $(C_FLAGS_RELEASE)
release : dist

tests : test.o libpnet.a
	$(CC) $(L_FLAGS) $(addprefix $(BUILD_DIR)/, $(notdir $^)) -o $@

dist : libpnet.a libpnet.so 
	@mkdir -p $(DIST_DIR)
	@cp -vr $(BUILD_DIR)/*.so $(DIST_DIR)/ 
	@cp -vr $(BUILD_DIR)/*.a $(DIST_DIR)/ 
	@cp -vr $(SRC_DIR)/*.h $(DIST_DIR)/ 
	@cp -v  README.md $(DIST_DIR)/ 
	sed -r -i 's/(\{AUTHOR\})/$(AUTHOR)/g' $(DIST_DIR)/*.h 
	sed -r -i 's/(\{YEAR\})/$(YEAR)/g' $(DIST_DIR)/*.h 
	sed -r -i 's/(\{VERSION\})/$(VERSION)/g' $(DIST_DIR)/*.h 
	sed -i -r 's/(badge\/Version-)[0-9]\.[0-9]{1,3}--[0-9]{1,3}/\1$(subst -,--,$(VERSION))/g' README.md $(DIST_DIR)/README.md
	sed -i -r 's/(PROJECT_NUMBER\s+= )[0-9]\.[0-9]{1,3}-[0-9]{1,3}/\1$(VERSION)/g' $(DOC_DIR)/Doxyfile

libpnet.a : src/pnet.o src/queue.o src/pnet_matrix.o src/pnet_error.o
	$(AR) $(AR_FLAGS) $(addprefix $(BUILD_DIR)/, $@) $(addprefix $(BUILD_DIR)/, $(notdir $^))

libpnet.so : src/pnet.o src/queue.o src/pnet_matrix.o src/pnet_error.o
	$(CC) -shared $(addprefix $(BUILD_DIR)/, $(notdir $^)) -o $(addprefix $(BUILD_DIR)/, $@)

# Other recipes (Dont edit) ----------------------------------------

install : dist
	@cp -vr $(DIST_DIR)/*.h $(INSTALL_INC_DIR)/
	@cp -vr $(DIST_DIR)/*.a $(INSTALL_LIB_DIR)/
	@cp -vr $(DIST_DIR)/*.so $(INSTALL_LIB_DIR)/

%.o : %.c
	$(CC) $(C_FLAGS) $(I_FLAGS) -c $^ -o $(addprefix $(BUILD_DIR)/, $(notdir $@))

build_dir : 
	@mkdir -p $(BUILD_DIR)

clear : 
	@rm -vrdf $(BUILD_DIR)
	@rm -vrdf $(DIST_DIR)
	@rm -vf *.exe
	@rm -vf *.o
	@rm -vf tests

mem : 
	valgrind -s --leak-check=full $(TEST_EXE)
# valgrind --tool=callgrind $(TEST_EXE)

doc : dist
	doxygen -q $(DOC_DIR)/Doxyfile
 