# Paths
OBJ_DIR = ./src
INC_DIR = ./include

# Flags and compiler
CPPFLAGS = -Wall  -I $(INC_DIR)
CC = g++

# Objects
OBJ_COMMAND = $(OBJ_DIR)/command.o
OBJ_PARSER = $(OBJ_DIR)/parser.o
OBJ_MAIN = $(OBJ_DIR)/main.o
OBJS = $(OBJ_COMMAND) $(OBJ_PARSER) $(OBJ_MAIN)

# Executables
EXEC_MAIN = main

# Define all targets
all: target_main

# And each one individually
target_main: $(EXEC_MAIN)

$(EXEC_MAIN): $(OBJS)
	@$(CC) $(CPPFLAGS) $(OBJS) -o $(EXEC_MAIN)

.SILENT: $(OBJS) # Silence implicit rule output
.PHONY: clean

clean:
	@echo "Cleaning up ..."
	@rm -f $(OBJS) $(EXEC_MAIN)

run: $(EXEC_MAIN)
	@./$(EXEC_MAIN)
