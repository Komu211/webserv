include srcs.mk


NAME			=	webserv

CXX				=	c++
CXXFLAGS		=	-std=c++17 -Wall -Wextra -Werror -MMD -MP
DEBUG_FLAGS		=	-g -fsanitize=address
RM				=	rm -f
DEPENDS			=	$(OBJS:.o=.d)

SRC_DIR			=	src
OBJ_DIR			=	obj

OBJS 			= $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRCS))

.DEFAULT_GOAL	= all

all: $(NAME)

-include $(DEPENDS)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@echo "Compiling $(NAME) project"

debug: $(OBJS)
	@$(CXX) $(DEBUG_FLAGS) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@echo "Compiling $(NAME) project with debug flags"


$(OBJ_DIR)/%.o: %.cpp Makefile | $(OBJ_DIR)
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

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
