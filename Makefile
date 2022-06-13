# ---------------------------------------------------------------
# https://www.rapidtables.com/code/linux/gcc/gcc-l.html <- how to link libs
# 
# Commands:
# 	build 		: build lib objects and test file for testing 
# 	release 	: build lib objects, archive and organize the lib files for use in the 'dist/' folder
# 	dist 		: dist just organizes the lib files for use in the 'dist/' folder
# 	clear 		: clear compiled executables
# 	clearall 	: clear compiled objects and lib files in 'build/' and 'dist/' folders as well as executables
# 	install  	: installs binaries, includes and libs to the specified "INSTALL_" path variables

CC := gcc

C_FLAGS :=

I_FLAGS :=

L_FLAGS :=

TEST_SOURCE := test.c

SOURCES :=
SOURCES += matrix.c
SOURCES += pnet.c
SOURCES += pnet_error.c

HEADERS :=
HEADERS += matrix.h
HEADERS += pnet_error.h
HEADERS += pnet.h

LIB_NAME := libpnet.a

DIST_DIR := dist/
BUILD_DIR := build/

ARCHIVER := ar -rcs

INSTALL_BIN_DIR := /usr/local/bin
INSTALL_LIB_DIR := /usr/local/lib
INSTALL_INC_DIR := /usr/local/include

# ---------------------------------------------------------------

README = README.md
LABEL_FILES = $(shell ls *.h)
AUTHOR = João Peterson Scheffer
YEAR = 2022
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

build : C_FLAGS += -g
build : $(TEST_EXE)
build : $(HEADERS)
build : $(OBJS_BUILD) 

release : C_FLAGS += -O2
release : $(HEADERS)
release : clear $(OBJS_BUILD) label dist

$(BUILD_DIR)%.o : %.c
	@mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) $(I_FLAGS) -c $< -o $@


dist : $(OBJS_BUILD)
	@mkdir -p $(DIST_DIR)
	$(ARCHIVER) $(DIST_DIR)$(LIB_NAME) $^
	cp $(HEADERS) $(DIST_DIR)


$(TEST_EXE) : $(OBJS_BUILD) $(TEST_OBJ)
	$(CC) $^ -o $@

label : $(README) $(LABEL_FILES)
	sed -i -r 's/(Created by ).+( - )20[0-9]{2}(\. Version )[0-9]\.[0-9]{1,3}-[0-9]{1,3}(\.)/\1$(AUTHOR)\2$(YEAR)\3$(VERSION)\4/g' $^

install :
	cp -r dist/*.h $(INSTALL_INC_DIR)/
	cp -r dist/*.a $(INSTALL_LIB_DIR)/

clear : 
	rm -f -r $(BUILD_DIR)*
	rm -f -r $(DIST_DIR)*
