#
# Makefile for neplan-lang
#

CFLAGS=-g
BUILD_DIR=build/
APP_NAME=neplan

dist: clean build
	@echo 'Making target $@ ...'

build: src/main.l
	mkdir -p ${BUILD_DIR}
	flex -o ${BUILD_DIR}$@.c $<
	${CC} ${CFLAGS} -o ${BUILD_DIR}${APP_NAME} ${BUILD_DIR}$@.c

clean:
	- rm -r ${BUILD_DIR}
