cc_binary (
    name = 'bulldozer',
    srcs = [
        'thread.cc',
        'threadPool.cc',
        'socket.cc',
        'poller.cc',
        'acceptor.cc',
        'server.cc',
        'timeStamp.cc',
        'timer.cc',
        'tEvent.cc',
        'fEvent.cc',
        'eventLoop.cc',
        'threadEventLoop.cc',
        'eventLoopPool.cc',
        'connection.cc',
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

