CXX			= clang++

CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -pedantic -fsanitize=address -g

SRCS_PATH	= srcs

SRCS		= Internet_socket.cpp Server.cpp utils.cpp Rules.cpp webserv.cpp parsing.cpp Virtual_server.cpp post_requests.cpp Request.cpp \
				Location.cpp get_requests.cpp

SRCS_NAME	= $(addprefix $(SRCS_PATH)/, $(SRCS))

OBJS		= $(SRCS_NAME:.cpp=.o)

NAME		= webserv

RM			= rm -rf



all		:	$(NAME)

$(NAME)	:	$(OBJS)
				$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
				rm -rf 

run		:	$(NAME)
				./$(NAME)

clean	:	
				$(RM) $(OBJS)

fclean	:	clean
				$(RM) $(NAME)

re		:	fclean all

.PHONY	:	all fclean re
