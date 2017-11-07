#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

/* ---
   Implementation of a Linked List to be used later.
   --- */
typedef struct {
	void *data;
	void *link;
} Node;

typedef struct {
	Node *head;
} LList;

LList* create_llist() {
	LList* list = malloc(sizeof(LList));
	list.head = NULL;
	return list;
}

/*
 * Simple Head Insertion Method.
 */
insert(LList l, void *data) {
	Node* node = malloc(sizeof(Node));
	node.data = data;
	node.link = l.head;
	l.head = node;
}

static char* titles[];
static int TITLES_LEN;

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
LList *sentence_chop(char* text_buffer);
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
    while((bytes_read += read(file_fd, text_buffer + bytes_read, stat_data.st_blksize)) < stat_data.st_size);
    /* End the text buffer with a null character. */
    *(text_buffer + stat_data.st_size + 1) = '\0';
    
    /* Get rid of all newlines and tabs. */
    cleanup(text_buffer);
	/* Load titles for use in sentence chopping */
    
    printf("%s\n", text_buffer);
    return 0;
}

int is_title(char *text_buffer, int word_len) {
	for(int i = 0; i < TITLES_LEN; i++) {
		if(strlen(titles[i]) > word_len)
			continue;
		if(strcmp(titles[i], *(text_buffer - strlen(titles[i])), strlen(titles[i])) == 0)
			return 0;
	}
	return 1; 
}

/*
 * Chops sentences into linked list.
 * Requires that title array be 
 */
LList *sentence_chop(char* text_buffer) {
	LList *sen_list = (LList*) malloc(sizeof(LList));
	int current_word_len = 0;
	int new_sentence = 1;
	for(char *c = text_buffer; *c != '\0'; c++) {
		/*Spaces signal the end of a word*/
		if(*c == ' ')
			current_word_len = 0;
		/*If we find a period then make sure it really is a period and not the end
		 *of a title like Mr, Mrs, or Dr. If it is really a period than we need
		 *to record this sentence beginning in the linked list.*/
		else if(*c == '.') {
			if(current_word_len >= 2 && current_word_len <= 3 && !is_title(c, current_word_len)) {
				new_sentence = 0;
				current_word_len = 0;
				continue;
			}
		} else {
			current_word_len++;
		}

		/*If new sentence is needed, add it.*/
		if(new_sentence) {
			insert(sen_list, c);
		}
	}
	return sen_list;
}