PROJUCER=~/JUCE/Projucer.app/Contents/MacOS/Projucer

# all: run

# clean:
# 	rm Source/*.o Source/*.hi Source/*_stub.h
# 	rm libFoo.a

lib:
	stack build

exe: # lib
	xcodebuild -project Builds/MacOSX/Loopo.xcodeproj

run: exe
	/Users/gmt/JUCE/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app/Contents/MacOS/AudioPluginHost -NSDocumentRevisionsDebugMode YES loopo.filtergraph

update-proj:
	#bin/add-libs.rb Loopo.jucer
	$(PROJUCER) --resave Loopo.jucer

####################

.PHONY: all

INCS=-I/Users/gmt/JUCE/modules/

OS=$(shell ghc -e ":m +System.Info" -e "putStrLn os")

CC = g++
CFLAGS = -O2 $(INCS) -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 -DDEBUG=1 --std=c++11 -undefined dynamic_lookup

.PHONY: clean install

all: cpp/libexample.so

cpp/libexample.so: cpp/utils.o cpp/gen_utils.o cpp/gen_std.o #cpp/juce_MidiMessage.o
ifeq ($(OS),darwin)
	$(CC) $(CFLAGS) -dynamic -shared -fPIC -install_name @rpath/libexample.so -o $@ $^
else
	$(CC) $(CFLAGS) -dynamic -shared -fPIC -o $@ $^
endif

%.o: %.cpp
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

# cpp/juce_MidiMessage.o: /Users/gmt/JUCE/modules/juce_audio_basics/midi/juce_MidiMessage.cpp
# 	$(CC) $(CFLAGS) -fPIC -c -o cpp/juce_MidiMessage.o  /Users/gmt/JUCE/modules/juce_audio_basics/midi/juce_MidiMessage.cpp

clean:
	rm -f cpp/utils.o cpp/gen_utils.o cpp/gen_std.o cpp/libexample.so

install:
ifeq ($(OS),darwin)
	install cpp/libexample.so "$(libdir)"
else
	install -t "$(libdir)" cpp/libexample.so
endif
