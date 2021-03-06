/*
  FILE: crawler.c
  Inputs: ./crawler [SEED URL] [TARGET DIRECTORY WHERE TO PUT THE DATA] [MAX CRAWLING DEPTH]
  Outputs: For each webpage crawled the crawler program will create a file in the 
  [TARGET DIRECTORY]. The name of the file will start a 1 for the  [SEED URL] 
  and be incremented for each subsequent HTML webpage crawled. 

  Description: Each file (e.g., 10) will include the URL associated with the saved webpage and the
  depth of search in the file. The URL will be on the first line of the file 
  and the depth on the second line. The HTML will for the webpage 
  will start on the third line.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/crawler.h"
#include "../include/hash.h"
#include "../include/header.h"
#include "../include/my_gumbo.h"

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
// to the curr URL search. 

DICTIONARY* dict = NULL; 
//const char *URL_PREFIX = "https://home.dartmouth.edu"; 		    /* Uncomment this line and comment out JHU one below + make new build for different results */
const char *URL_PREFIX = "https://www.jhu.edu";
int HASHES[MAX_HASH_SLOT]; 
FILE *out; 
int max_depth = 3 ;


/*we first download our web pages via getPage() that uses "wget". Once we get a downloaded page, we want to get all the URLS 
 * through "extractURLS()" updating our global DICTIONARY structure as well. We repeat this process by visiting the 
 */
int main(int argc, char *argv[]) {
	out = fopen("logger_crawler.txt", "w+"); 
	commandLine(argc, argv); 
	int* gh_idx = initLists() ;
	char* seedURL = argv[1];  			       	   	   // example would be "www.cs.dartmouth.edu"
	char* target_dir = argv[2];
	int max_depth  = atoi(argv[3]);  
	char url_to_visit[MAX_URL_LENGTH] = {0}; 
	int curr_depth = 0;
	int curr_hash_to_view = 0; 
        int url_list_length = 0; 
	// start downloading pages, extracting URLS, and repeat
	char* page = getPage(seedURL, curr_depth, target_dir, &curr_hash_to_view) ;
	char** url_list = extractURLs(page, seedURL, &url_list_length);
	updateListLinkToBeVisited(url_list, url_list_length, curr_depth, gh_idx); 
	setURLasVisited(seedURL, &curr_hash_to_view); 
	fprintf(out, "Hash Index Count: %d\n", *gh_idx); 
	while (snprintf(url_to_visit, MAX_URL_LENGTH, "%s", getAddressToBeVisited(&curr_depth, &curr_hash_to_view)) != 0){ 
		if (strcmp(url_to_visit, "(null)") == 0) {
			break;
		}
		printf("Next url to visit is: %s\n", url_to_visit);
		fprintf(out, "Next url to visit is: %s\n", url_to_visit);
		if (curr_depth > max_depth) {  				    // for urls over max_depth, set them to be visited and continue  
			setURLasVisited(url_to_visit, &curr_hash_to_view);  // mark current url visited 
			continue; 
		}
		if (strstr(url_to_visit, "javascript") || strstr(url_to_visit, "assets")) {
			fputs("Skipping this url because it's javascript. Wget can't download\n", out);
			setURLasVisited(url_to_visit, &curr_hash_to_view); // mark bad url as visited 
			continue; 
		}
							         	    // get html into string and return as page. Also save as a file into target dir 
		char *page = getPage(url_to_visit, curr_depth, target_dir, &curr_hash_to_view);	
		if ((page == NULL) || (page[0] == '\0')  || (strcmp(page, "") == 0) || (strstr(page, "PDF"))) {
			printf("warning! Cannot crawl current url. Most likely bad URL link. Continuing on..\n"); 
			fprintf(out, "warning! Cannot crawl current url. Most likely bad URL link. Continuing on..\n"); 
			setURLasVisited(url_to_visit, &curr_hash_to_view); // mark bad url as visited 
			continue; 
		}
		char** url_list = extractURLs(page, url_to_visit, &url_list_length); 
		int num_urls = url_list_length; 					     // for cleaning up purposes 
		updateListLinkToBeVisited(url_list, url_list_length, curr_depth+1, gh_idx);  // here the current depth increments 
		for (int i = 0; i < num_urls; ++i) {
			free(url_list[i]); 
		}
		setURLasVisited(url_to_visit, &curr_hash_to_view); 
		fprintf(out, "Hash Index Count: %d\n", *gh_idx); 
		memset(url_to_visit, 0, MAX_URL_LENGTH); 
		sleep(INTERVAL_PER_FETCH+1); 
	}
	// clean up the rest of the dynamically allocated memory portions 
	for (int i = 0; i < MAX_HASH_SLOT; ++i){
		if (dict->hash[i]) {
			fprintf(out, "freeing URLNODE at hash index: %d\n", i);
			free(dict->hash[i]->data);
		}			
		free(dict->hash[i]);
	}
	free(dict); 
	free(gh_idx);

	fputs("Freed memory.\n", out);
	printf("Freed memory.\n");
	printf("Finished!\n");
	fprintf(out, "Finished!\n");
	fclose(out);	
	return 0;
}

