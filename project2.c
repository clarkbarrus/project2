/*
CS 3113, Fall 2018 Project 2, Due 10/18/2018
By Clark Barrus
Instructor: Dr. Fagg, Dr. Grant

usage:

./project2

reads in a line of keyboard input at a time, parsing it into
tokens that are separated by white spaces (set by #define
SEPARATORS).

can use redirected input

if the first token is a recognized internal command, then that
command is executed. otherwise the tokens are printed on the
display.

internal commands:

wipe - clears the screen

esc - exits from the program

filez [target] - lists contents of the target directory or lists the targetted file

ditto - prints following arguements to stdout

environ - displays current environment

mimic - copies files or directories from one location
to another -r flag will make it recursive

morph - like mimic but erases old files and directories
-r flag will make it recursive

erase - erases src directory or file,
-r flag will make it recursive

*/

//Includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <dirent.h>

//Definitions
#define MAX_BUFFER 1024                        // max line buffer
#define MAX_FILENAME 257                       // max length of file path
#define MAX_ARGS 64                            // max # args
#define SEPARATORS " \t\n"                     // token sparators
#define ERASE 1
#define MIMIC 2
#define MORPH 4
#define RECUR 8

//Structures
struct filemanip_st {
  char src[MAX_FILENAME]; // This is the source file or the only file in case of an erase;
  char dst[MAX_FILENAME];
  unsigned int op; // 0 = erase; 1 = mimic; 2 = morph; 3 = recursive
};
typedef struct filemanip_st filemanip;
filemanip fileops; //Global fileops struct for use with morph/mimic operations

//Helper Funcitons
int is_directory(const char *path); // Checks for directory
int is_directory_empty(char * path); // Checks the a directory is empty
int dofileoperation(filemanip *); // perform file operations
void syserrmsg(const char * msg, const char * msg2);


//External vars
extern char **environ;                   // environment array

