#!/usr/bin/ruby

# add-libs.rb modifies a Projucer (.jucer) file to include library paths and
# names. It gets the library names by running 'stack ls dependencies' and
# searching for the libraries in ~/.stack and ./stack-work. It also adds a
# couple of extra libraries.
#
# Library names are added to the XCODE_MAC tag, and library paths are added to
# the libraryPath field of the CONFIGURATION tags.
#
# Note: this doesn't parse the XML in any way, it just does a dumb line
# substitution, so it assumes that the XCODE_MAC and libraryPath lines don't
# have anything else on them. TODO: fix this
#
# When it searches for libraries, it filters out certain alternate builds like
# profiling; see AVOID_SUFFIXES.
#
# If it finds multiple builds of a library, it first ensures that they are all
# identical, then returns one of them.

# Libs to find in ~/.stack
EXTRA_LIBS = ['Cffi']

# Libs to link in (no path needed)
EXTRA_LINK_LIBS = ["iconv"]

# Don't consider libs ending in these
AVOID_SUFFIXES = [ "_l", "_p", "_thr", "_debug" ]

def assert(b, s='??')
  if !b
    raise StandardError.new("Assertion failure: #{s}")
  end
end

# Verify that all the library files are the same, and return one.
def check_libs_same(files)
  lens = files.map { |f| File.size(f) }
  puts "lens #{lens}"
  sames = files[1..-1].map { |other|
    ok = system("diff #{files[0]} #{other}")
    ret = $?
    assert ok
    [ok, ret]
  }
  puts "sames #{sames}"
  files[0]
end

assert ARGV.length == 1

jucerFile = ARGV[0]

# Like 'primitive-0.6.4.0'
versioned_lib_names = `stack ls dependencies --separator='-'`.split("\n")
puts versioned_lib_names.join('+')

versioned_lib_names += EXTRA_LIBS

# Remove loopo
# versioned_lib_names -= ["loopo-0.1.0.0"]

# rts is libHSrts
versioned_lib_names -= ["rts-1.0"]
versioned_lib_names << "libHSrts"

def should_avoid(f)
  AVOID_SUFFIXES.map { |suf| f.include?("#{suf}.a") }.any?
end

# Find each library in ~/.stack
lib2files = Hash[versioned_lib_names.map { |vln|
  puts "finding #{vln}"
  files = `find ~/.stack | grep #{vln}`.split("\n")
  files += `find #{Dir.pwd}/.stack-work | grep #{vln}`.split("\n")
  files = files.select { |f| f.end_with?(".a") }
  files = files.select { |f| !should_avoid(f) }
  #puts vln, files
  #puts files
  #files.map { |f| puts `ls -l #{f}` }
  assert files.length >= 1, [vln, files]
  file = check_libs_same(files)
  [vln, file]
}]
puts lib2files
puts lib2files.values

def lib_dir_and_name(file)
  m = /^(?<path>.*\/)lib(?<name>[^\/].*)\.a$/.match(file)
  assert m != nil
  #puts "MM #{m}"
  [m['path'], m['name']]
end

# lib_names = lib2files.values.map(&:lib_name)
# lib_dirs = lib2files.values.map(&:lib_dir)
#puts "HUH #{lib2files.values} #{lib2files.values.class}"
lib_dirs, lib_names = lib2files.values.map { |p| lib_dir_and_name(p) }.transpose
puts "lib_names #{lib_names}"
puts "lib_dirs #{lib_dirs}"

lib_names += EXTRA_LINK_LIBS

lib_names_string = lib_names.join("&#10;")
xcode_mac_line = <<-END
  <XCODE_MAC targetFolder="Builds/MacOSX" externalLibraries="#{lib_names_string}">
END

library_path_string = lib_dirs.join("&#10;")
library_path_line = <<-END
  libraryPath="#{library_path_string}"/>
END

jucer = File.read(jucerFile).split("\n")

# Also makes sure there's only one.
def replace_line(lines, findMe, replaceWith, expectedNum)
  assert lines.select{ |s| s.include?(findMe) }.length == expectedNum
  lines.map { |line|
    if line.include?(findMe)
      replaceWith
    else
      line
    end
  }
end

jucer = replace_line(jucer, "<XCODE_MAC", xcode_mac_line, 1)
jucer = replace_line(jucer, "libraryPath", library_path_line, 2)

File.write(jucerFile, jucer.join("\n")+"\n")
