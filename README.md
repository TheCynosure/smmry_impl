# Smmry Impl

A final project for one of my seminars. I saw this algorithm (or similar) used on Reddit with the TL;DR bots. I wanted to implement it, I do not claim any rights to the algorithm. It is clearly listed here on the [Smmry.com website](http://smmry.com/about
).

### Implementation

The algorithm is slightly more technical when actually put into code, so here is how I implemented it.

1. Load the text into an in memory buffer and do some initial cleanup.
2. Chop the entire thing into senteces making sure that each period does not come after a abbreviation.
3. Load the sentences into a linked list with their lengths and positions in text.
4. Go through the sentences strip out plurals replacing them with their singular definitions.
- If a word is not in the most common irregular plurals and doesn't end in s then move on.
- Otherwise, use the common [plural rules](http://www.ef.edu/english-resources/english-grammar/singular-and-plural-nouns/).
5. Go through the sentences and replace words with their synonyms.
6. Create a hashmap and go through each sentence, hash every word and store the number of occurences.
7. For each sentence in our linked list, assign it a score that is the sum of the frequencies for all its words.
8. Return the top number of specified sentences.

### How to run

```
cd smmry_impl
cmake .
make
./summ <file_to_summarize> <number_of_lines>
```

### Example

For an example we are going to summarize this [Wikipedia article](https://en.wikipedia.org/wiki/Wood_stork) (the abstract section at the top).

1. First copy the abstract into a file using your favorite text editor. I am going to copy it into the file `example.txt`.
2. Run the summarize algorithm:
```
./summ example.txt 3
```
3. The summary will be printed to the console and will be the 3 most important sentences.

> Predators of the wood stork include raccoons (which predate on chicks), northern crested caracaras, which prey on eggs, and other birds of prey, which feed on eggs and chicks. During the breeding season, which is initiated when the water levels decline and can occur anytime between November and August, a single clutch of three to five eggs is laid. They fledge 60 to 65 days after hatching, although only about 31% of nests fledge a chick in any given year, with most chicks dying during their first two weeks, despite being watched by an adult during that time.


### Notice

Again, this is for an educational project and this is not my algorithm. I simply implemented it and keep the reference around because it is an elegant algo.
