VPATH		=	$(SRC_DIR):$(SRC_DIR)/server

SRCS		=	main.cpp \
				Server.cpp \
				ServerConfig.cpp \
				Socket.cpp \
				PollManager.cpp \


INCLUDES	=	-Iincludes \
				-Iincludes/server