#!/usr/bin/ruby

def assert(b, s='??')
  if !b
    raise StandardError.new("Assertion failure: #{s}")
  end
end

assert ARGV.length == 1

outputLib = ARGV[0]

`rm -f #{outputLib}`

# Like 'primitive-0.6.4.0'
versioned_lib_names = `stack ls dependencies --separator='-'`.split("\n")
#puts versioned_lib_names.join('+')

# Remove loopo
versioned_lib_names -= ["loopo-0.1.0.0"]

# rts is libHSrts
versioned_lib_names -= ["rts-1.0"]
versioned_lib_names << "libHSrts"

# Find each library in ~/.stack
lib2files = Hash[versioned_lib_names.map { |vln|
  puts "finding #{vln}"
  files = `find ~/.stack | grep #{vln}`.split("\n")
  files = files.select { |f| f.end_with?(".a") }
  files = files.select { |f| !f.end_with?("_p.a") }
  #puts vln, files
  #puts files
  #files.map { |f| puts `ls -l #{f}` }
  assert files.length >= 1, [vln, files]
  file = files[0]
  [vln, file]
}]
#puts lib2files

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