int main (int argc, char ** argv)
{
  char buf[MAX_BUFFER];                      // line buffer
  char * args[MAX_ARGS];                     // pointers to arg strings
  char ** arg;                               // working pointer thru args
  char * prompt = "==>" ;                    // shell prompt

  setbuf(stdout, NULL);

  //Check for batch file
  if(argc >= 2) {
    //Batchfile exists, contained in argv[1]
    if ( -1 == fileno(freopen(argv[1], "r", stdin))) {
      //Check for valid batchfile
      fprintf(stderr, "Error opening batchfile");
      exit(-1);
    } //Batchfile now set as stdin
  }

  // keep reading input until "quit" command or eof of redirected input

  while (!feof(stdin)) {
    // get command line from input

    fprintf(stdout,"%s%s", getenv("PWD"), prompt); // write prompt

    if (fgets (buf, MAX_BUFFER, stdin ))
    { // read a line
      //If stdin is not terminal, print buf
      if (isatty(fileno(stdin)) == 0)
      {
        if (ENOTTY == errno)
        fprintf(stdout, "%s", buf);
        else
        fprintf(stderr, "Error: stdin not valid");
      }

      // tokenize the input into args array
      char * bufcopy = strdup(buf); //Copy user input to potentially pass to system()
      arg = args;
      *arg++ = strtok(buf,SEPARATORS);   // tokenize input
      while ((*arg++ = strtok(NULL,SEPARATORS)));
      // last entry will be NULL

      //TODO Add IO redirection, <, >, >>

      // Check if there's anything there
      if (args[0]){
        // check for internal/external command

        //mkdirz
        if (!strcmp(args[0],"mkdirz"))  // "mkdir" command
        {
          if (args[1]) //Check that an argument was given
          {
            //rwxrwxrw-
            unsigned int dst_perms = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH | S_IXGRP; // rwxrwxrw-
            //Make the directory
            if(mkdir(args[1], dst_perms)) {
              //Error checking
              syserrmsg("mkdirz error", NULL);
              perror(NULL);
            }
          }
          else //No arguement given
          {
            syserrmsg("mkdirz takes one arguement", NULL);
          }
          continue;
        }

        //rmdirz
        if (!strcmp(args[0],"rmdirz"))  // "rmdir" command
        {
          if (args[1]) //Check that an argument was given
          {
            if(rmdir(args[1])) {
              //Error checking
              syserrmsg("rmdirz error", NULL);
              perror(NULL);
            }
          }
          else //No arguement given
          {
            syserrmsg("rmdirz takes one arguement", NULL);
          }
          continue;
        }

        //wipe
        if (!strcmp(args[0],"wipe"))  // "clear" command
        {
          fflush(stdout);
          system("clear");
          fflush(stdout);
          continue;
        }

        //esc
        if (!strcmp(args[0],"esc"))    // "quit" command
        {
          if (isatty(fileno(stdin)) == 0) //If bashfile, add a new line character
          fprintf(stdout, "\n");
          break; // break out of 'while' loop
        }

        //help
        if (!strcmp(args[0], "help")) //Prints readme file
        {
          //Open readme file
          int fd = open("/projects/1/README.txt", O_RDONLY);

          if (fd == -1) //Check for error
          {
            fprintf(stderr, "%s", "Error: Opening README.txt\n");
            exit(-1);
          }

          //Print help file to stdout
          char readmebuf[MAX_BUFFER];
          int readret = 0;
          while (readret = read(fd, readmebuf, MAX_BUFFER - 1))
          {
            if(readret == -1) //Check for error
            {
              fprintf(stderr, "Error: Reading README.txt\n");
            }
            readmebuf[readret] = '\0'; //Set last char in cstring to be null
            fprintf(stdout, "%s", readmebuf); //Should print properly
          }
          //Close help file
          if (close(fd) == -1) //Check for error
          {
            fprintf(stderr, "%s", "Error: Closing README.txt\n");
            exit(-1);
          }
          continue;
        }

        //mimic
        if (!strcmp(args[0],"mimic")) {
          if (!args[1] || !args[2]) {
            syserrmsg("error mimic takes two or three parameters", NULL);
          }
          else {
            fileops.op =  0;
            if (args[1] == "-r") {
              strcpy(fileops.src, args[2]);
              strcpy(fileops.dst, args[3]);
              fileops.op = MIMIC | RECUR;
              dofileoperation(&fileops);
            }
            else { //If incorrect parameters are passed, dofileops will fail
              strcpy(fileops.src, args[1]);
              strcpy(fileops.dst, args[2]);
              fileops.op = MIMIC;
              dofileoperation(&fileops);
            }
          }
          continue;
        }

        //morph
        if (!strcmp(args[0],"morph")) {
          if (!args[1] || !args[2]) {
            syserrmsg("error morph takes two or three parameters", NULL);
          }
          else {
            fileops.op = 0;
            if (args[1] == "-r") {
              strcpy(fileops.src, args[2]);
              strcpy(fileops.dst, args[3]);
              fileops.op = MORPH | RECUR;
              dofileoperation(&fileops);
            }
            else { //If incorrect parameters are passed, dofileops will fail
              strcpy(fileops.src, args[1]);
              strcpy(fileops.dst, args[2]);
              fileops.op = MORPH | RECUR;
              dofileoperation(&fileops);
            }
          }
          continue;
        }

        //erase
        if (!strcmp(args[0], "erase")) //"rm" command
        {
          if (!args[1]) {
            syserrmsg("error erase takes one or two parameters", NULL);
          }
          else {
            if (args[1] == "-r") {
              strcpy(fileops.src, args[2]);
              fileops.op = ERASE | RECUR;
              dofileoperation(&fileops);
            }
            else { //If incorrect parameters are passed, dofileops will fail
              strcpy(fileops.src, args[1]);
              fileops.op = ERASE;
              dofileoperation(&fileops);
            }
          }
          continue;
        }

        //filez
        if (!strcmp(args[0],"filez")) // "ls" command
        {
          //Construct command to pass to system
          if (args[1])
          {
            char buffer[MAX_BUFFER];
            snprintf(buffer, MAX_BUFFER, "%s%s", "ls -1 ", args[1]);
            system(buffer);
          }
          else
          {
            fflush(stdout);
            system("ls -1");
            fflush(stdout);
          }
          continue;
        }

        //ditto
        if (!strcmp(args[0],"ditto")) { // comment command
          arg = args;
          arg++;
          if (*arg) fprintf(stdout,"%s",*arg++);
          while (*arg) fprintf(stdout," %s",*arg++);
          fprintf(stdout,"\n");
          continue;
        }

        //chdir
        if (!strcmp(args[0],"chdir")) { // "cd" command
        if (!args[1]) {
          fprintf(stdout,"%s\n", getenv("PWD"));
        } else if (!chdir(args[1])) {
          char buffer1[MAX_BUFFER];
          getcwd(args[1], MAX_BUFFER);
          snprintf(buffer1, MAX_BUFFER, "%s%s", "PWD=", args[1]);
          putenv(buffer1);
        } else {
          fprintf(stderr, "Invalid path for chdir\n");
        }
        continue;
      }

      //environ
      //Code for this case provided by instructor on website
      if (!strcmp(args[0],"environ")) // "environment" command

      {
        char **env = environ;

        while (*env) printf("%s\n",*env++);  // step through environment
        continue;
      }

      // else pass command onto OS
      fflush(stdout);
      system(bufcopy);
      fflush(stdout);


    }
    free(bufcopy); //Free buf copy after potentially passing it to system
  }
  }
  return 0;
}

