An implementation of the Summary Algorithm spelled out here:

http://smmry.com/about

The way the algorithm works is spelled out here in more detail.

1. Load the text into an in memory buffer and do some initial cleanup.
2. Chop the entire thing into senteces making sure that each period does not come after a abbreviation.
3. Load the sentences into a linked list with their lengths and positions in text.
4. Go through the sentences strip out plurals replacing them with their singular definitions.
- If a word is not in the most common irregular plurals and doesn't end in s then move on.
- Otherwise, use the common [plural rules](http://www.ef.edu/english-resources/english-grammar/singular-and-plural-nouns/).
5. Go through the setences and replace words with their synonyms.
6. Create a hashmap and go through each sentence, hash every word and store the number of occurences.
7. For each sentence in our linked list, assign it a score that is the sum of the NOO (step 2) for all its words.
8. Return the top number of specified sentences.

 Obviously this algorithm is heavily based on the one listed at http://smmry.com/about.
 I do not claim any rights to any of it, this implementation is purely for an educational project.
