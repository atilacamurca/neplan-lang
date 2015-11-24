#
# Makefile for neplan-lang
#

CFLAGS = -g
BUILD_DIR = build/
APP_NAME = neplan
SOURCE_FILES = src/debug.h src/debug.c

dist: clean build
	@echo 'Making target $@ ...'

build: src/main.l
	mkdir -p ${BUILD_DIR}
	flex -o ${BUILD_DIR}$@.c $<
	cp ${SOURCE_FILES} ${BUILD_DIR}
	${CC} ${CFLAGS} -o ${BUILD_DIR}${APP_NAME} ${BUILD_DIR}$@.c ${BUILD_DIR}debug.c

clean:
	- rm -r ${BUILD_DIR}
