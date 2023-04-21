# Paths
OBJ_DIR = ./src
INC_DIR = ./include

# Flags and compiler
CPPFLAGS = -Wall  -I $(INC_DIR)
CC = g++

# Objects
OBJ_COMMAND = $(OBJ_DIR)/command.o
OBJ_PARSER = $(OBJ_DIR)/parser.o
OBJ_HISTORY = $(OBJ_DIR)/history.o
OBJ_MYSH = $(OBJ_DIR)/mysh.o
OBJS = $(OBJ_COMMAND) $(OBJ_PARSER) $(OBJ_HISTORY) $(OBJ_MYSH)

# Executables
EXEC_MYSH = mysh

# Define all targets
all: target_mysh

# And each one individually
target_mysh: $(EXEC_MYSH)

$(EXEC_MYSH): $(OBJS)
	@$(CC) $(CPPFLAGS) $(OBJS) -o $(EXEC_MYSH)

.SILENT: $(OBJS) # Silence implicit rule output
.PHONY: clean

clean:
	@echo "Cleaning up ..."
	@rm -f $(OBJS) $(EXEC_MYSH)

run: $(EXEC_MYSH)
	./$(EXEC_MYSH)
