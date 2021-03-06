
#include <stdio.h> 
#include <stdlib.h> 
#include "../include/indexer.h" 
#include "../include/allocate.h"
#include "../include/saveClean.h" 

/* File: saveClean.c 
 * Input: INVERTED_INDEX* from indexer.c 
 * Output: 
 * 	An index file that will have every word we extracted into our text files recorded with the total number 
	 * of documents it appears in as well as how many times it appears in each of the documents. 
 * 	For example, "administration 3 5 2 10 1 21 2" indicates that the word administration appears 3 times
 * 		2 times in document ID 5 ("text_5") 2 times, document ID 10 once, and 2 times in document ID 21 
 * Description: 
 * 	Once our index data structure is completed, we now format our results into a .dat file in our "saveIndex()" 
 * 	function. The rest of the functions deal with cleaning up our memory afterwards. 
 */

void saveIndex(INVERTED_INDEX* index, char* filename, FILE* logger) {
	int cur, doc_count;
	cur = 0; 
	doc_count = 0; 
	FILE *index_output = openFile(filename, "wb"); 
	while (cur < MAX_HASH_SLOT) {
		if (index->hash[cur]) {   			// we won't know how many values in the hash are NULL 
			DocNode* doc_page = index->hash[cur]->page;
			DocNode* cur_page = index->hash[cur]->page; 
			while (doc_page) {  			 // we need to first print out the overall Doc Count before going into each specific doc
				++doc_count; 	
				doc_page = doc_page->next; 
			}
			fprintf(index_output, "\n%s %d ", index->hash[cur]->word, doc_count); 
			while (cur_page) {  			 // get document id and frequency for index.dat 
				fprintf(index_output, "%d %d ", cur_page->docId, cur_page->page_word_frequency); 
				cur_page = cur_page->next; 
			}
			fprintf(logger, "Total doc count for word \"%s\": %d\n", index->hash[cur]->word, doc_count);
			doc_count = 0; 				 // reset for next word 
		}
		++cur; 
	}
	fclose(index_output); 
}

void cleanUp(INVERTED_INDEX* index, FILE* logger) {
	fprintf(logger, "Now cleaning..\n"); 
	for (int i =0; i < MAX_HASH_SLOT; ++i) {
		cleanUpWordNodesAtHashSlot(index, i); 
	}
	free(index); 
	fputs("Successfully cleaned\n", logger);
	index = NULL;
}

void cleanUpWordNodesAtHashSlot(INVERTED_INDEX* index, int idx) {
	WordNode* currWnode = index->hash[idx]; 
	if (currWnode == NULL) {
		free(currWnode);
		currWnode = NULL; 
	}
	else {
		while (currWnode) {    		 // there may be additional word nodes at this hash slot  
			cleanUpDocNodesFromWordNode(currWnode); 
			WordNode* remove = currWnode; 
			currWnode = currWnode->next; 
			free(remove); 
			remove = NULL; 
		}
	}	
}

void cleanUpDocNodesFromWordNode(WordNode* wnode) {
	DocNode* dnode = wnode->page;
	free(dnode); 
	/*
	DocNode* dnode = wnode->page; 
	while (dnode) {     			 // free every allocated document node 
		DocNode* freedNode = dnode;
		dnode = dnode->next;  		 // need to make sure they are all set to NULL after freeing
		free(freedNode); 
		freedNode = NULL; 
	}
	*/
}

