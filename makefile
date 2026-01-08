# Paramètres de compilation
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -pthread

# Noms des exécutables
EXEC_MAIN = main
EXEC_ENGINE = game
EXEC_DISPLAY = display

# Cibles par défaut
all: $(EXEC_MAIN) $(EXEC_ENGINE) $(EXEC_DISPLAY)

$(EXEC_MAIN): main_process.c
	$(CC) $(CFLAGS) -o $(EXEC_MAIN) main_process.c

$(EXEC_ENGINE): game_process.c
	$(CC) $(CFLAGS) -o $(EXEC_ENGINE) game_process.c $(LDFLAGS) 

$(EXEC_DISPLAY): display_process.c
	$(CC) $(CFLAGS) -o $(EXEC_DISPLAY) display_process.c

# Nettoyage des fichiers compilés
clean:
	rm -f $(EXEC_MAIN) $(EXEC_ENGINE) $(EXEC_DISPLAY)
	rm -f /tmp/2048_pipe  # Supprime aussi le pipe nommé s'il existe

# Aide pour le lancement
help:
	@echo "1. Tapez 'make' pour compiler"
	@echo "2. Ouvrez un terminal et lancez : ./game"
	@echo "3. Ouvrez un autre terminal et lancez : ./main"