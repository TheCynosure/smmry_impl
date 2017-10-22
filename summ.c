#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

/* ---
   Implementation of a Linked List to be used later.
   --- */
typedef struct {
	
} Node;

typedef struct {
	
} LList;

/* ---
   Implementation of a Hash Map to be used later.
   --- */

typedef struct {
	
} HMap;

/* ---
   End of Implementation
   --- */


/* Initially called to strip out newlines, tabs. */
void cleanup(char* text_buff) {
	char* c;
	for(c = text_buff; *c != '\0'; c++) {
		if(*c == '\n')
			*c = ' ';
		else if(*c == '\t')
			*c = ' ';
	}
}
/* Returns a list with pointers to the start of all sentences and their lengths. */
LList sentence_chop();
/* Replaces the plurals in all sentences. */
void replace_plurals(LList sentence_list);
/* Replace the synonyms in all sentences. */
void replace_synonyms(LList sentence_list);
/* Get the occurences of all words in the list. */
HMap get_occurences(LList sentence_list);
/* Get the scores of the words. */
LList get_scores(HMap word_occurences, LList sentence_list);
/* Return the top number of sentences. */
char* concat_top_sentences(int num_of_top);

int main(int argc, char** argv) {
	int file_fd;
	/* No file was given, throw error */
	if(argc < 2) {
		fprintf(stderr, "You need to specify a file!\n");
		return 1;
	}

	/* Open the file and check if that worked */
	file_fd = open(argv[1], O_RDONLY);
	if(file_fd < 0) {
		fprintf(stderr, "Can't access file, quitting!\n");
		return 1;
	}
	
	/* Find the size of the file. */
	struct stat stat_data;
	fstat(file_fd, &stat_data);
	/* Create a text buffer the size of the text file plus a space for a \0 */
	char* text_buffer = malloc(stat_data.st_size + 1);
	if(text_buffer == NULL) {
		fprintf(stderr, "Problem allocating text buffer!\n");
		return 2;
	}


	/* While bytes_read is less than the file size, keep reading in 256 byte chunks. */
	int bytes_read = 0;
	while((bytes_read += read(file_fd, text_buffer + bytes_read, 256)) < stat_data.st_size);
	/* End the text buffer with a null character. */
	*(text_buffer + stat_data.st_size + 1) = '\0';

	/* Get rid of all newlines and tabs. */
	cleanup(text_buffer);
	
	printf("%s\n", text_buffer);
	return 0;
}

