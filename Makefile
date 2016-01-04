
LIB_NAME = mlclut

CURR_DIR = .
SRC_DIR = $(CURR_DIR)/src
OBJ_DIR = $(CURR_DIR)/obj
BIN_DIR = $(CURR_DIR)/bin

OBJS = $(OBJ_DIR)/mlclut_descriptions.o \
	   $(OBJ_DIR)/mlclut.o

TEST_SRC_DIR = $(SRC_DIR)/tests
TEST_BIN_DIR = $(BIN_DIR)/tests
TEST_OBJ_DIR = $(OBJ_DIR)/tests

TEST_BINS = $(TEST_BIN_DIR)/main
#TEST_FILES =
TEST_OBJS = $(TEST_OBJ_DIR)/main.o

# headers and libraries

HDR_DIR = $(CURR_DIR)/include
INCLUDES = -I. -I$(HDR_DIR)

LIB_DIR = $(CURR_DIR)/lib
LIBS = $(LIB_DIR)/lib$(LIB_NAME).a
LIBRARIES = -L$(LIB_DIR)

MCLABUTILS = $(shell echo ${MCLABDIR})

MCLAB_INCLUDE = $(MCLABUTILS)/include
MCLAB_LIB = $(MCLABUTILS)/lib

MLUTILS  = $(shell echo ${MLUTILSDIR})

ML_INCLUDE = $(MLUTILS)/include
ML_LIB = $(MLUTILS)/lib

INCLUDES += -I$(MCLAB_INCLUDE) -I$(ML_INCLUDE)
LIBRARIES += -L$(MCLAB_LIB) -L$(ML_LIB)

# compiler and compiler flags

CC = clang
CFLAGS_PRODUCTION = -O2 -DNDEBUG
CFLAGS = -g -fno-builtin --std=c99 --pedantic --pedantic-errors -Wall -Wextra -Wno-unused $(INCLUDES)
BIN_FLAGS = -framework OpenCL

#CLFAGS += $(CFLAGS_PRODUCTION)

all: $(LIBS) $(TEST_BINS)

$(TEST_BINS): $(LIBS) $(TEST_OBJS)
	test -d $(TEST_BIN_DIR) || mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(BIN_FLAGS) $(TEST_OBJ_DIR)/$(@F).o $(LIBRARIES) -l$(LIB_NAME) -lmlutils -lMCLabUtils -o $@

$(TEST_OBJS):
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


