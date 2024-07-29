COMPILER = gcc
FILE_EXTENSION = .c
OBJS = main.o	\
	roblox-gen/gen.o	\
	getopt.o					\
	roblox-gen/util.o	\
	roblox-gen/parse/lexer.o\
	roblox-gen/err.o	\
	roblox-gen/parse/parse.o

EXEC_NAME = rbxg
CFLAGS =

.PHONY:
all: $(EXEC_NAME)

# clean
.PHONY:
clean:
	@echo CLEANING *.o
	@del *.o $(EXEC_NAME)
	@del roblox-gen\*.o
	@del roblox-gen\parse\*.o

# executable
$(EXEC_NAME): $(OBJS)
	@echo TARGET '$(EXEC_NAME)' CREATING BINARIES FROM
	@echo '$(OBJS)'...
	@echo FLAGS [$(CFLAGS)]
	@$(COMPILER) $(CFLAGS) $(OBJS) -o $(EXEC_NAME)

# build em' all
%.o: %$(FILE_EXTENSION)
	@$(COMPILER) $(CFLAGS) -c $< -o $@

