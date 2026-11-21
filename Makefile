NAME = codexion
CC = cc
CFLAGS = -g -Wall -Wextra -Werror -pthread -I includes

SRCS = src/main.c \
	   src/init.c \
	   src/parser.c \
	   src/heap.c \
	   src/dongle.c \
	   src/monitor.c \
	   src/coder.c \
	   src/routine.c \
	   src/utils.c \

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	echo "$(NAME) compiled successfully!"


clean:
	rm -f $(OBJS)
	echo "Object files removed."

fclean: clean
	rm -f $(NAME)
	echo "Executable file removed."

re: fclean all

.PHONY: all clean fclean re