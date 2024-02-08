
CFLAGS=-std=c17 -Wall -Wextra -Werror -lm

all:
	gcc minishell.c -o minishell $(CFLAGS)