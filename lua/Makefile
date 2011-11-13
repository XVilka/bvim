# TODO: determine platform
PLATFORM=linux
VERSION=5.1.4

lua: compile install

compile:
	@if [ ! -d lua-${VERSION} ]; then tar -xf lua-${VERSION}.tar.gz; fi
	@if [ ! -d lua-${VERSION} ]; then patch -p0 < bitwise_operators_5.1.4_1.patch; fi
	@if [ ! -d lua-${VERSION} ]; then patch -p0 < jh-lua-binoctal-5.1.4.patch; fi
	cd lua-${VERSION} && make ${PLATFORM}

install:
	@if [ ! -d lib ]; then mkdir lib; fi
	@if [ ! -d include ]; then mkdir include; fi
	cp lua-${VERSION}/src/liblua.a lib/liblua.a
	cp lua-${VERSION}/src/lua.h include/lua.h
	cp lua-${VERSION}/src/luaconf.h include/luaconf.h
	cp lua-${VERSION}/src/lualib.h include/lualib.h
	cp lua-${VERSION}/src/lauxlib.h include/lauxlib.h

clean:
	rm -rf lua-${VERSION}
	rm -rf include
	rm -rf lib
