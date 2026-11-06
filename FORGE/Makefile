CC = clang
CFLAGS = -Wall -Wextra -O2
TARGET = forge
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
    $(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
    rm -f $(TARGET)

install: $(TARGET)
    cp $(TARGET) $(PREFIX)/bin/$(TARGET)
    chmod +x $(PREFIX)/bin/$(TARGET)
    @echo "¡FORGE instalado globalmente!"
