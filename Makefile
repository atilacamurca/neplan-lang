#
# Makefile for neplan-lang
#

CFLAGS = -g
BUILD_DIR = build/
FIXTURES_DIR = fixtures/
APP_NAME = neplan
SOURCE_FILES = src/debug.h src/debug.c src/main.h

dist: clean build
	@echo 'Making target $@ ...'

fixtures: build
	@echo 'Testing fixtures ...'
	./${BUILD_DIR}${APP_NAME} -d ${FIXTURES_DIR}arithmetics-ops.n

build: src/main.l
	mkdir -p ${BUILD_DIR}
	flex -o ${BUILD_DIR}$@.c $<
	cp ${SOURCE_FILES} ${BUILD_DIR}
	${CC} ${CFLAGS} -o ${BUILD_DIR}${APP_NAME} ${BUILD_DIR}$@.c ${BUILD_DIR}debug.c

clean:
	- rm -r ${BUILD_DIR}
