all: jobExecutor worker

worker: Worker.o Trie.o ListNode.o
	gcc Worker.o Trie.o ListNode.o -o worker

jobExecutor: jobExecutor.o
	gcc jobExecutor.o -o jobExecutor

Worker.o: Worker.c
	gcc -c Worker.c

Trie.o: Trie.c
	gcc -c Trie.c

ListNode.o: ListNode.c
	gcc -c ListNode.c

jobExecutor.o: jobExecutor.c
	gcc -c jobExecutor.c

clean:
	rm -f worker jobExecutor Worker.o jobExecutor.o Trie.o ListNode.o