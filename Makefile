all: run

# clean:
# 	rm Source/*.o Source/*.hi Source/*_stub.h
# 	rm libFoo.a

lib:
	stack build

exe: lib
	xcodebuild -project Builds/MacOSX/Loopo.xcodeproj

run: exe
	/Users/gmt/JUCE/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app/Contents/MacOS/AudioPluginHost -NSDocumentRevisionsDebugMode YES loopo.filtergraph

update-libs:
	bin/add-libs.rb Loopo.jucer
