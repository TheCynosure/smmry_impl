#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

/* ---
   Data Type to use later.
   --- */

typedef struct {
	char* key;
	char* val;
} Dt;

/* ---
   Implementation of a Linked List to be used later.
   --- */
   
typedef struct {
	char *data;
	void *link;
	int score;
} Node;

typedef struct {
	Node *head;
	Node *tail;
	int size;
} LList;

LList* create_llist() {
	LList* list = malloc(sizeof(LList));
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	return list;
}

/* Simple Tail Insertion Method. */
void insert(LList* l, char* data) {
	Node* node = malloc(sizeof(Node));
	node->data = data;
	node->link = NULL;
	if(l->head == NULL)
		l->head = node;
	else
		l->tail->link = node;
	l->tail = node;
	l->size++;
}

void freeList(LList *l) {
    Node* current_node = l->head;
    while(current_node != NULL) {
        Node* temp = current_node->link;
        free(current_node->data);
        free(current_node);
        current_node = temp;
    }
    free(l);
}

/* ---
 * End of Implementation of Linked List
   --- */

/* ---
 * Implementation of a BST to be used later.
   --- */

typedef struct {
	char *data;
	int word_len;
	int score;
	void* left_child;
	void* right_child;
} BSTNode;

typedef struct {
	BSTNode* root;
} Bst;

Bst* create_bst() {
	Bst* bst = malloc(sizeof(bst));
	bst->root = NULL;
	return bst;
}

/* Returns 1 if added a node, returns 0 otherwise. */
int add(Bst* bst, char* data, int word_len) {
	BSTNode** curr = &bst->root;
	int allocate = 1;
	while(*curr != NULL) {
		int shorter_word_len = ((*curr)->word_len < word_len)? (*curr)->word_len: word_len;
		int result = strncmp((*curr)->data, data, shorter_word_len);
		if(result == 0) {
		    /* Equal Strings */
		    (*curr)->score++;
		    allocate = 0;
		    break;
		} else if(result > 0)
			curr = (BSTNode**) &((*curr)->left_child);
		else {
			curr = (BSTNode**) &((*curr)->right_child);
		}
	}
	
	if(allocate) {
    	/* Allocate a new node */
    	BSTNode* node = malloc(sizeof(BSTNode));
    	node->data = data;
    	node->score = 1;
    	node->word_len = word_len;
    	node->left_child = NULL;
    	node->right_child = NULL;
    	*curr = node;
	}	 
	return allocate;
}

int get_score(Bst* bst, char* data, int word_len) {
	BSTNode** curr = &bst->root;
    while(*curr != NULL) {
		int shorter_word_len = ((*curr)->word_len < word_len)? (*curr)->word_len: word_len;
		int result = strncmp((*curr)->data, data, shorter_word_len);
		if(result == 0) {
		    return (*curr)->score;
		    break;
		} else if(result > 0)
			curr = (BSTNode**) &((*curr)->left_child);
		else {
			curr = (BSTNode**) &((*curr)->right_child);
		}
	}
	return 0;
}

void freeNode(BSTNode* node) {
    if(node == NULL)
        return;
    freeNode(node->left_child);
    freeNode(node->right_child);
    free(node->data);
    free(node);
}

void freeBst(Bst* bst) {
    freeNode(bst->root);
    free(bst);   
}

/* ---
 * End of Implementation
   --- */
 

#define TITLES_LEN 12
static char* titles[TITLES_LEN];
#define SYN_LEN 350
static Dt* synonyms[SYN_LEN];
#define NOUN_LEN 31
static Dt* irreg_nouns[NOUN_LEN];

