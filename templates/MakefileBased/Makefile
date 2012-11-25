OBJS_DIR=Build.lnx
GPP=g++
GCC=gcc
FLAGS= --show-column  -g2 -Wall -O0  -D_DEBUG 
LIBS=   

all: ${OBJS_DIR} ${OBJS_DIR}/Project.bin

include Makefile.common

clean:
	rm -rf ${OBJS} ${OBJS_DIR}/Project.bin

${OBJS_DIR}/Project.bin: ${OBJS}
	${GPP} ${OBJS} ${LIBS} -o $@

${OBJS_DIR}:
	mkdir ${OBJS_DIR}

