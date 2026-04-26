CC = gcc
CFLAGS = -Wall -Ibackend/include
SRC = backend/server.c \
      backend/src/json_loader.c \
      backend/src/math.c \
      backend/src/memory.c \
      backend/src/string.c \
      backend/src/screen.c \
      backend/src/keyboard.c \
      backend/src/input_engine.c

TARGET = typing_server

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
