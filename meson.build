project('socks', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

executable('socks',
           'main.cpp',
           'SockBase.cpp',
           'SockDgram.cpp',
           'SockStream.cpp',
           dependencies : [ dependency('threads') ],
           # link_args : '-lws2_32',
           install : true)
