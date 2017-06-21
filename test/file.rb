##
# IO Test

def assert_not_windows(*args, &block)
  assert(*args, &block) if OS.posix?
end

def assert_windows(*args, &block)
  assert(*args, &block) if OS.windows?
end

assert_not_windows('File', '15.2.21') do
  File.class == Class
end

assert_not_windows('File', '15.2.21.2') do
  File.superclass == IO
end

assert_not_windows('File TEST SETUP') do
  MRubyIOTestUtil.io_test_setup
end

assert_not_windows('File#initialize', '15.2.21.4.1') do
  io = File.open($mrbtest_io_rfname, "r")
  assert_nil io.close
  assert_raise IOError do
    io.close
  end
end

assert_not_windows('File#path', '15.2.21.4.2') do
  io = File.open($mrbtest_io_rfname, "r")
  assert_equal $mrbtest_io_msg, io.read
  assert_equal $mrbtest_io_rfname, io.path
  io.close
  assert_equal $mrbtest_io_rfname, io.path
  io.closed?
end

assert_not_windows('File.basename') do
  assert_equal '/', File.basename('//')
  assert_equal 'a', File.basename('/a/')
  assert_equal 'b', File.basename('/a/b')
  assert_equal 'b', File.basename('../a/b')
end

assert_not_windows('File.dirname') do
  assert_equal '.',    File.dirname('')
  assert_equal '.',    File.dirname('a')
  assert_equal '/',    File.dirname('/a')
  assert_equal 'a',    File.dirname('a/b')
  assert_equal '/a',   File.dirname('/a/b')
end

assert_not_windows('File.extname') do
  assert_equal '.txt', File.extname('foo/foo.txt')
  assert_equal '.gz',  File.extname('foo/foo.tar.gz')
  assert_equal '', File.extname('foo/bar')
  assert_equal '', File.extname('foo/.bar')
  assert_equal '', File.extname('foo.txt/bar')
  assert_equal '', File.extname('.foo')
end

assert_not_windows('IO#flock') do
  f = File.open $mrbtest_io_rfname
  begin
    assert_equal(f.flock(File::LOCK_SH), 0)
    assert_equal(f.flock(File::LOCK_UN), 0)
    assert_equal(f.flock(File::LOCK_EX | File::LOCK_NB), 0)
    assert_equal(f.flock(File::LOCK_UN), 0)
  rescue NotImplementedError => e
    skip e.message
  ensure
    f.close
  end
end

assert_not_windows('File.join') do
  assert_equal "", File.join()
  assert_equal "a", File.join("a")
  assert_equal "/a", File.join("/a")
  assert_equal "a/", File.join("a/")
  assert_equal "a/b/c", File.join("a", "b", "c")
  assert_equal "/a/b/c", File.join("/a", "b", "c")
  assert_equal "a/b/c/", File.join("a", "b", "c/")
  assert_equal "a/b/c", File.join("a/", "/b/", "/c")
  assert_equal "a/b/c", File.join(["a", "b", "c"])
  assert_equal "a/b/c", File.join("a", ["b", ["c"]])
end

assert_not_windows('File.realpath') do
  if File::ALT_SEPARATOR
    readme_path = File._getwd + File::ALT_SEPARATOR + "README.md"
    assert_equal readme_path, File.realpath("README.md")
  else
    dir = MRubyIOTestUtil.mkdtemp("mruby-io-test.XXXXXX")
    begin
      dir1 = File.realpath($mrbtest_io_rfname)
      dir2 = File.realpath("./#{dir}//./../#{$mrbtest_io_symlinkname}")
      assert_equal dir1, dir2
    ensure
      MRubyIOTestUtil.rmdir dir
    end
  end
end

assert_not_windows("File.readlink") do
  begin
    assert_equal $mrbtest_io_rfname, File.readlink($mrbtest_io_symlinkname)
  rescue NotImplementedError => e
    skip e.message
  end
end

assert_not_windows("File.readlink fails with non-symlink") do
  begin
    assert_raise(RuntimeError) {
      begin
        File.readlink($mrbtest_io_rfname)
      rescue => e
        if Object.const_defined?(:SystemCallError) and e.kind_of?(SystemCallError)
          raise RuntimeError, "SystemCallError converted to RuntimeError"
        end
        raise e
      end
    }
  rescue NotImplementedError => e
    skip e.message
  end
end

assert_not_windows('File TEST CLEANUP') do
  assert_nil MRubyIOTestUtil.io_test_cleanup
end

assert_not_windows('File.expand_path') do
  assert_equal "/",    File.expand_path("..", "/tmp"),       "parent path with base_dir (1)"
  assert_equal "/tmp", File.expand_path("..", "/tmp/mruby"), "parent path with base_dir (2)"

  assert_equal "/home", File.expand_path("/home"),      "absolute"
  assert_equal "/home", File.expand_path("/home", "."), "absolute with base_dir"

  assert_equal "/hoge", File.expand_path("/tmp/..//hoge")
  assert_equal "/hoge", File.expand_path("////tmp/..///////hoge")

  assert_equal "/", File.expand_path("../../../..", "/")
  if File._getwd[1] == ":"
    drive_letter = File._getwd[0]
    assert_equal drive_letter + ":\\", File.expand_path(([".."] * 100).join("/"))
  else
    assert_equal "/", File.expand_path(([".."] * 100).join("/"))
  end
end

assert_not_windows('File.expand_path (with ENV)') do
  skip unless Object.const_defined?(:ENV) && ENV['HOME']

  assert_equal ENV['HOME'], File.expand_path("~/"),      "home"
  assert_equal ENV['HOME'], File.expand_path("~/", "/"), "home with base_dir"

  assert_equal "#{ENV['HOME']}/user", File.expand_path("user", ENV['HOME']), "relative with base_dir"
end

assert_not_windows('File.path') do
  assert_equal "", File.path("")
  assert_equal "a/b/c", File.path("a/b/c")
  assert_equal "a/../b/./c", File.path("a/../b/./c")
  assert_raise(TypeError) { File.path(nil) }
  assert_raise(TypeError) { File.path(123) }

end

assert_not_windows('File.symlink') do
  target_name = "/usr/bin"
  symlink_name = "test-bin-dummy"
  if !File.exist?(target_name)
    skip("target directory of File.symlink is not found")
  else
    assert_equal 0, File.symlink(target_name, symlink_name)
    begin
      assert_equal true, File.symlink?(symlink_name)
    ensure
      File.delete symlink_name
    end
  end
end

assert_not_windows('File.chmod') do
  File.open('chmod-test', 'w') {}
  begin
    assert_equal 1, File.chmod(0400, 'chmod-test')
  ensure
    File.delete('chmod-test')
  end
end
