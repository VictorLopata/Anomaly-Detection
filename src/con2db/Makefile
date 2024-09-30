# Nome del compilatore
CXX = g++

# Flag per il compilatore (es. standard C++, opzioni di warning)
CXXFLAGS = -std=c++17 -Wall

# Include il percorso della libreria libpq
INCLUDES = -I/usr/include

# Librerie da linkare (libpq per PostgreSQL)
LIBS = -lpq

# Nome dell'eseguibile
TARGET = my_program

# Cartella per i file oggetto
OBJDIR = obj

# File sorgente
SRCS = main.cpp pgsql.cpp

# Crea la lista dei file oggetto, mettendoli nella cartella obj
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

# Regola di default
all: $(TARGET)

# Crea la cartella obj se non esiste e compila l'eseguibile
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Regola per compilare i file sorgenti in file oggetto dentro la cartella obj
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regola per creare la cartella obj se non esiste
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Pulizia dei file compilati
clean:
	rm -f $(OBJDIR)/*.o $(TARGET)
	rmdir $(OBJDIR)

# Installazione delle dipendenze (opzionale, dipende dal sistema)
.PHONY: all clean

