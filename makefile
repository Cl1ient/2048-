# Paramètres de compilation
CC = gcc
CFLAGS = -Wall -Wextra -g -Isrc/core -Isrc/server
LDFLAGS = -pthread

# Sous-dossiers
CORE = src/core
SERVER = src/server
INPUT = src/input
DISPLAY = src/display

# Noms des exécutables
EXEC_MAIN = main
EXEC_ENGINE = game
EXEC_DISPLAY = display

# Cibles par défaut
all: $(EXEC_MAIN) $(EXEC_ENGINE) $(EXEC_DISPLAY)

$(EXEC_MAIN): $(INPUT)/main_process.c $(CORE)/common.h
	$(CC) $(CFLAGS) -o $(EXEC_MAIN) $(INPUT)/main_process.c

$(EXEC_ENGINE): $(SERVER)/game_process.c $(CORE)/game_logic.c $(SERVER)/client_manager.c \
                $(CORE)/common.h $(CORE)/game_logic.h $(SERVER)/client_manager.h
	$(CC) $(CFLAGS) -o $(EXEC_ENGINE) $(SERVER)/game_process.c $(CORE)/game_logic.c $(SERVER)/client_manager.c $(LDFLAGS)

$(EXEC_DISPLAY): $(DISPLAY)/display_process.c $(CORE)/common.h
	$(CC) $(CFLAGS) -o $(EXEC_DISPLAY) $(DISPLAY)/display_process.c

# Nettoyage des fichiers compilés
clean:
	rm -f $(EXEC_MAIN) $(EXEC_ENGINE) $(EXEC_DISPLAY)
	rm -f pipe_2048_input
