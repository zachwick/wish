/**
 * (W)ick (I)nteractive (Sh)el
 *
 * Copyright 2016 zach wick <zach@zachwick.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <libguile.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define WISH_RL_BUFSIZE 1024
#define WISH_TOK_BUFSIZE 64
#define WISH_TOK_DELIM " \t\r\n\a"

// Forward definitions
int wish_cd(char **args);
int wish_exit(char **args);
int wish_launch(char **args);
int wish_execute(char **args);
char **wish_splitline(char *line);
void wish_loop(void);
char* wish_prompt();

// Default vars that can be overridden
char* wshome = "/home/";
SCM help_func;

// State holding variables
char *currentdir = "";

// Show a shell prompt of the current directory's path followed by '>'
char*
wish_prompt()
{
  currentdir = "";
  char* prompt;
  prompt = strcat(getcwd(NULL, 0), " > ");
  return prompt;
}

// List of built-in commands
char *builtin_str[] = {
  "cd",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &wish_cd,
  &wish_exit
};

int
wish_num_builtins()
{
  return sizeof(builtin_str) / sizeof(char *);
}

// An implementation of 'cd', which changes the user's currently active
// directory to the given path
int
wish_cd(char **args)
{
  if (args[1] == NULL)
    {
      // if no path is given, ie. just 'cd' is entered, then change to the
      // path given as the 'wshome' variable in the loaded 'wishrc.scm' file
      if (chdir(wshome) != 0)
	{
	  perror("wish");
	}
    }
  else
    {
      if (chdir(args[1]) != 0)
	{
	  perror("wish");
	}
    }
  return 1;
}

int
wish_exit(char **args)
{
  return 0;
}

int
wish_launch(char **args)
{
  pid_t pid;
  pid_t wpid;
  int status;

  pid = fork();

  if (pid == 0)
    {
      // Child process
      if (execvp(args[0], args) == -1)
	{
	  perror("wish");
	}
      exit(EXIT_FAILURE);
    }
  else if (pid < 0)
    {
      // Forking error
      perror("wish");
    }
  else
    {
      // Parent process
      do {
	wpid = waitpid(pid, &status, WUNTRACED);
	if (wpid == -1)
	  {
	    fprintf(stderr, "wish: forking error\n");
	    exit(EXIT_FAILURE);
	  }
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

  return 1;
}

int
wish_execute(char **args)
{
  int i;

  if (args[0] == NULL)
    {
      // An empty command was entered
      return 1;
    }

	if (strcmp(args[0],"help") == 0)
		{
			scm_call_0(help_func);
			return 1;
		}
  for (i = 0; i < wish_num_builtins(); i++)
    {
      if (strcmp(args[0], builtin_str[i]) == 0)
				{
					return (*builtin_func[i])(args);
				}
    }

  return wish_launch(args);
}

char
**wish_splitline(char *line)
{
  int bufsize = WISH_TOK_BUFSIZE;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens)
    {
      fprintf(stderr, "wish: token allocation error\n");
      exit(EXIT_FAILURE);
    }

  token = strtok(line, WISH_TOK_DELIM);

  while (token != NULL)
    {
      tokens[position] = token;
      position++;

      if (position >= bufsize)
	{
	  bufsize += WISH_TOK_BUFSIZE;
	  tokens = realloc(tokens, bufsize * sizeof(char*));
	  if (!tokens)
	    {
	      fprintf(stderr, "wish: token reallocation error\n");
	      exit(EXIT_FAILURE);
	    }
	}

      token = strtok(NULL, WISH_TOK_DELIM);
    }

  tokens[position] = NULL;
  return tokens;
}

void
wish_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    // Print out a prompt
    line = readline(wish_prompt());

    // Add input line to history
    add_history(line);

    // Split the line of text input
    args = wish_splitline(line);

    // Execute the split line of text input
    status = wish_execute(args);

    free(line);
    free(args);
  } while (status);
}

int
main(int argc, char **argv)
{
  // Initialize Guile
  scm_init_guile();
  // Load configuration script files
  SCM init_func;
  SCM wshome_scm;

  scm_c_primitive_load("wishrc.scm");

  init_func = scm_variable_ref(scm_c_lookup("wish_config"));

  scm_call_0(init_func);

	help_func = scm_variable_ref(scm_c_lookup("wish_help"));

  wshome_scm = scm_variable_ref(scm_c_lookup("wshome"));
  wshome =  scm_to_locale_string (wshome_scm);

  // Set currentdir
  currentdir = getcwd(NULL, 0);

  // Run the wish command loop
  wish_loop();
  // Run any shutdown/cleanup scripts

  // Free anything that need freed
  free(wshome);

  return EXIT_SUCCESS;
}
