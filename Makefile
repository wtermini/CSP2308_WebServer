CC = gcc
CFLAGS = -Wall
OBJECTS = mywebserver.o myfiles.o mylog.o myparser.o myconfload.o myresponse.o
TARGET = mywebserver
mywebserver : $(OBJECTS)
	mkdir -p bin
	$(CC) $(CFLAGS) $(OBJECTS) -o bin/$(TARGET)
	cp defaults/default_mywebserver.conf bin/mywebserver.conf
	cp -r defaults/samplepage bin/samplepage
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
	
.PHONY: clean

clean:
	rm -r -f $(TARGET) *.o bin buildtmp
	
run: mywebserver
	cd bin && ./$(TARGET)