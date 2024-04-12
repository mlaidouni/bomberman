# Compiler
CC = gcc
# Flags de compilation
CFLAGS = -Wall
# Chemin d'inclusion pour les fichiers d'en-tête
INC = -Ilib

# Répertoire des sources
SRCDIR_CLI = src-cli
SRCDIR_SRV = src-srv

# Répertoires de sortie
BINDIR = bin

# Noms des exécutables
EXEC_CLI = $(BINDIR)/cli
EXEC_SRV = $(BINDIR)/srv

# Fichiers sources
SOURCES_CLI = $(SRCDIR_CLI)/client.c
SOURCES_SRV = $(SRCDIR_SRV)/server.c

# Commandes pour la compilation des exécutables
all: $(EXEC_CLI) $(EXEC_SRV)

$(EXEC_CLI): $(SOURCES_CLI)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(INC) $< -o $@

$(EXEC_SRV): $(SOURCES_SRV)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(INC) $< -o $@

# Nettoyer les fichiers générés
clean:
	rm -rf $(BINDIR)

# Cibles factices
.PHONY: all clean

