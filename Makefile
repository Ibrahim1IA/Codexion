NAME = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -I includes

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

GREEN       = \033[0;32m
RED         = \033[0;31m
RESET       = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	echo "$(GREEN)$(NAME) compiled successfully!$(RESET)"


clean:
	rm -f $(OBJS)
	echo "$(RED)Object files removed.$(RESET)"