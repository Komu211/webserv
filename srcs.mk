VPATH		=	$(SRC_DIR):$(SRC_DIR)/server:$(SRC_DIR)/config:$(SRC_DIR)/utils

SRCS		=	main.cpp \
				Server.cpp \
				GlobalConfig.cpp \
				ServerConfig.cpp \
				LocationConfig.cpp \
				Socket.cpp \
				PollManager.cpp \
				utils.cpp


INCLUDES	=	-Iincludes \
				-Iincludes/server \
				-Iincludes/config \
				-Iincludes/utils 