// (1) -- Command line processing on arguments -- ensuring the right # and general correctness of arguments are passed in 

void commandLine(int argc, char *argv[]) {
	if (argc != 4) {
		perror("Not enough arguments supplied\n");
		exit(1);
	}
	if (atoi(argv[3]) > max_depth) {
		perror("Depth is greater than max depth\n");
		exit(2); 
	}
	FILE *dir = fopen(argv[2], "r"); 
	if (dir == NULL) {
		perror("Directory to store HTMLs not supplied\n");
		exit(3);
	}
	fclose(dir); 
}

//(2) - initLists (properly intialize the Dictionary Hash Table) as well as the hash index counter 
//      that will make sure to keep track of all the pending URLS to expand when we crawl 

int *initLists(){
	dict = (DICTIONARY*) malloc(sizeof(DICTIONARY));
	MALLOC_CHECK(dict);
	
	memset(HASHES, 0, MAX_HASH_SLOT); 
	int *gh_idx = malloc(sizeof(int));  
	MALLOC_CHECK(gh_idx); 
	*gh_idx = 0; 

	for (int i =0; i < MAX_HASH_SLOT ; ++i) {
		dict->hash[i] = NULL; 
	}
	return gh_idx; 
}

// (3) --  getPage  -> using wget to download HTML page from seedURL. A file will be written
// to the target directory with specific information for the INDEXER file to work with. 
// Such information will include the curr_hash which has to be passed in as well 

char *getPage(char *seedURL, int curr_depth, char *target_directory, int *curr_hash) {
	char command[MAX_URL_LENGTH];  
	snprintf(command, MAX_URL_LENGTH, "wget %s -O buf.html", seedURL); 
	system(command);
	FILE *html = fopen("buf.html", "r"); 
	if (html == NULL) {
		perror("Wget didn't properly download to a temp file\n"); 
		exit(1);
	}
	fseek(html, 0, SEEK_END);						 // seek to the end of the file 
	int html_size = ftell(html); 						 // get current size of file after going to end 
	rewind(html);  	         						 // rewind all the way back to read in later to buffer 

	char *buf = malloc(sizeof(char) * html_size); 
	MALLOC_CHECK(buf);
	memset(buf, 0, html_size); 
		
	size_t result = fread(buf, sizeof(char), html_size, html); 		 // read into buffer
	if (result != html_size) {
		fputs("reading error", stderr); 
		exit(2); 
	}
	// Create a new file whose name will start at 1 for seedURL and increment for every new file 
	// Format of file: URL, depth, HTML 
	char file[5]; 
	snprintf(file, 5, "%d", *curr_hash); 					// we use the curr_hash to name our file 
	FILE *curr = fopen(file, "wb+"); 
	fprintf(curr, "%s\n", seedURL); 
	fprintf(curr, "%d\n", curr_depth);
	fprintf(curr, "%s", buf); 
	memset(command, 0, MAX_URL_LENGTH); 
	snprintf(command, MAX_URL_LENGTH, "mv %s %s", file, target_directory);  // moving file to TARGET_DIR
	fprintf(out, "Moving file %s to %s\n", file, target_directory);  
	system(command); 
	fclose(html);    							// cleaning up by closing the file stream and removing the temp file 
	fclose(curr); 
	system("rm buf.html");
	return buf;
}

