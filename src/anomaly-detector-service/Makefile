# Variabili
CXX = g++
CXXFLAGS = -std=c++11 -Wall -I/opt/homebrew/include -I/opt/homebrew/opt/postgresql/include
LDFLAGS = -L/opt/homebrew/lib -lhiredis -L/opt/homebrew/opt/postgresql/lib -lpq
TARGET = anomaly-detector-service
SRC = main.cpp ./../con2redis/redisfun.cpp ./../con2db/pgsql.cpp
OBJ_DIR = obj
OBJ = $(OBJ_DIR)/main.o $(OBJ_DIR)/redisfun.o $(OBJ_DIR)/pgsql.o

# Regole
all: $(OBJ_DIR) $(TARGET)

# Crea la directory obj se non esiste
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compila il file oggetto main.cpp
$(OBJ_DIR)/main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o $(OBJ_DIR)/main.o

# Compila il file oggetto redisfun.cpp
$(OBJ_DIR)/redisfun.o: ./../con2redis/redisfun.cpp
	$(CXX) $(CXXFLAGS) -c ./../con2redis/redisfun.cpp -o $(OBJ_DIR)/redisfun.o

# Compila il file oggetto pgsql.cpp
$(OBJ_DIR)/pgsql.o: ./../con2db/pgsql.cpp
	$(CXX) $(CXXFLAGS) -c ./../con2db/pgsql.cpp -o $(OBJ_DIR)/pgsql.o

# Linka i file oggetto per creare l'eseguibile
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $(TARGET)

# Regola per pulire
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(TARGET)

.PHONY: clean
