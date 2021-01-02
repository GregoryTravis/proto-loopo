PROJUCER=~/JUCE/Projucer.app/Contents/MacOS/Projucer

all: run

# clean:
# 	rm Source/*.o Source/*.hi Source/*_stub.h
# 	rm libFoo.a

lib: update-proj
	cabal configure && cabal build

exe: lib
	xcodebuild -project Builds/MacOSX/Loopo.xcodeproj

run: exe
	/Users/gmt/JUCE/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app/Contents/MacOS/AudioPluginHost -NSDocumentRevisionsDebugMode YES loopo.filtergraph

update-proj: Loopo.jucer
	bin/add-libs.rb Loopo.jucer HSloopo-0.1.0.0 base-4.12.0.0 vector-0.12.1.2 ghc-prim-0.5.3 integer-gmp-1.0.2.0 HShoppy-example-0.1.0 HShoppy-example-cpp-0.1.0 hoppy-runtime-0.8.0 containers-0.6.0.1
	$(PROJUCER) --resave Loopo.jucer

# array-0.5.3.0
# base-4.12.0.0
# deepseq-1.4.4.0
# ghc-prim-0.5.3
# integer-gmp-1.0.2.0
# primitive-0.6.4.0
# transformers-0.5.6.2
