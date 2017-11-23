read_file = open("../commonsynonyms.txt", "r");
for line in read_file:
    split_line = line.split("---", 1)
    for word in split_line[1].split(","):
        word = word.replace("\n", "")
        print word + " " + split_line[0]