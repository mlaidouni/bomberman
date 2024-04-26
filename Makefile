# Compilateur
CC = gcc
# Options de compilation
CFLAGS = -Wall -lncurses

# Répertoire des sources
SRCDIR_CLI = src-cli
SRCDIR_SRV = src-srv
LIBDIR = lib

# Répertoires de sortie
BINDIR = bin

# Noms des exécutables
EXEC_CLI = $(BINDIR)/cli
EXEC_SRV = $(BINDIR)/srv

# Fichiers sources
# On exclut le fichier ncurses.c pour éviter les erreurs de compilation
SOURCES_CLI = $(filter-out $(SRCDIR_CLI)/ncurses.c, $(wildcard $(SRCDIR_CLI)/*.c))
SOURCES_SRV = $(wildcard $(SRCDIR_SRV)/*.c)
SOURCES_LIB = $(wildcard $(LIBDIR)/*.c)

# Commandes pour la compilation des exécutables
all: $(EXEC_CLI) $(EXEC_SRV)

$(EXEC_CLI): $(SOURCES_CLI) $(SOURCES_LIB)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(SOURCES_CLI) $(SOURCES_LIB) -o $@

$(EXEC_SRV): $(SOURCES_SRV)
	$(CC) $(CFLAGS) $(SOURCES_SRV) -o $@

# Nettoyer les fichiers générés
clean:
	rm -rf $(BINDIR)

# Cibles factices
.PHONY: all clean
