#ifndef _INDEXER_H_
#define _INDEXER_H_


// *****************Impementation Spec********************************
// File: indexer.c
// This file contains useful information for implementing the crawler:
// - DEFINES
// - MACROS
// - DATA STRUCTURES
// - PROTOTYPES

#define WORD_LENGTH 1000
#define MAX_HASH_SLOT 20000

// DATA STRUCTURES. All these structures should be malloc 'd

// This is the key data structure that holds the information of each URL.

typedef struct _DocumentNode {
	struct _DocumentNode *next;  // pointer to next member in list 
	int docID;		     // doc identifier  
	int page_word_frequency;     // # of occurrences in word  
} _DocumentNode; 

typedef struct _DocumentNode DocNode; // alias 

typedef struct _WordNode {
	struct _WordNode *prev;       // pointer to prev word 
	struct _WordNode *next;	      // pointer to next word  
	char word[WORD_LENGTH];       
	DocNode *page;		      // pointer to first element in page list 
} _WordNode; 

typedef struct _WordNode WordNode; 

WordNode *WordList; 

typedef struct _INVERTED_INDEX {
	WordNode *hash[MAX_HASH_SLOT];  // hash slot 
} _INVERTED_INDEX;
//PROTOTYPES used by indexer.c You have to code them.

#endif