//Executes file operation, morph, mimic, remove etc, based on files in fileops
//and based on fileops->op
int dofileoperation(filemanip *fileops ) {

  if ( ((fileops->op & MORPH) == MORPH) ||
      ((fileops->op & MIMIC) == MIMIC)) {

    unsigned int src_flags = O_RDONLY;
    unsigned int dst_flags = O_CREAT | O_WRONLY | O_TRUNC;
    unsigned int dst_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // rw-rw-rw-

    // Check to make sure that the src and dest are correct
    int is_src_dir = is_directory(fileops->src);

    int is_dst_dir = is_directory(fileops->dst);


    // If the dst is a valid directory,
    // give the dst the file name of the src
    if (is_dst_dir) {
      syserrmsg("Fileops: entered dst is dir branch", NULL);
      char dst_name[MAX_FILENAME/2];
      char dst_path[MAX_FILENAME/2];
      char dst_dir[MAX_FILENAME/2];

      // Get the name from src
      strcpy(dst_name,basename(fileops->src));

      // The last folder name
      strcpy(dst_dir, basename(fileops->dst));

      // Get the path from dst
      strcpy(dst_path, dirname(fileops->dst));

      // concatenate the new file together
      fileops->dst[0] = '\0'; // zero out the string
      strcat(fileops->dst, dst_path);
      strcat(fileops->dst, "/");
      strcat(fileops->dst, dst_dir);
      strcat(fileops->dst, "/");
      strcat(fileops->dst, dst_name);
    }


    //Src is a directory
    //Create new directory if empty, or recursively copy contents
    if (is_src_dir) {
      syserrmsg("Fileops: Entered src is dir branch", NULL);

      //If directory is empty make a new directory in dst
      int n = is_directory_empty(fileops->src);
      if (n == -1) {
        syserrmsg("is_directory_empty failed", NULL);
        return EXIT_FAILURE;
      }
      if (n == 1) {
        syserrmsg("src dir is empty", NULL);
        //Since directory is empty, create new directory at dst
        if(mkdir(fileops->dst, dst_perms | S_IXUSR | S_IXGRP)) { //Add permissions to directory
          syserrmsg("dir create error", NULL);
          perror(NULL);
          return EXIT_FAILURE;
        }
        syserrmsg("Succcessfully called mkdir in dst", NULL);
      }
      else if (n == 0) {
        //Directory is not Empty
        if(fileops->op & RECUR) { //Copy contents recursively
          syserrmsg("Fileops: entered recursive branch", NULL);

          //Create new directory
          if(mkdir(fileops->dst, dst_perms | S_IXUSR | S_IXGRP)) {
            syserrmsg("dir create error", NULL);
            perror(NULL);
            return EXIT_FAILURE;
          }

          //Open dir
          DIR * src_dirp = opendir(fileops->src);

          struct dirent * entry = readdir(src_dirp);
          while(entry != NULL) { //Loop for all files in directory
            //Read dirent. If . or .. ignore and move on.
            if (strcmp(entry->d_name, ".") || strcmp(entry->d_name, "..")) {
              //Don't do anything for . and .. files
            }
            else {
              //Set up and call dofileoperation
              filemanip newfilemanip;
              newfilemanip.op = fileops->op;

              //newfilemanip.src = fileops->src + dirent->d_name
              strcpy(newfilemanip.src, fileops->src);
              strcat(newfilemanip.src, "/");
              strcat(newfilemanip.src, entry->d_name);

              //Destination is newly created directory at dst
              strcpy(newfilemanip.dst, fileops->dst);

              syserrmsg("Calling dofileoperation with args:", NULL);
              syserrmsg(newfilemanip.dst, newfilemanip.src);
              //Call dofileoperation
              dofileoperation(&newfilemanip);
            }

            //Prep for next file entry
            entry = readdir(src_dirp);
          }
          closedir(src_dirp);
        }
        else { //Fail, since not recursive
          syserrmsg("src is not an empty directory, recursion not active", NULL);
          return EXIT_FAILURE;
        }

      }
    } //End of src is dir case
    //Src is a file
    else {
      //syserrmsg("Fileops: entered src is file branch", NULL);

      //Open src for reading
      int src_fd = open(fileops->src, src_flags);
      if (src_fd == -1) {
        if (fileops->op & MIMIC) {
          syserrmsg("mimic [src]", NULL);
          return EXIT_FAILURE;
        }
        else {
          syserrmsg("morph [src]", NULL);
          return EXIT_FAILURE;
        }
      }
      //Open dst_fd for writing
      int dst_fd = open(fileops->dst, dst_flags, dst_perms);
      //syserrmsg("Successfully called open() in dst", NULL);

      if (dst_fd == -1) {
        if (fileops->op & MIMIC) {
          syserrmsg("mimic [dst]", NULL);
          return EXIT_FAILURE;
        }
        else {
          syserrmsg("morph [dst]", NULL);
          return EXIT_FAILURE;
        }
      }

      //Copy from src file to dst file
      ssize_t num_read;
      char buf[MAX_BUFFER];
      while ((num_read = read(src_fd, buf, MAX_BUFFER)) > 0) {
        if (write(dst_fd, buf, num_read) != num_read) {
          if (fileops->op & MIMIC) {
            syserrmsg("mimic write error", NULL);
            return EXIT_FAILURE;
          }
          else {
            syserrmsg("morph write error", NULL);
            return EXIT_FAILURE;
          }
        }
      }
      if (num_read == -1) {
        if (fileops->op & MIMIC) {
          syserrmsg("mimic error reading", NULL);
          return EXIT_FAILURE;
        }
        else { syserrmsg("morph error reading", NULL);
          return EXIT_FAILURE;
        }
      }

      //close dst file descriptor
      if (close(dst_fd) == -1) {
        if (fileops->op & MIMIC) {
          syserrmsg("mimic: Error closing [dst] file", NULL);
          return EXIT_FAILURE;
        }
        else {
          syserrmsg("mimic: Error closing [dst] file", NULL);
          return EXIT_FAILURE;
        }
      }

      //Close src file descriptors
      if (close(src_fd) == -1) {
        if (fileops->op & MIMIC) {
          syserrmsg("mimic: Error closing [src] file", NULL);
          return EXIT_FAILURE;
        }
        else {
          syserrmsg("morph: Error closing [src] file", NULL);
          return EXIT_FAILURE;
        }
      }
    } //End of src is file case

  } //End if case that executes for morph, mimic flags

  // Check to see if the file needs to be removed
  // This happenes in the morph and erase case
  if ( ((fileops->op & MORPH) == MORPH) ||
      ((fileops->op & ERASE) == ERASE)) {

    // Removes file or directory
    // https://linux.die.net/man/3/remove
    if (remove(fileops->src)) {
      if ((fileops->op & MORPH) == MORPH) {
        syserrmsg("morph remove error", NULL);
        return EXIT_FAILURE;
      }
      else {
        syserrmsg("erase remove error", NULL);
        return EXIT_FAILURE;
      }
    }
  }

  return EXIT_SUCCESS;
}

