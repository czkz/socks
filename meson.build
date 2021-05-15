project('socks', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++11'])

executable('socks',
           'main.cpp',
           'Host.cpp',
           'SockHandle.cpp',
           'SockHandle.cpp',
           'SockStream.cpp',
           'SockStream.cpp',
           'SockDgram.cpp',
           'SockDgram.cpp',
           dependencies : [ dependency('threads') ],
           # link_args : '-lws2_32',
           install : true)
