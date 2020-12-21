all: libFoo.a

clean:
	rm Source/*.o Source/*.hi Source/*_stub.h
	rm libFoo.a

libFoo.a: Source/Foo.hs
	rm -f libFoo.a Source/Foo.o
	ghc -c Source/Foo.hs
	ar cqs libFoo.a Source/Foo.o
