#
# Makefile for neplan-lang
#

CFLAGS = -g
BUILD_DIR = build/
FIXTURES_DIR = fixtures/
APP_NAME = neplan
SOURCE_FILES = src/debug.h src/debug.c src/main.h src/opcodes.h src/main_helper.c

run: dist
	./build/neplan

dist: clean build
	@echo 'Making target $@ ...'

fixtures: dist
	@echo 'Testing fixtures ...'
	./${BUILD_DIR}${APP_NAME} -d ${FIXTURES_DIR}hello-neplan-01.n
	./${BUILD_DIR}${APP_NAME} -d ${FIXTURES_DIR}hello-neplan-02.n
	./${BUILD_DIR}${APP_NAME} -d ${FIXTURES_DIR}hello-neplan-03.n
	./${BUILD_DIR}${APP_NAME} -d ${FIXTURES_DIR}hello-neplan-04.n
	./${BUILD_DIR}${APP_NAME} -d ${FIXTURES_DIR}hello-neplan-05.n

${BUILD_DIR}main.tab.c: src/parser.y
	bison -v -d $< -o $@

${BUILD_DIR}main.lex.c: src/lexer.l
	flex -o $@ $<

build: init compile

init:
	mkdir -p ${BUILD_DIR}
	cp ${SOURCE_FILES} ${BUILD_DIR}

compile: ${BUILD_DIR}main.tab.c ${BUILD_DIR}main.lex.c
	${CC} ${CFLAGS} -o ${BUILD_DIR}${APP_NAME} ${BUILD_DIR}main.tab.c \
		${BUILD_DIR}main.lex.c ${BUILD_DIR}main_helper.c ${BUILD_DIR}debug.c -lm

clean:
	- rm -r ${BUILD_DIR}
