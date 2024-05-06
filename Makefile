# Compilateur
CC = gcc
# Options de compilation
CFLAGS = -Wall -Wno-psabi
IMPORT = -lncurses

# Répertoire des sources
SRCDIR_CLI = src-cli
SRCDIR_SRV = src-srv
LIBDIR = lib

# Répertoires de sortie
BINDIR = bin

# Noms des exécutables
EXEC_CLI = $(BINDIR)/cli
EXEC_SRV = $(BINDIR)/srv

SRC_CLI = $(wildcard $(SRCDIR_CLI)/*.c)
SRC_SRV = $(wildcard $(SRCDIR_SRV)/*.c)
SRC_LIB = $(wildcard $(LIBDIR)/*.c)

OBJ_CLI = $(addprefix $(BINDIR)/, $(SRC_CLI:.c=.o))
OBJ_SRV = $(addprefix $(BINDIR)/, $(SRC_SRV:.c=.o))
OBJ_LIB = $(addprefix $(BINDIR)/, $(SRC_LIB:.c=.o))

all: $(BINDIR) $(BINDIR)/$(SRCDIR_CLI) $(BINDIR)/$(SRCDIR_SRV) $(BINDIR)/$(LIBDIR) $(EXEC_CLI) $(EXEC_SRV)

$(EXEC_CLI): $(OBJ_CLI) $(OBJ_LIB)
	$(CC) $(OBJ_CLI) $(OBJ_LIB) -o $(EXEC_CLI) $(CFLAGS) $(IMPORT)

$(EXEC_SRV): $(OBJ_SRV) $(OBJ_LIB)
	$(CC) $(OBJ_SRV) $(OBJ_LIB) -o $(EXEC_SRV) $(CFLAGS) $(IMPORT)

$(BINDIR)/$(SRCDIR_CLI)/%.o: $(SRCDIR_CLI)/%.c
	$(CC) -c $< -o $(BINDIR)/$(SRCDIR_CLI)/$(notdir $@) $(CFLAGS)

$(BINDIR)/$(SRCDIR_SRV)/%.o: $(SRCDIR_SRV)/%.c
	$(CC) -c $< -o $(BINDIR)/$(SRCDIR_SRV)/$(notdir $@) $(CFLAGS)

$(BINDIR)/$(LIBDIR)/%.o: $(LIBDIR)/%.c
	$(CC) -c $< -o $(BINDIR)/$(LIBDIR)/$(notdir $@) $(CFLAGS)

$(BINDIR):
	@mkdir -p $(BINDIR)

$(BINDIR)/$(SRCDIR_CLI):
	@mkdir -p $(BINDIR)/$(SRCDIR_CLI)

$(BINDIR)/$(SRCDIR_SRV):
	@mkdir -p $(BINDIR)/$(SRCDIR_SRV)

$(BINDIR)/$(LIBDIR):
	@mkdir -p $(BINDIR)/$(LIBDIR)

clean:
	rm -rf $(BINDIR)

# Cibles factices
.PHONY: all clean
