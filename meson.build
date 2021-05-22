project('socks', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

socks_lib = library('socks',
                    'Host.cpp',
                    'SockPlatform.cpp',
                    'SockHandle.cpp',
                    'SockStream.cpp',
                    'SockDgram.cpp',
                    install : true)

example_basic = executable('example_basic',
                           'examples/basic.cpp',
                           dependencies : [ dependency('threads') ],
                           link_with : socks_lib)

example_http = executable('example_http',
                          'examples/http.cpp',
                          link_with : socks_lib)

example_forward = executable('example_forward',
                             'examples/forward.cpp',
                             dependencies : [ dependency('threads') ],
                             link_with : socks_lib)
