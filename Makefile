CC=gcc
SOURCES=main.c parser.c executor.c
TARGET=main

all: compile run

compile: $(SOURCES)
	$(CC) -o $(TARGET) $(SOURCES)
	
run: $(TARGET)
	./$(TARGET)

clean: 
	rm ./$(TARGET)