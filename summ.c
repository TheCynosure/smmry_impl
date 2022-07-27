#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Custom Data Types */
typedef struct {
  char *key;
  char *val;
} Dt;

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

typedef struct {
  char *data;
  int word_len;
  int score;
  void *left_child;
  void *right_child;
} BSTNode;

typedef struct {
  BSTNode *root;
} Bst;

#define TITLES_LEN 12
static char *titles[TITLES_LEN];
#define SIMPLE_LEN 22
static char *simples[SIMPLE_LEN];
#define SYN_LEN 350
static Dt *synonyms[SYN_LEN];
#define NOUN_LEN 31
static Dt *irreg_nouns[NOUN_LEN];

/* Function Pre-Declarations */
void strip_newlines_tabs(char *text_buff);
void load_array(char *array[], char *titles_file_path);
LList *sentence_chop(char *text_buffer);
int is_title(char *text_buffer, int word_len);
void load_list(char *syns_path, Dt *list[]);
char *contains(Dt *list[], int l_size, char word[]);
void free_dt_list(Dt *list[], int len);
void add_words_to_bst(Bst *bst, LList *l);
void tally_get_top_scorers(Bst *word_bst, LList *l, int return_num,
                           Node *top_scorers[]);
void get_rid_of_simples(Bst *word_bst);

/* Data Type Functions */
LList *create_llist(void);
void insert(LList *l, char *data);
void free_list(LList *l);
Bst *create_bst(void);
int add(Bst *bst, char *data, int word_len);
int get_score(Bst *bst, char *data, int word_len);
BSTNode *get_node(Bst *bst, char *data, int word_len);
void free_node(BSTNode *node);
void free_bst(Bst *bst);

int main(int argc, char **argv) {
  /* Check for the wrong number of arguments */
  if (argc < 3) {
    fprintf(stderr, "Usage: ./summ <filename> <# of sentences for summary>\n");
    return 1;
  }

  /* Open the file and check for an error */
  int file_fd = open(argv[1], O_RDONLY);
  if (file_fd < 0) {
    fprintf(stderr, "Can't access file, quitting!\n");
    return 1;
  }

  /* Get the number of sentences to return */
  int return_num = 0;
  sscanf(argv[2], "%d", &return_num);

  /* Get the size of the file */
  struct stat stat_data;
  fstat(file_fd, &stat_data);

  /* Create a text buffer one bigger than the size of the file */
  char *text_buffer = malloc(stat_data.st_size + 1);
  if (text_buffer == NULL) {
    fprintf(stderr, "Problem allocating text buffer!\n");
    return 2;
  }

  /* Read the file into our text buffer */
  int bytes_read = 0;
  while ((bytes_read += read(file_fd, text_buffer + bytes_read,
                             stat_data.st_blksize)) < stat_data.st_size)
    ;

  /* Just incase, null terminate the buffer */
  *(text_buffer + bytes_read + 1) = '\0';

  /* Get rid of all newlines and tabs. */
  strip_newlines_tabs(text_buffer);

  /* Load titles for use in sentence chopping. */
  load_array(titles, "data/titles.txt");

  /* Load the sentences into this linked list. */
  LList *l = sentence_chop(text_buffer);

  /* Make sure we have enough sentences to return */
  if (return_num > l->size) {
    fprintf(stderr, "Too many return sentences, not a long enough text!\n");
    free(text_buffer);
    free_list(l);
    return 1;
  }

  /* Load our word lists. */
  load_list("data/formattedcommonsyns.txt", synonyms);
  load_list("data/formattedirregnouns.txt", irreg_nouns);

  /* Put the words into our BST */
  Bst *word_bst = create_bst();
  add_words_to_bst(word_bst, l);

  /* Set all simple words scores to zero */
  load_array(simples, "data/simplewords.txt");
  get_rid_of_simples(word_bst);

  /* Tally the scores of each sentence */
  Node *top_scorers[return_num];
  memset(top_scorers, 0, sizeof(Node *) * return_num);
  tally_get_top_scorers(word_bst, l, return_num, top_scorers);

  /* Print the top scoring sentences */
  int i;
  for (i = 0; i < return_num; i++) {
    printf("%s", top_scorers[i]->data);
  }
  printf("\n");

  /* Free all allocated heap memory */
  free_dt_list(synonyms, SYN_LEN);
  free_dt_list(irreg_nouns, NOUN_LEN);
  free_list(l);
  free_bst(word_bst);
  free(text_buffer);
  for (i = 0; i < TITLES_LEN; i++) {
    free(titles[i]);
  }

  for (i = 0; i < SIMPLE_LEN; i++) {
    free(simples[i]);
  }
  return 0;
}

