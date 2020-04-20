.DEFAULT_GOALS: all

all: build_server build_client

build_server:
	@[ ! -d './bin' ] && mkdir bin || :
	@echo 'Compiling echoerd server'
	@gcc -o bin/echoerd src/echoerd.c && \
	echo 'Echoerd server compilation successful.' || \
	echo 'Echoerd server compilation failed.'

build_client:
	@[ ! -d './bin' ] && mkdir bin || :
	@echo 'Compiling echoer client'
	@gcc -o bin/echoer src/echoer.c && \
	echo 'Echoer client compilation successful.' || \
	echo 'Echoer client compilation failed.'
