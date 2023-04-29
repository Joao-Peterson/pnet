# ---------------------------------------------------------------
# Commands:
# 	build 		: build lib objects and test file for testing 
# 	release 	: build lib objects, archive and organize the lib files for use in the 'dist/' folder
# 	dist 		: dist just organizes the lib files for use in the 'dist/' folder
# 	clear 		: clear compiled executables
# 	clearall 	: clear compiled objects and lib files in 'build/' and 'dist/' folders as well as executables
# 	install  	: installs binaries, includes and libs to the specified "INSTALL_" path variables
# 	label  		: update the author, year and version using sed in the specified files 
# 	test  		: runs the test binary 
# 	mem  		: runs valgrind on the test binary 
# 	doc  		: runs doxygen and generate output on /docs folder 

CC := gcc

C_FLAGS :=

BUILD_OPTIMIZATION := -g
RELEASE_OPTIMIZATION := -O2

I_FLAGS :=

L_FLAGS := -lpthread

TEST_SOURCE := test.c

SOURCES :=
SOURCES += pnet_matrix.c
SOURCES += pnet.c
SOURCES += pnet_error.c

HEADERS :=
HEADERS += pnet_matrix.h
HEADERS += pnet_error.h
HEADERS += pnet.h

LIB_STATIC := libpnet.a
LIB_DYN := libpnet.so

DIST_DIR := dist/
BUILD_DIR := build/
DOC_DIR := docs/

ARCHIVER := ar -rcs

INSTALL_BIN_DIR := /usr/local/bin
INSTALL_LIB_DIR := /usr/local/lib
INSTALL_INC_DIR := /usr/local/include

# ---------------------------------------------------------------

README = README.md
LABEL_FILES = $(shell ls *.h)
LABEL_FILES += $(DOC_DIR)Doxyfile
AUTHOR = João Peterson Scheffer
YEAR = 2022
#VERSION = 0.000-000
VERSION = 1.0-0

# ---------------------------------------------------------------

# rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) \
#   $(filter $(subst *,%,$2),$d))

OBJS := $(SOURCES:.c=.o)
OBJS_BUILD := $(addprefix $(BUILD_DIR), $(OBJS))
TEST_OBJ := $(BUILD_DIR)$(TEST_SOURCE:.c=.o)
TEST_EXE := $(BUILD_DIR)$(TEST_SOURCE:.c=.exe)

# ---------------------------------------------------------------

.PHONY : build label clean install

build : C_FLAGS += $(BUILD_OPTIMIZATION)
build : $(BUILD_DIR)$(LIB_DYN) 
build : $(BUILD_DIR)$(LIB_STATIC) 
build : $(TEST_EXE)

release : C_FLAGS += $(RELEASE_OPTIMIZATION)
release : clear
release : $(BUILD_DIR)$(LIB_DYN) 
release : $(BUILD_DIR)$(LIB_STATIC) 
release : label doc dist


dist : $(BUILD_DIR)$(LIB_STATIC) $(BUILD_DIR)$(LIB_DYN)
	@mkdir -p $(DIST_DIR)
	cp $(HEADERS) $(DIST_DIR)
	cp $(BUILD_DIR)$(LIB_STATIC) $(DIST_DIR)$(LIB_STATIC)
	cp $(BUILD_DIR)$(LIB_DYN) $(DIST_DIR)$(LIB_DYN)


$(BUILD_DIR)$(LIB_STATIC) : C_FLAGS +=
$(BUILD_DIR)$(LIB_STATIC) : $(OBJS_BUILD)
	$(ARCHIVER) $(BUILD_DIR)$(LIB_STATIC) $(OBJS_BUILD)
	

$(BUILD_DIR)$(LIB_DYN) : C_FLAGS += -fPIC
$(BUILD_DIR)$(LIB_DYN) : $(OBJS_BUILD)
	$(CC) -shared -fPIC -o $(BUILD_DIR)$(LIB_DYN) $(OBJS_BUILD)


$(BUILD_DIR)%.o : %.c
	@mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) $(I_FLAGS) -c $< -o $@


$(BUILD_DIR)%.exe : %.c
	$(CC) $(C_FLAGS) $^ $(BUILD_DIR)$(LIB_STATIC) -o $@


label : $(README) $(LABEL_FILES)
# match and print. For testing. 
# sed -r -n '/(Created by ).+( - )20[0-9]{2}(\. Version )[0-9]\.[0-9]{1,3}-[0-9]{1,3}(\.)/p' $^
	sed -i -r 's/(Created by ).+( - )20[0-9]{2}(\. Version )[0-9]\.[0-9]{1,3}-[0-9]{1,3}(\.)/\1$(AUTHOR)\2$(YEAR)\3$(VERSION)\4/g' $^
	sed -i -r 's/(PROJECT_NUMBER\s+= )[0-9]\.[0-9]{1,3}-[0-9]{1,3}/\1$(VERSION)/g' $^
	sed -i -r 's/(badge\/Version-)[0-9]\.[0-9]{1,3}--[0-9]{1,3}/\1$(subst -,--,$(VERSION))/g' $^

install :
	cp -r dist/*.h $(INSTALL_INC_DIR)/
	cp $(DIST_DIR)$(LIB_STATIC) $(INSTALL_LIB_DIR)/
	cp $(DIST_DIR)$(LIB_DYN) $(INSTALL_LIB_DIR)/

test : build
	$(TEST_EXE) 

mem : 
	valgrind -s --leak-check=full $(TEST_EXE)
# valgrind --tool=callgrind $(TEST_EXE)

doc : label
	doxygen -q $(DOC_DIR)Doxyfile

app : 
	$(TEST_APP_EXE)

clear : 
	rm -f -r $(BUILD_DIR)*
	rm -f -r $(DIST_DIR)*
