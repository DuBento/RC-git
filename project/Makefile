# ==============================================================================
# Makefile:				Generic C Makefile
# Originaly built: 		10/09/2019
# Author:				André Nascimento	andreffnascimento@tecnico.ulisboa.pt
# ==============================================================================

# ==============================================================================
# RC - Project_1:

# Authors:				
#	André Nascimento		92419		andreffnascimento@tecnico.ulisboa.pt
#	Duarte Bento			92456		duarte.bento@tecnico.ulisboa.pt
# 	Francisco Malveiro		92465		francisco.malveiro@tecnico.ulisboa.pt

# Makefile's last Modification: 	01/10/2020
# ==============================================================================





# Location properties
# ==============================================================================
# (relative to the makefile location)
DIR_SRC	:= src
DIR_EXE	:= bin
DIR_OBJ	:= obj
# ==============================================================================


# Executable files
# ==============================================================================
# (names of the executables that will be generated)
EXE_NAMES := pd #user as fd
# ==============================================================================


# Executable dependencies 
# ==============================================================================
# (paths relative to the [DIR_SRC] folder to the executable's required .c files)
$(DIR_EXE)/pd.deps = pd.c common.c udp.c 
# ==============================================================================



# Source code dependencies
# ==============================================================================
# (paths relative to the [DIR_SRC] folder of the files #included in the code)
$(DIR_OBJ)/pd.deps =
# ==============================================================================


# Compilation properties
# ==============================================================================
C_VERSION	= C99
INCLUDES	= -I$(ROOT_DIR) -I$(ROOT_DIR)/src
MACROS 		= -DDEBUG
# ==============================================================================





# =============================== Makefile code ================================
# ============================== [DO NOT TOUCH] ================================

# Default rule (make command)
.DEFAULT_GOAL := compile

# GCC properties
CC 			:= gcc
LD   		:= gcc
CC_FLAGS 	:= -Wall -Wextra -Werror -std=gnu99 -pedantic -I../
LD_FLAGS 	:= -lm
ROOT_DIR 	:= $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# C-language compiler version
ifeq ($(C_VERSION), C98)
CC_FLAGS += -ansi
endif

# Debug compilation flags
ifeq ($(DEBUG), true)
CC_FLAGS += -g -fsanitize=address
endif

# Final executable paths
EXE_FILES := $(foreach exe, $(EXE_NAMES), $(DIR_EXE)/$(exe))
SRC_FILES := $(if $(wildcard src), $(shell find src -name "*.c"))
SRC_FILES := $(patsubst $(DIR_SRC)/%.c, %.c, $(SRC_FILES))



# Compilation of all the src files (into .o files)
.SECONDEXPANSION:
.SECONDARY:
$(DIR_OBJ)/%.o: $(DIR_SRC)/%.c $(DIR_SRC)/$$($(DIR_OBJ)/$$*.deps)
	@ echo "\033[97;2m========================================================================================================================\033[0m\n"
	@ mkdir -p $(dir $@)
	@ $(CC) $(CFLAGS) -o $@ -c $< $(MACROS) $(INCLUDES)
	@ echo "\033[92;4m[ COMPILED ]\033[0;92m :\033[91m" $(CC) "\033[93m"$(CC_FLAGS) "\033[96m-o \033[96;7m"$@"\033[0;94m -c \033[4;94m"$<"\033[0m"
	@ echo "\033[92;2m - [Macros]\t:\033[97m" $(MACROS) 
	@ echo "\033[92;2m - [Includes]\t:\033[97m" $(INCLUDES) 
	@ echo "\033[92;2m - [Dependecies]:\033[97m" $^"\033[0m\n" 


# Links the multiple object files into an executable file
.SECONDEXPANSION:
$(EXE_FILES): $$(patsubst %.c, $(DIR_OBJ)/%.o, $$($$@.deps))
	@ echo "\033[97;2m========================================================================================================================\033[0m\n"
	@ mkdir -p $(dir $@)
	@ $(LD) $(CC_FLAGS) $(LD_FLAGS) -o $@ $^
	@ echo "\033[92;7m[ FINISHED ]\033[0;92m :\033[91m" $(LD) "\033[93m"$(CC_FLAGS) $(LD_FLAGS) "\033[96m-o \033[96;7m"$@"\033[0;94m" $^"\033[0m\n"


# ============================== Makefile rules ================================

# Compiles all the executable files
.PHONY:
compile: $(EXE_FILES)
	@ echo "\033[97;2m========================================================================================================================\033[0m\n"
	@ echo "\t\t\033[92;4mAll executables were sucssesfuly compiled!\033[92;0m\n"


# Compiles the specified executable file
.PHONY:
$(EXE_NAMES): 
	@ make --no-print-directory $(DIR_EXE)/$@


# Deletes the bin and obj directories
.PHONY:
clean:
	@ rm -rf $(DIR_OBJ)
	@ rm -rf $(DIR_EXE)