/*
 * Makes a string buffer out of the string between last_sen -> current_sen.
 * Then adds it into the ll passed in as a parameter.
 */
void chop_and_add(LList *ll, char *current_sen, char *last_sen) {
  int size_of_sen = current_sen - last_sen + 1;
  char *sentence = malloc(size_of_sen);
  snprintf(sentence, size_of_sen, "%s", last_sen);
  insert(ll, sentence);
}

/* Initially called to strip out newlines, tabs.
 * Modifies the text_buffer that is passed in, must be null terminated.
 */
void strip_newlines_tabs(char *text_buff) {
  char *c;
  for (c = text_buff; *c != '\0'; c++) {
    if (*c == '\n' || *c == ' ')
      *c = ' ';
  }
}

/*
 * Loads the titles from the titles file into the titles array.
 */
void load_array(char *array[], char *file_path) {
  int file_fd = open(file_path, O_RDONLY);
  if (file_fd < 0) {
    fprintf(stderr, "Failed to open titles file!\n");
    exit(3);
  }

  /* Get the file size */
  struct stat stat_data;
  fstat(file_fd, &stat_data);
  /* Create a text buffer the size of the text file plus a space for a \0 */
  char *text_buffer = malloc(stat_data.st_size + 1);
  if (text_buffer == NULL) {
    fprintf(stderr, "Problem allocating text buffer!\n");
    exit(2);
  }

  /* While bytes_read is less than the file size, keep reading in st_blksize
   * byte chunks. */
  int bytes_read = 0;
  while ((bytes_read += read(file_fd, text_buffer + bytes_read,
                             stat_data.st_blksize)) < stat_data.st_size)
    ;
  /* End the text buffer with a null character. */
  *(text_buffer + bytes_read + 1) = '\0';

  /* Loop through the character buffer and add words followed by a newline */
  char *c;
  char *last_newline = text_buffer;
  int index = 0;
  for (c = text_buffer; *c != '\0'; c++) {
    if (*c == '\n') {
      /* This does depend on the newline being in a valid place with a word
       * following it. Basically allocates space for all the titles and then
       * puts them in the title array. */
      last_newline++;
      int size = c - last_newline;
      array[index] = (char *)malloc(size + 1);
      strncpy(array[index], last_newline, size);
      *(array[index] + size) = '\0';
      last_newline = c;
      index++;
    }
  }
  free(text_buffer);
  close(file_fd);
}

/*
 * Chops sentences into linked list.
 * Requires that title array be
 */
LList *sentence_chop(char *text_buffer) {
  LList *sen_list = (LList *)malloc(sizeof(LList));
  int current_word_len = 0;
  int new_sentence = 0;
  char *c;
  char *last_sentence = text_buffer;
  int inside_paren = 0;
  for (c = text_buffer; *c != '\0'; c++) {
    /*Spaces signal the end of a word*/
    if (*c == ' ')
      current_word_len = 0;
    /*If we find a period then make sure it really is a period and not the end
     *of a title like Mr, Mrs, or Dr. If it is really a period than we need
     *to record this sentence beginning in the linked list.*/
    else if (*c == '.') {
      if (!inside_paren &&
          (*(c + 1) == ' ' || *(c + 1) == '\0' || *(c + 1) == '(' ||
           *(c + 1) == '[') &&
          !is_title(c, current_word_len)) {
        new_sentence = 1;
        current_word_len = 0;
        continue;
      }
    } else {
      current_word_len++;
      if (*c == '(') {
        inside_paren = 1;
      } else if (*c == ')') {
        inside_paren = 0;
      }
    }

    /*If new sentence is needed, add it.*/
    if (new_sentence) {
      chop_and_add(sen_list, c, last_sentence);
      last_sentence = c;
      new_sentence = 0;
    }
  }

  /*If we ended on a period, add it.*/
  if (new_sentence)
    chop_and_add(sen_list, c, last_sentence);

  return sen_list;
}

