CC = clang
RM = rm

LIB_MK_PATH = ../libraries
include $(LIB_MK_PATH)/mclabutils.mk $(LIB_MK_PATH)/mlutils.mk $(LIB_MK_PATH)/stb_lib.mk

LIB_NAME = mlclut

# directories

CURR_DIR = .
SRC_DIR = $(CURR_DIR)/src
OBJ_DIR = $(CURR_DIR)/obj
BIN_DIR = $(CURR_DIR)/bin

OBJS = $(OBJ_DIR)/mlclut_descriptions.o \
	   $(OBJ_DIR)/mlclut_images.o \
	   $(OBJ_DIR)/mlclut.o

TEST_SRC_DIR = $(SRC_DIR)/tests
TEST_BIN_DIR = $(BIN_DIR)/tests
TEST_OBJ_DIR = $(OBJ_DIR)/tests

TEST_BINS = $(TEST_BIN_DIR)/device_infos \
			$(TEST_BIN_DIR)/image_formats
#TEST_FILES =
TEST_OBJS = $(TEST_OBJ_DIR)/device_infos.o \
			$(TEST_OBJ_DIR)/image_formats.o

# headers and libraries

HDR_DIR = $(CURR_DIR)/include
INCLUDES = -I. -I$(HDR_DIR)

LIB_DIR = $(CURR_DIR)/lib
LIBS = $(LIB_DIR)/lib$(LIB_NAME).a
LIBRARIES = -L$(LIB_DIR)

INCLUDES += -I$(MCLAB_INCLUDE) -I$(ML_INCLUDE) -I$(STB_DIR)
LIBRARIES += -L$(MCLAB_LIB) -L$(ML_LIB)

# compiler and compiler flags

CFLAGS_PRODUCTION = -O2 -DNDEBUG
CFLAGS = -g -fno-builtin --std=c99 --pedantic --pedantic-errors -Wall -Wextra -Wno-unused $(INCLUDES)

UNAME = $(shell uname)

ifeq ($(UNAME), Darwin)
BIN_FLAGS = -framework OpenCL
else
BIN_FLAGS = -lOpenCL
endif

#CLFAGS += $(CFLAGS_PRODUCTION)

all: $(LIBS) $(TEST_BINS)

$(TEST_BINS): $(LIBS) $(TEST_OBJS)
	test -d $(TEST_BIN_DIR) || mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(BIN_FLAGS) $(TEST_OBJ_DIR)/$(@F).o $(LIBRARIES) -l$(LIB_NAME) -lmlutils -lMCLabUtils -o $@

$(TEST_OBJS): $(TEST_SRC_DIR)/$(@F:.o=.c)
	test -d $(TEST_OBJ_DIR) || mkdir -p $(TEST_OBJ_DIR)
	$(CC) $(CFLAGS) $(TEST_SRC_DIR)/$(@F:.o=.c) -c -o $@

$(OBJS): $(SRC_DIR)/$(@F:.o=.c)
	test -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/$(@F:.o=.c) -c -o $@

$(LIBS): $(OBJS)
	test -d $(LIB_DIR) || mkdir -p $(LIB_DIR)
	ar r $@ $(OBJS)
	ranlib $@

clean:
	$(RM) -rf $(OBJ_DIR)
	$(RM) -rf $(BIN_DIR)
	$(RM) -rf $(LIB_DIR)


