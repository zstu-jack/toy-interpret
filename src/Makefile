TARGET_DIR = $(CURDIR)/
TARGET = $(TARGET_DIR)/a.out

###############################################################################
# variables
###############################################################################
CC  =   gcc
CXX =   g++
CPP =   g++

###############################################################################
# flags
###############################################################################
CPPFLAGS= -g3 -O0 -Wall -std=c++11
LDFLAGS = -g3

###############################################################################
# paths
###############################################################################
OBJ_DIR =   ./.objs
LIB_DIR = ./

PUBLIC_INC  = -I./
PUBLIC_LIB  = -L$(LIB_DIR)

SRC = $(wildcard *.cpp) $(wildcard *.c) $(wildcard *.cxx) $(wildcard */*.cpp) $(wildcard */*/*.cpp) $(wildcard */*/*/*.cpp) $(wildcard */*/*/*/*.cpp) $(wildcard */*/*/*/*/*.cpp)
OBJS = $(addprefix $(OBJ_DIR)/,$(subst .cpp,.o,$(SRC)))

###############################################################################
# args
###############################################################################
all : $(TARGET)

$(TARGET) : $(OBJS)
	@if [ ! -e $(TARGET_DIR) ]; then \
		mkdir -p $(TARGET_DIR); \
	fi

	$(CXX) -o $@ $(OBJS) $(PUBLIC_LIB) $(LDFLAGS)

$(OBJ_DIR)/%.o:%.cpp
	@if [ ! -e $(dir $@) ]; then \
	    mkdir -p $(dir $@); \
	fi
	$(CXX) $(CPPFLAGS) $(PUBLIC_INC) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

