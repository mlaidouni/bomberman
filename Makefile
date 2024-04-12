# Compilateur
CC = gcc
# Options de compilation
CFLAGS = -Wall

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
SOURCES_CLI = $(SRCDIR_CLI)/client.c
SOURCES_SRV = $(SRCDIR_SRV)/server.c
SOURCES_MSG = $(LIBDIR)/message.c

# Commandes pour la compilation des exécutables
all: $(EXEC_CLI) $(EXEC_SRV)

$(EXEC_CLI): $(SOURCES_CLI) $(SOURCES_MSG)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(SOURCES_CLI) $(SOURCES_MSG) -o $@

$(EXEC_SRV): $(SOURCES_SRV)
	$(CC) $(CFLAGS) $(SOURCES_SRV) -o $@

# Nettoyer les fichiers générés
clean:
	rm -rf $(BINDIR)

# Cibles factices
.PHONY: all clean

