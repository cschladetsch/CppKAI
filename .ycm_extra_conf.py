import os
import ycm_core

# These are the compilation flags that will be used by YCM
flags = [
  '-Wall',
  '-Wextra',
  '-Werror',
  '-std=c++20',  # Change this to match your project's C++ standard
  '-x', 'c++',
  '-I', 'include',  # Add your include directories here
  '-I', 'Include',
]

# Set this to the absolute path to the folder (NOT the file!) containing the compilation database
compilation_database_folder = ''

if os.path.exists(compilation_database_folder):
  database = ycm_core.CompilationDatabase(compilation_database_folder)
else:
  database = None

SOURCE_EXTENSIONS = ['.cpp', '.cxx', '.cc', '.c', '.m', '.mm']

def DirectoryOfThisScript():
  return os.path.dirname(os.path.abspath(__file__))

def MakeRelativePathsInFlagsAbsolute(flags, working_directory):
  if not working_directory:
    return list(flags)
  new_flags = []
  make_next_absolute = False
  path_flags = ['-isystem', '-I', '-iquote', '--sysroot=']
  for flag in flags:
    new_flag = flag

    if make_next_absolute:
      make_next_absolute = False
      if not flag.startswith('/'):
        new_flag = os.path.join(working_directory, flag)

    for path_flag in path_flags:
      if flag == path_flag:
        make_next_absolute = True
        break

      if flag.startswith(path_flag):
        path = flag[len(path_flag):]
        new_flag = path_flag + os.path.join(working_directory, path)
        break

    if new_flag:
      new_flags.append(new_flag)
  return new_flags

def FlagsForFile(filename, **kwargs):
  if database:
    # Bear in mind that compilation_info.compiler_flags_ does NOT return a
    # python list, but a "list-like" StringVec object
    compilation_info = database.GetCompilationInfoForFile(filename)
    final_flags = MakeRelativePathsInFlagsAbsolute(
      compilation_info.compiler_flags_,
      compilation_info.compiler_working_dir_)

    # NOTE: This is just for YouCompleteMe; it's highly likely that your project
    # does NOT need to remove the stdlib flag. DO NOT USE THIS IN YOUR
    # ycm_extra_conf IF YOU'RE NOT 100% SURE YOU NEED IT.
    try:
      final_flags.remove('-stdlib=libc++')
    except ValueError:
      pass
  else:
    relative_to = DirectoryOfThisScript()
    final_flags = MakeRelativePathsInFlagsAbsolute(flags, relative_to)

  return {
    'flags': final_flags,
    'do_cache': True
  }
