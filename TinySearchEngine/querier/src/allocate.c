
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include "../include/query.h" 
#include "../include/allocate.h" 

/* FILE: allocate.c 
 * Input: N/A 
 * Output: Allocated memory or open file streams 
 *
 * DESCRIPTION: Contains helper functions for opening files and dynamically allocating memory for structs and other data structures. Need to reduce the clutter that comes with checking for NULL after allocating and initializing struct members. Some are identical to the one in the indexer/ directory but other unique ones include the allocateDocNodeArray which initializes an array of structs. This is primarily used in query.c to keep track of all the queried documents we came across for a word.
 */

FILE* openFile(char* filename, char* mode) {
	FILE* out = fopen(filename, mode); 
	if (out == NULL) {
		printf("Filename: %s", filename); 
		perror("Can't open file. Exiting...\n");
		exit(EXIT_RETURN); 
	}
	return out; 
}

// helper check function to see whether our index.dat has an empty line in it. If so, then the querier will break so preemptively stopping execution with the error check 
void checkIndexDataFile(FILE* index_dat) {
	char checkEmpty[LINE_LENGTH]; 
	while (fgets(checkEmpty, LINE_LENGTH, index_dat)) {
		if (strcmp(checkEmpty, "\n") == 0) {
			perror("Empty line detected in index.dat. Exiting\n"); 
			exit(FAIL) ;
		}
	}
	rewind(index_dat); 
}

INVERTED_INDEX* allocateInvertedIndex(FILE*log) {
	INVERTED_INDEX* index = malloc(sizeof(INVERTED_INDEX)); 
	if (index == NULL) {
		perror("Not enough memory for index. Exiting..\n"); 
		fprintf(log, "Not enough memory for index. Exiting..\n"); 
		exit(1);
	}
	for (int i = 0; i < MAX_HASH_SLOT; ++i) {
		index->hash[i] = NULL; 
	}
	return index; 
}

WordNode* allocateWordNode(FILE* log) {
	WordNode* wnode = malloc(sizeof(WordNode)); 
	if (wnode == NULL) {
		perror("Couldn't allocate memory for word\n");
		fputs("Couldn't allocate memory for word\n", log); 
		exit(EXIT_RETURN); 
	}
	wnode->next = NULL;
	wnode->prev = NULL; 
	wnode->page = NULL; 
	return wnode; 
}

DocNode* allocateDocNodeArray(int totalDocs) {
	DocNode* allDocNodesPerWord = malloc(sizeof(DocNode) * totalDocs); 
	if (allDocNodesPerWord == NULL) {
		perror("Couldn't allocate enough memory for array of DocNodes for word\n"); 
	}
	for (int i = 0; i < totalDocs; ++i) {
		allDocNodesPerWord[i].docId = -1; 
	      	allDocNodesPerWord[i].page_word_frequency = 0; 
		allDocNodesPerWord[i].next = NULL; 
	}
	return allDocNodesPerWord; 
}

// for the AND case only -> since we're filtering down with each successive AND query we only need to do this once 
// This will create a linked list of shared docIds that we can later use to prompt the user to open up a file 
sharedDocId* allocateSharedId() {
	sharedDocId* sdoc = malloc(sizeof(sharedDocId)); 
	if (sdoc == NULL) {
		perror("Couldn't allocate memory for shared doc list\n"); 
		exit(EXIT_RETURN); 
	}
	sdoc->next = NULL; 
	sdoc->id = -1; 
	return sdoc; 
}



