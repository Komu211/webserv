VPATH		=	$(SRC_DIR):$(SRC_DIR)/server

SRCS		=	main.cpp \
				Server.cpp \
				ServerConfig.cpp \
				Socket.cpp \
				ActiveSockets.cpp


INCLUDES	=	-Iincludes \
				-Iincludes/server