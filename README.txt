Title: Project 2
Clark Barrus, clark.barrus@ou.edu
10/14/18
Approach to programming problem:

How to run code:
Code can be simply run with ./project1 or by passing a batchfile as an argument.
Application behaves as simple shell with the following commands:
esc - quits program
filez - Lists files in current directory
wipe - Clears the screen
environ - Lists current environment
ditto - Prints to the command line
help -Displays this README file
mimic - Copies the file given by the first argument with the new name given by\
 the second argument
erase - Deletes file pointed to by the first argument
morph - Changes the file pointed to by the first argument into a new file, or \
into a new directory
chdir - Changes directory (or lists directory contents if no arguement give)
Known bugs and assumptions:
morph was implemented using the rename function which isn't working in as many \
cases as it seems it should. It is having a hard time, especially with absolute\
 and relative directory destinations

References and Materials:
