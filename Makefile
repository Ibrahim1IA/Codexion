# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: iissoufo <iissoufo@student.1337.ma>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/22 12:27:23 by iissoufo          #+#    #+#              #
#    Updated: 2026/11/26 01:30:54 by iissoufo         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = codexion
CC = cc
CFLAGS = -g -Wall -Wextra -Werror -pthread -I includes

SRCS = src/main.c \
	   src/init.c \
	   src/parser.c \
	   src/heap.c \
	   src/dongle.c \
	   src/dongle2.c \
	   src/monitor.c \
	   src/coder.c \
	   src/routine.c \
	   src/utils.c \
	   src/utils2.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

src/%.o : src/%.c includes/codexion.h 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re