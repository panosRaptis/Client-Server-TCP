CC = gcc
TARGET = mycrawler
CFLAGS = -O2 -Wall
LFLAGS = -pthread

all: webCrawler.o workersPInfo.o queue.o HashTable.o worker.o jobExecutor.o Trie.o inetStrCrawler.o communication.o PollargsList.o FileList.o stats.o WordQList.o workerThread.o helpers.o PostingList.o workersInfo.o
	$(CC) -o $(TARGET) $^ $(LFLAGS)

clean:
	rm -f *.o
	rm -f $(TARGET)

webCrawler.o: webCrawler.c communication.h WordQList.h constants.h stats.h workerThread.h helpers.h HashTable.h workersInfo.h queue.h inetStrCrawler.h jobExecutor.h worker.h Trie.h PostingList.h FileList.h workersPInfo.h PollArgsList.h
	$(CC) $(CFLAGS) -c webCrawler.c -o webCrawler.o

workersPInfo.o: workersPInfo.c workersPInfo.h
	$(CC) $(CFLAGS) -c workersPInfo.c -o workersPInfo.o

queue.o: queue.c queue.h HashTable.h WordQList.h constants.h
	$(CC) $(CFLAGS) -c queue.c -o queue.o

HashTable.o: HashTable.c HashTable.h WordQList.h constants.h
	$(CC) $(CFLAGS) -c HashTable.c -o HashTable.o

worker.o: worker.c workersPInfo.h worker.h Trie.h WordQList.h constants.h PostingList.h FileList.h helpers.h communication.h
	$(CC) $(CFLAGS) -c worker.c -o worker.o

jobExecutor.o: jobExecutor.c jobExecutor.h constants.h helpers.h WordQList.h communication.h worker.h Trie.h PostingList.h FileList.h workersPInfo.h PollArgsList.h
	$(CC) $(CFLAGS) -c jobExecutor.c -o jobExecutor.o

Trie.o: Trie.c Trie.h WordQList.h constants.h PostingList.h FileList.h helpers.h communication.h
	$(CC) $(CFLAGS) -c Trie.c -o Trie.o

inetStrCrawler.o: inetStrCrawler.c inetStrCrawler.h constants.h
	$(CC) $(CFLAGS) -c inetStrCrawler.c -o inetStrCrawler.o

communication.o: communication.c communication.h WordQList.h constants.h
	$(CC) $(CFLAGS) -c communication.c -o communication.o

PollargsList.o: PollargsList.c PollArgsList.h constants.h
	$(CC) $(CFLAGS) -c PollargsList.c -o PollargsList.o

FileList.o: FileList.c FileList.h PostingList.h helpers.h WordQList.h constants.h communication.h
	$(CC) $(CFLAGS) -c FileList.c -o FileList.o

stats.o: stats.c stats.h
	$(CC) $(CFLAGS) -c stats.c -o stats.o

WordQList.o: WordQList.c WordQList.h constants.h
	$(CC) $(CFLAGS) -c WordQList.c -o WordQList.o

workerThread.o: workerThread.c workerThread.h helpers.h WordQList.h constants.h communication.h HashTable.h workersInfo.h queue.h stats.h inetStrCrawler.h
	$(CC) $(CFLAGS) -c workerThread.c -o workerThread.o

helpers.o: helpers.c helpers.h WordQList.h constants.h communication.h
	$(CC) $(CFLAGS) -c helpers.c -o helpers.o

PostingList.o: PostingList.c PostingList.h
	$(CC) $(CFLAGS) -c PostingList.c -o PostingList.o

workersInfo.o: workersInfo.c workersInfo.h queue.h HashTable.h WordQList.h constants.h stats.h
	$(CC) $(CFLAGS) -c workersInfo.c -o workersInfo.o
