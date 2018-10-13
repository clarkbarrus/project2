/*
 CS 3113, Fall 2018 Project 2, Due 10/18/2018
 By Clark Barrus
 Instructor: Dr. Fagg, Dr. Grant

 strtokeg - skeleton shell using strtok to parse command line

 usage:

 ./a.out

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

//test

 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_BUFFER 1024                        // max line buffer
#define MAX_ARGS 64                            // max # args
#define SEPARATORS " \t\n"                     // token sparators

extern char **environ;                   // environment array

int main (int argc, char ** argv)
{
    char buf[MAX_BUFFER];                      // line buffer
    char * args[MAX_ARGS];                     // pointers to arg strings
    char ** arg;                               // working pointer thru args
    char * prompt = "==>" ;                    // shell prompt

    setbuf(stdout, NULL);

    //Check for batch file
    if(argc >= 2)
      {
	//Batchfile exists, contained in argv[1]
	if ( -1 == fileno(freopen(argv[1], "r", stdin)))
	  {
	    fprintf(stderr, "Error opening batchfile");
	    exit(-1);
	  } //Sets batchfile as stdin
      }

    // keep reading input until "quit" command or eof of redirected input

    while (!feof(stdin)) {

        // get command line from input



        fputs (prompt, stdout);                // write prompt

        if (fgets (buf, MAX_BUFFER, stdin )) { // read a line

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

            if (args[0]) {                     // if there's anything there

                // check for internal/external command

                if (!strcmp(args[0],"wipe")) { // "clear" command
		  fflush(stdout);
		  system("clear");
		  fflush(stdout);
		  continue;
                }

                if (!strcmp(args[0],"esc"))   // "quit" command
		  {
		  if (isatty(fileno(stdin)) == 0) //If bashfile, add a new line character
		    fprintf(stdout, "\n");

		  break;                     // break out of 'while' loop

		  }

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

		if (!strcmp(args[0],"mimic")) // "copy" command
		  {
		    FILE *  fd1;//File descriptors for files
		    FILE *  fd2;

		    if (args[1] && args[2]) //Check that two args are given
		      {
			//Open source file
			if((fd1 = fopen(args[1], "r")) == NULL)
			  {
			    fprintf(stderr, "Error: opening source file.\n");
			    continue;
			  }

			//Open destination file, overwriting if necessary
			if((fd2 = fopen(args[2], "w")) == NULL)
			  {
			    fprintf(stderr, "Error: opening/creating destination file");
			    continue;
			  }

			//Read from source file and write into destination file
			//http://www.cplusplus.com/reference/cstdio/fread/
			long lSize;
			char * buffer5;
			size_t result;


			// obtain file size:
			fseek (fd1 , 0 , SEEK_END);
			lSize = ftell (fd1);
			rewind (fd1);

			// allocate memory to contain the whole file:
			buffer5 = (char*) malloc (sizeof(char)*lSize);
			if (buffer5 == NULL) {fputs ("Memory error",stderr); exit (2);}

			// copy the file into the buffer:
			result = fread (buffer5,1,lSize,fd1);

			//Error checking
			if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

			/* the whole file is now loaded in the memory buffer. */
			//Write buffer to fd2
			result = fwrite(buffer5, sizeof(char), lSize, fd2);

			//Error checking
			if (result != lSize) {fputs ("Writing error",stderr); exit (3);}

			// Close both files

			if (fclose(fd1) == -1) //Check for error
			  {
			    fprintf(stderr, "%s", "Error: Closing README.txt\n");
			    exit(-1);
			  }
			if (fclose(fd2) == -1) //Check for error
			  {
			    fprintf(stderr, "%s", "Error: Closing README.txt\n");
			    exit(-1);
			  }


			free (buffer5);
		      }
		    else
		      {
			fprintf(stderr, "Error: mimic must have two arguments\n");
		      }

		    continue;
		  }


		if (!strcmp(args[0],"filez")) // "ls" command
		  {
		    //Construct command to pass to system
		    if (args[1])
		      {
			char buffer[MAX_BUFFER];
			snprintf(buffer, MAX_BUFFER, "%s%s", "ls -1 ", args[1]);
			system(buffer);
		      }
		    else {
		      fflush(stdout);
		      system("ls -1");
		      fflush(stdout);
		    }

		    continue;
		  }

		if (!strcmp(args[0],"ditto")) { // comment command
		  arg = args;
		  arg++;
		  if (*arg) fprintf(stdout,"%s",*arg++);
		  while (*arg) fprintf(stdout," %s",*arg++);
		  fprintf(stdout,"\n");
		  continue;
		}

		if (!strcmp(args[0],"morph")) { // "rename" command
		    if(args[1] && args[2]) //Make sure morph has two arguements
		    {
		      FILE *  fd1; //File descriptors for files
		      FILE *  fd2;
		      char * arg1copy = strdup(args[1]);

                        //Open source file
                        if((fd1 = fopen(args[1], "r")) == NULL)
                          {
                            fprintf(stderr, "Error: opening source file.\n");
                            continue;
                          }

                        //Open destination file, overwriting if necessary
			if((fd2 = fopen(args[2], "w")) == NULL)
                          {

			    char * filename[2];
			    filename[0] = strtok(args[1],"/");   // tokenize input
			    while (NULL != (filename[1] = strtok(NULL,"/")))
			      {
				filename[0] = filename[1];
			      }

			    //Filename[0] contains relative filename form args[1]
			    //Append filename form args[1] to possible directors in args[2]
			    char buffe[MAX_BUFFER];
			    snprintf(buffe, MAX_BUFFER, "%s/%s", args[2], filename[0]);
			    //buffe now contains the new potential absolute/relative filename

			    if((fd2 = fopen(buffe, "w")) == NULL) //If args[2] is invalid directory this will fail
			      {
				fprintf(stderr, "Error: opening/creating destination file.");
				continue;
			      }
			    //fd2 should now contain file pointer to new file
                          }

                        //Read from source file and write into destination file
                        //http://www.cplusplus.com/reference/cstdio/fread/
                        long lsize;
                        char * buffer8;
                        size_t res;

			// obtain file size:
                        fseek (fd1 , 0 , SEEK_END);
                        lsize = ftell (fd1);
                        rewind (fd1);

                        // allocate memory to contain the whole file:
                        buffer8 = (char*) malloc (sizeof(char)*lsize);
                        if (buffer8 == NULL) {fputs ("Memory error",stderr); exit (2);}

                        // copy the file into the buffer:
                        res = fread (buffer8,1,lsize,fd1);

                        //Error checking
                        if (res != lsize) {fputs ("Reading error",stderr); exit (3);}

                        /* the whole file is now loaded in the memory buffer. */
                        //Write buffer to fd2
                        res = fwrite(buffer8, sizeof(char), lsize, fd2);

                        //Error checking
                        if (res != lsize) {fputs ("Writing error",stderr); exit (3);}

                        // Close both files

                        if (fclose(fd1) == -1) //Check for error
                          {
                            fprintf(stderr, "%s", "Error: Closing README.txt\n");
                            exit(-1);
                          }
                        if (fclose(fd2) == -1) //Check for error
                          {
                            fprintf(stderr, "%s", "Error: Closing README.txt\n");
                            exit(-1);
                          }

			if(remove(arg1copy))
                          fprintf(stderr, "Error: erase unsuccessful\n");

                        free (buffer8);
			free (arg1copy);



		    }
		  else
		    {
		      fprintf(stderr, "Please provide two aruguements to morph.\n");
		    }
		    continue;
		}

		if (!strcmp(args[0],"chdir")) { // "cd" command

		  if (!args[1])
		    {
		      fprintf(stdout,"%s\n", getenv("PWD"));
		    } else if (!chdir(args[1])) {
		      char buffer1[MAX_BUFFER];
		      getcwd(args[1], MAX_BUFFER);
		      snprintf(buffer1, MAX_BUFFER, "%s%s", "PWD=", args[1]);
		      putenv(buffer1);
		    } else
		      {
			fprintf(stderr, "Invalid path for chdir\n");
		      }
		  continue;
		}

		if (!strcmp(args[0], "erase")) //"rm" command
		  {
		    if (args[1])
		      {
			if(remove(args[1]))
			  fprintf(stderr, "Error: erase unsuccessful\n");
		      }
		    else
		      fprintf(stderr, "Error: erase must have an arguement.\n");

		    continue;
		  }

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