/*
 * Returns 1 (true) if the word pointed to by text_buffer (on the last letter)
 * and is of size word_len is a title. Returns 0 (false) otherwise.
 */
int is_title(char *text_buffer, int word_len) {
  int i;
  for (i = 0; i < TITLES_LEN; i++) {
    if (word_len - 1 != strlen(titles[i]))
      continue;
    else if (strncmp(titles[i], text_buffer - strlen(titles[i]),
                     strlen(titles[i])) == 0)
      return 1;
  }
  return 0;
}

/* Loads our word list data into our arrays.
 * Specifically the synonyms and plural forms of words.
 */
#define MAX_LEN 1024
void load_list(char *syns_path, Dt *list[]) {
  char *line = (char *)malloc(MAX_LEN);
  FILE *syn_fd = fopen(syns_path, "r");
  size_t n;
  int index = 0;
  while (getline(&line, &n, syn_fd) != EOF) {
    char synonym[MAX_LEN], base_word[MAX_LEN];
    sscanf(line, "%s %s", synonym, base_word);
    /*Convert the first letter of base word to lower.*/
    base_word[0] = tolower(base_word[0]);
    /*Load into array*/
    Dt *new_dt = malloc(sizeof(Dt));

    char *synonym_cp = malloc(strlen(synonym));
    strcpy(synonym_cp, synonym);

    char *base_word_cp = malloc(strlen(base_word));
    strcpy(base_word_cp, base_word);

    new_dt->key = synonym_cp;
    new_dt->val = base_word_cp;

    list[index++] = new_dt;
  }
  free(line);
  fclose(syn_fd);
}

/* Checks if the word is in the specified DtList. Then returns the data if true
 * or NULL Otherwise.
 */
char *contains(Dt *list[], int l_size, char word[]) {
  int i;
  for (i = 0; i < l_size; i++) {
    if (strcmp(list[i]->key, word) == 0)
      return list[i]->val;
  }
  return NULL;
}

/* Frees a list with a double type */
void free_dt_list(Dt *list[], int len) {
  int i;
  for (i = 0; i < len; i++) {
    free(list[i]->key);
    free(list[i]->val);
    free(list[i]);
  }
}

void add_words_to_bst(Bst *word_bst, LList *l) {
  Node *current_node = l->head;
  while (current_node != NULL) {
    char *c = current_node->data;
    char *curr_word = current_node->data;
    int curr_word_len = 0;
    for (c = current_node->data; *c != '\0'; c++) {
      if (*c == ' ') {
        /* Add word to our Bst. */
        char *word = malloc(256);
        strncpy(word, curr_word, curr_word_len);
        word[0] = tolower(word[0]);
        char *eq;
        if ((eq = contains(synonyms, SYN_LEN, word)) != NULL) {
          strcpy(word, eq);
        } else if ((eq = contains(irreg_nouns, NOUN_LEN, word)) != NULL) {
          strcpy(word, eq);
        }

        if (add(word_bst, word, curr_word_len) == 0)
          free(word);

        curr_word = c + 1;
        curr_word_len = 0;
      } else {
        curr_word_len++;
      }
    }
    current_node = current_node->link;
  }
}

void tally_get_top_scorers(Bst *word_bst, LList *l, int return_num,
                           Node *top_scorers[]) {
  Node *current_node = l->head;
  while (current_node != NULL) {
    current_node->score = 0;
    char *c = current_node->data;
    char *curr_word = current_node->data;
    int curr_word_len = 0;
    for (c = current_node->data; *c != '\0'; c++) {
      if (*c == ' ') {
        /* Add score to our current sentence score if the word has appeared more
         * than once */
        int score = get_score(word_bst, curr_word, curr_word_len);
        if (score > 1)
          current_node->score += score;
        curr_word = c + 1;
        curr_word_len = 0;
      } else {
        curr_word_len++;
      }
    }

    int i;
    for (i = 0; i < return_num; i++) {
      if (top_scorers[i] == 0 || top_scorers[i]->score < current_node->score) {
        if (return_num > 1) {
          int k = i;
          Node *last_val = NULL;
          for (k; k < return_num; k++) {
            Node *temp = top_scorers[k];
            top_scorers[k] = last_val;
            last_val = temp;
          }
        }

        top_scorers[i] = current_node;
        break;
      }
    }
    current_node = current_node->link;
  }
}