// (4) -- extractURLs(page, SEED_URL). After downlaoding the HTML, we need to get the URLs. The parser function the professor provided 
// does not work. There is a Google open-source parser in C called Gumbo that was modified and used to parse the urls here 
//
// NOTE: This is still not a perfect solution because depending on the SEED, you can get very strange href links such as Javascript 
//  	 assets that can't be parsed. Some general error-handling is done here and in the main function for the ones encountered while developing. 

char** extractURLs(char *page, char *seedURL, int *url_list_length) {
	char *url_results = all_urls(&page); 					 // get all the URLS from the Gumbo parser 
	int num_of_urls = 0;   							 // get the number of URL strings we need to malloc later 
	for (int i = 0; i < strlen(url_results); ++i) {
		if (url_results[i] == '\n') { 					 //allocating more char* memories than will actually be needed 
			++num_of_urls;
		}
	}
	// The table that keeps pointers to a list of URLs extracted from current HTML page 
	char **url_list = malloc(sizeof(char*) * num_of_urls);
	MALLOC_CHECK(url_list); 
	for (int i = 0; i < num_of_urls; ++i) {
		url_list[i] = calloc(MAX_URL_LENGTH, sizeof(char)); 
		MALLOC_CHECK(url_list[i]); 
	}
	int len = strlen(url_results); 
	int n = 0;
	int single_index = 0; 
	char single[MAX_URL_LENGTH] = {0};  					// buffer for a single url to be placed here 
	int url_list_index = 0; 
	snprintf(url_list[url_list_index++], MAX_URL_LENGTH, "%s", seedURL);	// first entry will be the seedURL  
	while (n < len-1) {
		if (url_results[n] == '\n') { 					// detect line break, store/capture our single URL and check if it is valid 
			single[single_index] = '\0'; 
			char *found = strstr(single, URL_PREFIX);   	        // detect substring of similar URL to seed
			if (found) {
				char *detect_pdf = strstr(single, ".pdf");      // don't extract pdfs 
				if (detect_pdf) {
					;
				}
				else {
				       	snprintf(url_list[url_list_index], MAX_URL_LENGTH, "%s", single); 
					++url_list_index; 
				}
			}
			memset(single, 0, MAX_URL_LENGTH);  			// reset the buffer for next URL 
			++n; 							
			single_index = 0;  					
		}
		else { 
			single[single_index++] = url_results[n++]; 
			if (single_index % 500 == 0) {  		        // gumbo can return bad (and extremely long) URL parsing. Need to break out
				char *detect_good = strstr(single, "https://"); // if can't find this prefix then it's a bad link and why it's > 500 chars  
				if (detect_good == NULL) {
					fprintf(out, "DETECTED BAD URL - url buffer at this point: %s\n", single);
					break;
				}
			}
		}
	}	
	// Not enough to reallocate-- need to also set these pointers to NULL 
	// otherwise the next function will keep iterating and trying to dereference empty pointers 
	// because they're all set to 0 after the realloc instead of explicit null. 
	for (int i = url_list_index+1 ; i < num_of_urls ; ++i) {  
		free(url_list[i]);
		url_list[i] = NULL; 
	}

	*url_list_length = url_list_index;

	return url_list; 
}

//(5) updateListLinkToBeVisited(URLsLists, curr_depth + 1) - This is where we create a node around our URL (URLNODE) 
// and place it in the dictionary hash table. If there are collisions, then we have to set up a linked list at that index in the 
// hash table and iterate through the end until we can add our new node. Otherwise, we can proceed normally and create an 
// entry into our hash table 

