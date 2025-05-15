include srcs.mk


NAME		=	webserv

CC			=	c++
CFLAGS		=	-Wall -Wextra -Werror -std=c++17
DEBUG_FLAGS	=	-g -fsanitize=address
RM			=	rm -f
INCLUDES	=	-Iincludes

SRC_DIR		=	src
OBJ_DIR		=	obj

OBJS 		= $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRCS))

.DEFAULT_GOAL = all


all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@echo "Compiling $(NAME) project"

debug: $(OBJS)
	@$(CC) $(DEBUG_FLAGS) $(CFLAGS) -o $(NAME) $(OBJS)
	@echo "Compiling $(NAME) project with debug flags"


$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@echo "Creating folder for object files"


clean:
	@echo "Deleting $(NAME) objects"
	@rm -rf $(OBJ_DIR)


fclean: clean
	@$(RM) $(NAME)
	@echo "Deleting $(NAME) executable"

re: fclean all

.PHONY: all clean fclean re