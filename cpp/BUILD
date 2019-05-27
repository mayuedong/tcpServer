cc_binary (
    name = 'bulldozer',
    srcs = [
        #'time.cc',
       # 'exception.cc',
        'thread.cc',
        'threadPool.cc',
       # 'socket.cc',
        'poller.cc',
        'timeStamp.cc',
        'channel.cc',
        'eventLoop.cc',
        'main.cc'
    ],
    incs = [
    ],
    deps = [
        '#pthread',
        '#rt',
    ],
    optimize = ['O0', 'g', 'msse4.2', '-std=c++11', ]
)

