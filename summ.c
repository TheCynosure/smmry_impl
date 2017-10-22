#include <stdio.h>

// ---
// Implementation of a Linked List to be used later.
// ---
typedef struct {
	
} Node;

typedef struct {
	
} LList;

// ---
// Implementation of a Hash Map to be used later.
// ---

typedef struct {
	
} HMap;

// ---
// End of Implementation
// ---


//Initially called to strip out newlines, tabs, extra spaces.
void cleanup(char* text_buff);
//Returns a list with pointers to the start of all sentences and their lengths.
LList sentence_chop();
//Replaces the plurals in all sentences.
void replace_plurals(LList sentence_list);
//Replace the synonyms in all sentences.
void replace_synonyms(LList sentence_list);
//Get the occurences of all words in the list.
HMap get_occurences(LList sentence_list);
//Get the scores of the words.
LList get_scores(HMap word_occurences, LList sentence_list);
//Return the top number of sentences.
char* concat_top_sentences(int num_of_top);

int main(int argc, char** argv) {
	
}

