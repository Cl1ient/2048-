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

$(EXEC_MAIN): main_process.c common.h
	$(CC) $(CFLAGS) -o $(EXEC_MAIN) main_process.c

$(EXEC_ENGINE): game_process.c common.h
	$(CC) $(CFLAGS) -o $(EXEC_ENGINE) game_process.c $(LDFLAGS) 

$(EXEC_DISPLAY): display_process.c common.h
	$(CC) $(CFLAGS) -o $(EXEC_DISPLAY) display_process.c

# Nettoyage des fichiers compilés
clean:
	rm -f $(EXEC_MAIN) $(EXEC_ENGINE) $(EXEC_DISPLAY)
	rm -f pipe_2048_input