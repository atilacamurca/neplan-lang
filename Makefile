#
# Makefile for neplan-lang
#

CFLAGS = -g
BUILD_DIR = build/
FIXTURES_DIR = fixtures/
APP_NAME = neplan
SOURCE_FILES = src/debug.h src/debug.c src/main.h src/opcodes.h src/main_helper.c

dist: clean build
	@echo 'Making target $@ ...'

fixtures: build
	@echo 'Testing fixtures ...'
	./${BUILD_DIR}${APP_NAME} -d ${FIXTURES_DIR}hello-neplan-01.n

build: src/main.l src/main.y src/main.h src/main_helper.c
	mkdir -p ${BUILD_DIR}
	bison -d src/main.y -o ${BUILD_DIR}main.tab.c
	flex -o ${BUILD_DIR}main.lex.c src/main.l
	cp ${SOURCE_FILES} ${BUILD_DIR}
	${CC} ${CFLAGS} -o ${BUILD_DIR}${APP_NAME} ${BUILD_DIR}main.tab.c \
		${BUILD_DIR}main.lex.c ${BUILD_DIR}main_helper.c ${BUILD_DIR}debug.c -lm

clean:
	- rm -r ${BUILD_DIR}