void updateListLinkToBeVisited(char** url_list, int url_list_length, int depth, int* gh_idx) {
	int n = 0;
	int curr_hash = 0; 
	unsigned long hash_value = 0; 
	while ((url_list[n]) && (url_list_length >0)) {
		if ((url_list[n] == NULL) || (url_list[n] == '\0')) {
			break;
		}
		hash_value = hash1(url_list[n]) % MAX_HASH_SLOT ;
		curr_hash = hash_value;  
		if (dict->hash[hash_value] == NULL ) { 
			URLNODE *node = malloc(sizeof(URLNODE)); 
			MALLOC_CHECK(node); 
			node->depth = depth; 
			node->visited = 0; 
			snprintf(node->url, MAX_URL_LENGTH, "%s", url_list[n]); 
			dict->hash[hash_value] = malloc(sizeof(DNODE)); 
			MALLOC_CHECK(dict->hash[hash_value]); 
			dict->hash[hash_value]->data = node; 
			snprintf(dict->hash[hash_value]->key, KEY_LENGTH, "%s", node->url); 
			dict->hash[hash_value]->next = NULL; 
			dict->hash[hash_value]->prev = NULL; 
			HASHES[(*gh_idx)++] = curr_hash;   	   // we store this hash for getAddressToBeVisited() to extract the DNODE* here and continue process
			fprintf(out, "Adding node...currdepth: %d url_val->%s\n", dict->hash[hash_value]->data->depth, dict->hash[hash_value]->data->url);
		}
		// Collision could occur because
			// 1.) same exact url link (so not unique) 
			// 2.) genuine collision occurred of different values mapping to the same hash index 
		else if (dict->hash[hash_value]) {  
			DNODE *coll_curr = dict->hash[hash_value]; 
			bool proceed = true; 
			while (coll_curr) {
				if (strcmp(coll_curr->key, url_list[n]) == 0) { 
					proceed = false; 
					break; 
				} 
				coll_curr = coll_curr->next;
			}
			if (proceed) { 
				URLNODE *url_node = malloc(sizeof(URLNODE)); 
				MALLOC_CHECK(url_node); 
				url_node->depth = depth; 
				url_node->visited = 0; 
				snprintf(url_node->url, MAX_URL_LENGTH, "%s", url_list[n]); 
				DNODE *new = malloc(sizeof(DNODE)); 
				MALLOC_CHECK(new); 
				new->data = url_node;  		   
				snprintf(new->key, KEY_LENGTH, "%s", url_list[n]); 
				new->next = NULL;
				new->prev = coll_curr->prev; 
				coll_curr->prev->next = new; 
				HASHES[(*gh_idx)++] = curr_hash;   
				fputs("Collision occurred\n", out); 
				fprintf(out, "\nAdding node to linked list at hashed index...currdepth: %d url_val->%s\n", dict->hash[hash_value]->data->depth, dict->hash[hash_value]->data->url);
			}
		}
		++n;
		--url_list_length; 
	}
}

//(6) setURLasVisited -> set url as visited so will not visit again 

void setURLasVisited(char *seedURL, int *curr_hash_idx) {          // pass in the SEED URL 
	unsigned long hash_value = hash1(seedURL) % MAX_HASH_SLOT; // compute hash value for url parameter 
	unsigned long same_hash = hash_value; 
	DNODE *pointer = dict->hash[hash_value];     		   // access hash table for that URL hash value 
	while (pointer) { 
		if (strcmp(pointer->data->url, seedURL) == 0) {    // found the URL that share the same hash key (collision) 
			pointer->data->visited = 1; 
			fprintf(out, "Finished visiting url at hash index: %d\n", *curr_hash_idx); 
			(*curr_hash_idx)++;    			   // increment the current hash value 
			break;
		}
		pointer = pointer->next;      			   // else we iterate through linked list until we find the matching URL 
		hash_value = hash1(pointer->key);
		if (hash_value != same_hash) {
			break;
		}
	}
}

/* (7) This function checks the hash table for the next node to visit that hasn't already been visited 
 * and get that address for the crawler to use. 
 */		
char *getAddressToBeVisited(int *depth, int *curr_hash_idx) {
	int hash_value = HASHES[*curr_hash_idx]; 
	DNODE *current = dict->hash[hash_value]; 
	while (current) {
		if (current->data->visited) { 			    // positive value means visited so skip 
			current = current->next; 
		}
		else {
			current->data->depth = *depth;
			return current->data->url; 
		}
	}
	return NULL;
}


