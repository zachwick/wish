/**
 * (W)ick (I)nteractive (Sh)ell
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

#define WISH_RL_BUFSIZE 1024
#define WISH_TOK_BUFSIZE 64
#define WISH_TOK_DELIM " \t\r\n\a"

// Forward definitions
int wish_cd(char **args);
int wish_help(char **args);
int wish_exit(char **args);
int wish_launch(char **args);
int wish_execute(char **args);
char **wish_splitline(char *line);
void wish_loop(void);

// List of built-in commands
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &wish_cd,
  &wish_help,
  &wish_exit
};

int
wish_num_builtins()
{
  return sizeof(builtin_str) / sizeof(char *);
}

int
wish_cd(char **args)
{
  if (args[1] == NULL)
    {
      fprintf(stderr, "wish: \"cd\" expects an argument\n");
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
wish_help(char **args)
{
  int i;
  printf("(w)ick's (i)nteractive (sh)ell\n");
  printf("A minimalist shell for launching programs\n");
  printf("The following are built into wish:\n");

  for (i = 0; i < wish_num_builtins(); i++)
    {
      printf("  %s\n", builtin_str[i]);
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
    line = readline("wish> ");

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
  // Load configuration script files

  // Run the wish command loop
  wish_loop();
  // Run any shutdown/cleanup scripts

  return EXIT_SUCCESS;
}