/* Sets the score of the simple words in the BST to 0 */
void get_rid_of_simples(Bst *word_bst) {
  int i;
  for (i = 0; i < SIMPLE_LEN; i++) {
    BSTNode *n;
    if ((n = get_node(word_bst, simples[i], strlen(simples[i]))) != NULL)
      n->score = 0;
  }
}

/* Creates and returns a linked list */
LList *create_llist(void) {
  LList *list = malloc(sizeof(LList));
  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
  return list;
}

/* Tail insertion for the linked list */
void insert(LList *l, char *data) {
  Node *node = malloc(sizeof(Node));
  node->data = data;
  node->link = NULL;
  if (l->head == NULL)
    l->head = node;
  else
    l->tail->link = node;
  l->tail = node;
  l->size++;
}

/* Free the linked list and all of its nodes */
void free_list(LList *l) {
  Node *current_node = l->head;
  while (current_node != NULL) {
    Node *temp = current_node->link;
    free(current_node->data);
    free(current_node);
    current_node = temp;
  }
  free(l);
}

/* Creates and returns a bst */
Bst *create_bst(void) {
  Bst *bst = malloc(sizeof(bst));
  bst->root = NULL;
  return bst;
}

/* Adds a node to the bst if its data does not already exist in the tree,
   returns 0 if it didn't add a node, and 1 if it did. */
int add(Bst *bst, char *data, int word_len) {
  BSTNode **curr = &bst->root;
  int allocate = 1;
  while (*curr != NULL) {
    int shorter_word_len =
        ((*curr)->word_len < word_len) ? (*curr)->word_len : word_len;
    int result = strncmp((*curr)->data, data, shorter_word_len);
    if (result == 0) {
      /* Equal Strings */
      (*curr)->score++;
      allocate = 0;
      break;
    } else if (result > 0)
      curr = (BSTNode **)&((*curr)->left_child);
    else {
      curr = (BSTNode **)&((*curr)->right_child);
    }
  }

  if (allocate) {
    /* Allocate a new node */
    BSTNode *node = malloc(sizeof(BSTNode));
    node->data = data;
    node->score = 1;
    node->word_len = word_len;
    node->left_child = NULL;
    node->right_child = NULL;
    *curr = node;
  }
  return allocate;
}

/* Gets the score of a specified node */
int get_score(Bst *bst, char *data, int word_len) {
  BSTNode **curr = &bst->root;
  while (*curr != NULL) {
    int shorter_word_len =
        ((*curr)->word_len < word_len) ? (*curr)->word_len : word_len;
    int result = strncmp((*curr)->data, data, shorter_word_len);
    if (result == 0) {
      return (*curr)->score;
      break;
    } else if (result > 0)
      curr = (BSTNode **)&((*curr)->left_child);
    else {
      curr = (BSTNode **)&((*curr)->right_child);
    }
  }
  return 0;
}

/* Returns a pointer to the specified node, NULL if it does not exist */
BSTNode *get_node(Bst *bst, char *data, int word_len) {
  BSTNode **curr = &bst->root;
  while (*curr != NULL) {
    int shorter_word_len =
        ((*curr)->word_len < word_len) ? (*curr)->word_len : word_len;
    int result = strncmp((*curr)->data, data, shorter_word_len);
    if (result == 0) {
      return *curr;
      break;
    } else if (result > 0)
      curr = (BSTNode **)&((*curr)->left_child);
    else {
      curr = (BSTNode **)&((*curr)->right_child);
    }
  }
  return NULL;
}

/* Frees the specified node and its children */
void free_node(BSTNode *node) {
  if (node == NULL)
    return;
  free_node(node->left_child);
  free_node(node->right_child);
  free(node->data);
  free(node);
}

/* Frees the entire bst and all of its nodes */
void free_bst(Bst *bst) {
  free_node(bst->root);
  free(bst);
}