// Check if a file is a directory
int is_directory(const char *path) {
  syserrmsg("is_directory: is the following is a directory?", NULL);
  struct stat statbuf;
  if (stat(path, &statbuf) != 0)
    return 0; //Failed, can check errno

  return S_ISDIR(statbuf.st_mode); //Returns true/false for isdir
}

//Check if directory is empty
//Based on https://stackoverflow.com/questions/6383584/check-if-a-directory-is-empty-using-c-on-linux
int is_directory_empty(char * path) {
  int n = 0;
  struct dirent *d;
  DIR *dir = opendir(path);
  if (dir == NULL) { //Not a directory or doesn't exist
    syserrmsg("is_directory_empty opendir() error", NULL);
    return -1;
  }

  while ((d = readdir(dir)) != NULL) {
    if(++n > 2)
      break;
  }
  closedir(dir);
  if (n <= 2) {//Directory Empty, keeping in mind . and ..
    syserrmsg("is_directory_empty returned true", NULL);
    return 1; //True
  }
  else {
    syserrmsg("is_directory_empty returned false", NULL);
    return 0; //False
  }
}

// Error message helper method
void syserrmsg(const char * msg, const char * msg2) {
  fprintf(stderr, "%s\n", msg);
  if (msg2)
    fprintf(stderr, "%s\n", msg2);
}
