cc_binary (
    name = 'bulldozer',
    srcs = [
        'timer.cc',
       # 'exception.cc',
        'thread.cc',
        'threadPool.cc',
       # 'socket.cc',
        'poller.cc',
        'timeStamp.cc',
        'fEvent.cc',
        'tEvent.cc',
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

