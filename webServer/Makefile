CC = gcc
TARGET = myhttpd
CFLAGS = -O2 -Wall
LFLAGS = -pthread

all: queue.o worker.o communication.o PollargsList.o stats.o WordQList.o webServer.o inet_str_server.o helpers.o workersInfo.o
	$(CC) -o $(TARGET) $^ $(LFLAGS)

clean:
	rm -f *.o
	rm -f $(TARGET)

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c -o $@

worker.o: worker.c worker.h workersInfo.h queue.h stats.h helpers.h WordQList.h constants.h communication.h
	$(CC) $(CFLAGS) -c worker.c -o worker.o

communication.o: communication.c communication.h WordQList.h constants.h
	$(CC) $(CFLAGS) -c communication.c -o communication.o

PollargsList.o: PollargsList.c PollArgsList.h constants.h
	$(CC) $(CFLAGS) -c PollargsList.c -o PollargsList.o

stats.o: stats.c stats.h
	$(CC) $(CFLAGS) -c stats.c -o stats.o

WordQList.o: WordQList.c WordQList.h constants.h
	$(CC) $(CFLAGS) -c WordQList.c -o WordQList.o

webServer.o: webServer.c communication.h WordQList.h constants.h PollArgsList.h inet_str_server.h stats.h worker.h workersInfo.h queue.h helpers.h
	$(CC) $(CFLAGS) -c webServer.c -o webServer.o

inet_str_server.o: inet_str_server.c inet_str_server.h constants.h
	$(CC) $(CFLAGS) -c inet_str_server.c -o inet_str_server.o

helpers.o: helpers.c helpers.h WordQList.h constants.h
	$(CC) $(CFLAGS) -c helpers.c -o helpers.o

workersInfo.o: workersInfo.c workersInfo.h queue.h stats.h
	$(CC) $(CFLAGS) -c workersInfo.c -o workersInfo.o
