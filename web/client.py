import asyncore, socket

remote_host = ('108.61.126.68', 9001)
logger = None


def send(command):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(remote_host)
    s.send(command.encode())
    ret = ''
    while True:
        t = s.recv(1024)
        if t != b'':
            ret += t.decode('utf-8','ignore')
        else:
            break
    return ret


def parse(result):
    result = result.strip(' ').strip('\n')
    ret = []
    t = result.split('\n')
    for i in t:
        ret.append(i.split(' '))
    if len(ret) == 1:
        ret = ret[0]
    while len(ret) > 0 and ret[-1] == '':
        ret.pop(len(ret) - 1)
    return ret


def init(log):
    global logger
    logger = log
	


def runCommand(command):
    ret = send(command)
    logger.info('Returned: %s' % ret)
    return parse(ret)
