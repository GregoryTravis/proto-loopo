#!/usr/bin/ruby

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

# Remove loopo
# versioned_lib_names -= ["loopo-0.1.0.0"]

# rts is libHSrts
versioned_lib_names -= ["rts-1.0"]
versioned_lib_names << "libHSrts"

# Don't consider libs ending in these
AVOID_SUFFIXES = [ "_l", "_p", "_thr", "_debug" ]

def should_avoid(f)
  AVOID_SUFFIXES.map { |suf| f.include?("#{suf}.a") }.any?
end

# Find each library in ~/.stack
lib2files = Hash[versioned_lib_names.map { |vln|
  puts "finding #{vln}"
  files = `find ~/.stack | grep #{vln}`.split("\n")
  files += `find .stack-work | grep #{vln}`.split("\n")
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
def replace_line(lines, findMe, replaceWith)
  assert lines.select{ |s| s.include?(findMe) }.length == 1
  lines.map { |line|
    if line.include?(findMe)
      replaceWith
    else
      line
    end
  }
end

jucer = replace_line(jucer, "<XCODE_MAC", xcode_mac_line)
jucer = replace_line(jucer, "libraryPath", library_path_line)

#_jucerFile = "__#{jucerFile}"
File.write(jucerFile, jucer.join("\n")+"\n")

=begin
# Unpack each library into a dir
pwd = Dir.pwd
#tmpdir = Dir.mktmpdir
#tmpdir = '/Users/gmt/Loopo/haha'
tmpdir = "tmp-merge-libs.#{$$}"
#`rm -r #{tmpdir}`
`mkdir #{tmpdir}`
lib2files.map { |lib, path|
  #puts "exracting #{vln}"
  libdir = "#{tmpdir}/#{lib}"
  `mkdir #{libdir}`
  Dir.chdir(libdir)
  `ar x #{path}`
  Dir.chdir(pwd)
}

puts "building #{outputLib}"
out = `ar cqs #{outputLib} #{tmpdir}/*/*.o 2>&1`
out = out.split("\n").select { |line| !line.end_with?("has no symbols") }.join("\n")
if !out.empty?
  $stderr.puts out
  exit 1
end
`rm -r #{tmpdir}`
=end
