#Used to switch the words in the irregular nouns list.
#This way the plural word comes first.

read_file = open("../irregnouns.txt", "r")
for line in read_file:
    split_line = line.split();
    split_line[1].replace("\n", "")
    print split_line[1] + " " + split_line[0]