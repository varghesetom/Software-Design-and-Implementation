/*

  FILE: crawler.c

  Description:

  Inputs: ./crawler [SEED URL] [TARGET DIRECTORY WHERE TO PUT THE DATA] [MAX CRAWLING DEPTH]

  Outputs: For each webpage crawled the crawler program will create a file in the 
  [TARGET DIRECTORY]. The name of the file will start a 1 for the  [SEED URL] 
  and be incremented for each subsequent HTML webpage crawled. 

  Each file (e.g., 10) will include the URL associated with the saved webpage and the
  depth of search in the file. The URL will be on the first line of the file 
  and the depth on the second line. The HTML will for the webpage 
  will start on the third line.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crawler.h"
#include "html.h"
#include "hash.h"
#include "header.h"

// Define the dict structure that holds the hash table 
// and the double linked list of DNODES. Each DNODE holds
// a pointer to a URLNODE. This list is used to store
// unique URLs. The search time for this list is O(n).
// To speed that up to O(1) we use the hash table. The
// hash table holds pointers into the list where 
// DNODES with the same key are maintained, assuming
// the hash(key) is not NULL (which implies the URL has
// not been seen before). The hash table provide quick
// access to the point in the list that is relevant
// to the current URL search. 

DICTIONARY* dict = NULL; 


// This is the table that keeps pointers to a list of URL extracted
// from the current HTML page. NULL pointer represents the end of the
// list of URLs.


char *url_list[MAX_URL_PER_PAGE]; 

// (1) -- Command line processing on arguments 

void commandLine(int argc, char *argv[]) {
	if (argc != 4) {
		perror("Not enough arguments supplied\n");
		exit(1);
	}
	FILE *dir = fopen(argv[2], "r"); 
	if (dir == NULL) {
		perror("Directory to store HTMLs not supplied\n");
		exit(1);
	}
	fclose(dir); 


}

//(2) - itLists (properly intialize the Dictionary Hash Table) 

void initLists(){
	dict = (DICTIONARY*) malloc(sizeof(DICTIONARY));
	MALLOC_CHECK(dict);
	memset(dict, 0, sizeof(DICTIONARY)); 
//	BZERO(dict, sizeof(DICTIONARY));
	dict->start = NULL;
	dict->end = NULL; 
}

// (3) -- 


/* 1. need to use wget to execute and download seedURL to temp file 
 * 2. to do this need to determine length of this temp file 
 *  (remember, it has to store all the HTML information so this will be a pretty long file )
 * 3. return pointer to buffer 
*/

char *getPage(char *seedURL, int current_depth, char *target_directory) {
	char command[MAX_URL_LENGTH+25];  // +5 for null terminator and wget
	snprintf(command, MAX_URL_LENGTH+25, "wget %s -O buf.html", seedURL); 
	system(command);
	FILE *html = fopen("buf.html", "r"); 
	if (html == NULL) {
		perror("Wget didn't properly download to a temp file\n"); 
		exit(1);
	}
	fseek(html, 0, SEEK_END); // seek to the end of the file 
	int html_size = ftell(html);  // get current size of file after going to end 
	rewind(html);  	          // rewind all the way back to read in later to buffer 
	char *buf = malloc(sizeof(char) * html_size + 1); // +1 for null-terminator 
	MALLOC_CHECK(buf);
	memset(buf, 0, html_size); // need to initialize malloc'ed buffer before using 
	size_t result = fread(buf, sizeof(char), html_size, html); // read into buffer
	if (result != html_size) {
		fputs("reading error", stderr); 
		exit(2); 
	}
	fclose(html);   // cleaning up by closing the file stream and removing the temp file
	char remove[15]; 
	snprintf(remove, 15, "rm buf.html"); 

	return buf;
}


/*


// Input command processing logic

(1) Command line processing on arguments
    Inform the user if arguments are not present
    IF target_directory does not exist OR depth exceeds max_depth THEN
       Inform user of usage and exit failed

// Initialization of any data structures

(2) *initLists* Initialize any data structure and variables

// Bootstrap part of Crawler for first time through with SEED_URL

(3) page = *getPage(seedURL, current_depth, target_directory)* Get HTML into a string and return as page, 
            also save a file (1..N) with correct format (URL, depth, HTML) 
    IF page == NULL THEN
       *log(PANIC: Cannot crawl SEED_URL)* Inform user
       exit failed
(4) URLsLists = *extractURLs(page, SEED_URL)* Extract all URLs from SEED_URL page.
  
(5) *free(page)* Done with the page so release it

(6) *updateListLinkToBeVisited(URLsLists, current_depth + 1)*  For all the URL 
    in the URLsList that do not exist already in the dictionary then add a DNODE/URLNODE 
    pair to the DNODE list. 

(7) *setURLasVisited(SEED_URL)* Mark the current URL visited in the URLNODE.

// Main processing loop of crawler. While there are URL to visit and the depth is not 
// exceeded keep processing the URLs.

(8) WHILE ( URLToBeVisited = *getAddressFromTheLinksToBeVisited(current_depth)* ) DO
        // Get the next URL to be visited from the DNODE list (first one not visited from start)
 
      IF current_depth > max_depth THEN
    
          // For URLs that are over max_depth, we just set them to visited
          // and continue on
    
          setURLasVisited(URLToBeVisited) Mark the current URL visited in the URLNODE.
          continue;

    page = *getPage(URLToBeVisited, current_depth, target_directory)* Get HTML into a 
            string and return as page, also save a file (1..N) with correct format (URL, depth, HTML) 

    IF page == NULL THEN
       *log(PANIC: Cannot crawl URLToBeVisited)* Inform user
       setURLasVisited(URLToBeVisited) Mark the bad URL as visited in the URLNODE.
       Continue; // We don't want the bad URL to stop us processing the remaining URLs.
   
    URLsLists = *extractURLs(page, URLToBeVisited)* Extract all URLs from current page.
  
    *free(page)* Done with the page so release it

    *updateListLinkToBeVisited(URLsLists, current_depth + 1)* For all the URL 
    in the URLsList that do not exist already in the dictionary then add a DNODE/URLNODE 
    pair to the DNODE list. 

    *setURLasVisited(URLToBeVisited)* Mark the current URL visited in the URLNODE.

    // You must include a sleep delay before crawling the next page 
    // See note below for reason.

    *sleep(INTERVAL_PER_FETCH)* Sneak by the server by sleeping. Use the 
     standard Linux system call

(9)  *log(Nothing more to crawl)

(10) *cleanup* Clean up data structures and make sure all files are closed,
      resources deallocated.

*/


int main(int argc, char *argv[]) {
	commandLine(argc, argv); 
	char *seedURL = argv[1]; 
	int current_depth = atoi(argv[2]); 
	char *target_dir = argv[3];
	char *page = getPage(seedURL, current_depth, target_dir) ;
	printf("page is:\n %s", page); 
	free(page); 
	return 0;
}