/* Initially called to strip out newlines, tabs. */
void cleanup(char* text_buff);
/* Returns a list with pointers to the start of all sentences and their lengths. */
LList *sentence_chop(char* text_buffer);
/* Loads the titles array full of titles char pointers */
void load_titles(char* titles_file_path);
int is_title(char *text_buffer, int word_len);
void load_list(char* syns_path, Dt* list[]);
char* contains(Dt* list[], int l_size, char word[]);
void freeDtList(Dt* list[], int len);

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
	/* Load titles for use in sentence chopping. */
	load_titles("data/titles.txt");

	/* Load the sentences into this linked list. */
    LList *l = sentence_chop(text_buffer);

    /* Load our word lists. */
	load_list("data/formattedcommonsyns.txt", synonyms);
    load_list("data/formattedirregnouns.txt", irreg_nouns);

    Bst* word_bst = create_bst();
    /* Check for words in our sentence_chop, otherwise increment their normal counts in this BST. */
    Node* current_node = l->head;
    while(current_node != NULL) {
        char* c = current_node->data;
        char* curr_word = current_node->data;
        int curr_word_len = 0;
        for(c = current_node->data; *c != '\0'; c++) {
            if(*c == ' ') {
                /* Add word to our Bst. */
                char* word = malloc(256);
                strncpy(word, curr_word, curr_word_len);
                word[0] = tolower(word[0]);
                char* eq;
                if((eq = contains(synonyms, SYN_LEN, word)) != NULL) {
                    strcpy(word, eq);
                } else if((eq = contains(irreg_nouns, NOUN_LEN, word)) != NULL) {
                    strcpy(word, eq);
                }

                if(add(word_bst, word, curr_word_len) == 0)
                    free(word);
                
                curr_word = c + 1;
                curr_word_len = 0;
            } else {
                curr_word_len++;
            }
        }
        current_node = current_node->link;
    }
    
    /* Tally the scores of each sentence */
    current_node = l->head;
    int return_num = 3;
    Node* top_scorers[return_num];
    memset(top_scorers, 0, sizeof(Node*) * return_num);
    while(current_node != NULL) {
        current_node->score = 0;
        char* c = current_node->data;
        char* curr_word = current_node->data;
        int curr_word_len = 0;
        for(c = current_node->data; *c != '\0'; c++) {
            if(*c == ' ') {
                /* Add score to our current sentence score.  */
                current_node->score += get_score(word_bst, curr_word, curr_word_len);
                curr_word = c + 1;
                curr_word_len = 0;
            } else {
                curr_word_len++;
            }
        }

        int i;
        for(i = 0; i < return_num; i++) {
            if(top_scorers[i] == 0 || top_scorers[i]->score < current_node->score) {
                top_scorers[i] = current_node;
                break;
            }   
        }
        current_node = current_node->link;
    }

    int i;
    for(i = 0; i < return_num; i++) {
        printf("%s", top_scorers[i]->data);
    }
    
    /* Free all heap memory */
    freeDtList(synonyms, SYN_LEN);
    freeDtList(irreg_nouns, NOUN_LEN);
    for(i = 0; i < TITLES_LEN; i++) {
        free(titles[i]);
    }
    freeList(l);
    freeBst(word_bst);
    free(text_buffer);  
    return 0;
}

void freeDtList(Dt* list[], int len) {
    int i;
    for(i = 0; i < len; i++) {
        free(list[i]->key);
        free(list[i]->val);
        free(list[i]); 
    }
}

/* Checks if the word is in the specified DtList. Then returns the data if true or NULL
 * Otherwise.
 */
char* contains(Dt* list[], int l_size, char word[]) {
    int i;
    for(i = 0; i < l_size; i++) {
        if(strcmp(list[i]->key, word) == 0)
            return list[i]->val;
    }
    return NULL;
}

/* Loads our word list data into our arrays.
 * Specifically the synonyms and plural forms of words.
 */
#define MAX_LEN 1024
void load_list(char* syns_path, Dt* list[]) {
	char* line = (char*) malloc(MAX_LEN);
	FILE* syn_fd = fopen(syns_path, "r");	
	size_t n;
	int index = 0;		
	while(getline(&line, &n, syn_fd) != EOF) {
		char synonym[MAX_LEN], base_word[MAX_LEN];
		sscanf(line, "%s %s", synonym, base_word);
		/*Convert the first letter of base word to lower.*/
		base_word[0] = tolower(base_word[0]);
		/*Load into array*/
		Dt* new_dt = malloc(sizeof(Dt));
	
		char* synonym_cp = malloc(strlen(synonym));
		strcpy(synonym_cp, synonym);

		char* base_word_cp = malloc(strlen(base_word));
		strcpy(base_word_cp, base_word);

		new_dt->key = synonym_cp;
		new_dt->val = base_word_cp;
		
		list[index++] = new_dt;
	}
	free(line);
	fclose(syn_fd);
}

/* Initially called to strip out newlines, tabs. 
 * Modifies the text_buffer that is passed in, must be null terminated.
 */
