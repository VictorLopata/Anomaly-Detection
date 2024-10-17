# Variabili
CXX = g++
CXXFLAGS = -std=c++11 -Wall -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lhiredis
TARGET = covariance-service
SRC = main.cpp ./../con2redis/redisfun.cpp
OBJ_DIR = obj
OBJ = $(OBJ_DIR)/main.o $(OBJ_DIR)/redisfun.o

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



# Linka i file oggetto per creare l'eseguibile
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $(TARGET)

# Regola per pulire
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(TARGET)

.PHONY: clean