void cleanup(char* text_buff) {
    char* c;
    for(c = text_buff; *c != '\0'; c++) {
        if(*c == '\n' || *c == ' ')
            *c = ' ';
    }
}

/*
 * Makes a string buffer out of the string between last_sen -> current_sen.
 * Then adds it into the ll passed in as a parameter.
 */
void chop_and_add(LList *ll, char* current_sen, char* last_sen) {
	int size_of_sen = current_sen - last_sen + 1;
	char *sentence = malloc(size_of_sen);
	snprintf(sentence, size_of_sen, "%s", last_sen);
	insert(ll, sentence);
}

/*
 * Chops sentences into linked list.
 * Requires that title array be 
 */
LList *sentence_chop(char* text_buffer) {
	LList *sen_list = (LList*) malloc(sizeof(LList));
	int current_word_len = 0;
	int new_sentence = 0;
	char *c;
	char *last_sentence = text_buffer;
	int inside_paren = 0;
	for(c = text_buffer; *c != '\0'; c++) {
		/*Spaces signal the end of a word*/
		if(*c == ' ')
			current_word_len = 0;
		/*If we find a period then make sure it really is a period and not the end
		 *of a title like Mr, Mrs, or Dr. If it is really a period than we need
		 *to record this sentence beginning in the linked list.*/
		else if(*c == '.') {
			if(!inside_paren && (*(c+1) == ' ' || *(c+1) == '\0') && !is_title(c, current_word_len)) {
				new_sentence = 1;
				current_word_len = 0;
				continue;
			}
		} else {
			current_word_len++;
			if(*c == '(') {
				inside_paren = 1;
			} else if(*c == ')') {
				inside_paren = 0;
			}
		}
		
		/*If new sentence is needed, add it.*/
		if(new_sentence) {
			chop_and_add(sen_list, c, last_sentence);
			last_sentence = c;
			new_sentence = 0;
		}
	}

	/*If we ended on a period, add it.*/
	if(new_sentence)
		chop_and_add(sen_list, c, last_sentence);

	return sen_list;
}

/*
 * Loads the titles from the titles file into the titles array.
 */
void load_titles(char *title_file_path) {
	int file_fd = open(title_file_path, O_RDONLY);
	if(file_fd < 0) {
		fprintf(stderr, "Failed to open titles file!\n");
		exit(3);
	}

	/* Get the file size */
	struct stat stat_data;
    fstat(file_fd, &stat_data);
    /* Create a text buffer the size of the text file plus a space for a \0 */
    char* text_buffer = malloc(stat_data.st_size + 1);
    if(text_buffer == NULL) {
        fprintf(stderr, "Problem allocating text buffer!\n");
        exit(2);
    }
    
    /* While bytes_read is less than the file size, keep reading in st_blksize byte chunks. */
    int bytes_read = 0;
    while((bytes_read += read(file_fd, text_buffer + bytes_read, stat_data.st_blksize)) < stat_data.st_size);
    /* End the text buffer with a null character. */
    *(text_buffer + stat_data.st_size + 1) = '\0';

	/* Loop through the character buffer and add words followed by a newline */
	char *c;
	char *last_newline = text_buffer;
	int title_index = 0;
    for(c = text_buffer; *c != '\0'; c++) {
    	if(*c == '\n') {
    		/* This does depend on the newline being in a valid place with a word following it.
    		 * Basically allocates space for all the titles and then puts them in the title array. */
    		last_newline++;
    		int title_size = c - last_newline;
    		titles[title_index] = (char*) malloc(title_size + 1);
    		strncpy(titles[title_index], last_newline, title_size);
    		*(titles[title_index] + title_size) = '\0';
    		last_newline = c;
    		title_index++;
    	}
    }
    free(text_buffer);
    close(file_fd);
}

/*
 * Returns 1 (true) if the word pointed to by text_buffer (on the last letter) and is of size word_len
 * is a title. Returns 0 (false) otherwise.
 */
int is_title(char *text_buffer, int word_len) {
	int i;
	for(i = 0; i < TITLES_LEN; i++) {
		if(word_len - 1 != strlen(titles[i]))
			continue;
		else if(strncmp(titles[i], text_buffer - strlen(titles[i]), strlen(titles[i])) == 0)
			return 1;
	}
	return 0; 
}