from flask import request, render_template, abort, Blueprint

from client import runCommand
from .utility import msg, get_args, logger, query_current_user, get_privilege

TicketSystem = Blueprint('TicketSystem', __name__)


def parse_ticket(ticket):
    ret = []
    cnt = 0
    for s in ticket:
        cur = {'train_id': s[0]}
        cur['loc_from'], cur['date_from'], cur['time_from'], cur['loc_to'], cur['date_to'], cur['time_to'] = s[1:7]
        s = s[7:]
        cur['price'] = []
        while len(s) >= 3:
            cur['price'].append({'seat_type': s[0], 'ticket_left': s[1], 'price': s[2]})
            s = s[3:]
        ret.append(cur)
        cnt += 1
    return ret


# the following queries return a json object, which will be used to dynamically update the page
# /query_ticket will return an html page if there's no argument
@TicketSystem.route('/query_ticket')
def query_ticket():
    if len(request.args) == 0:
        return render_template('query_ticket.html', user=query_current_user()[0], admin=get_privilege())  # no specific ticket chosen
    para = ('loc1', 'loc2', 'date', 'catalog')
    ret = runCommand(' '.join(['query_ticket'] + get_args(para, request.args)))

    if len(ret) == 0:
        return msg['invalid_query'], 400
    num = int(ret[0][0])
    ret = parse_ticket(ret[1:])
    logger.debug('ticket info: %s' % ret)
    return render_template('query_result.html', user=query_current_user()[0], num=num, ticket=ret, transfer=False)


@TicketSystem.route('/query_transfer_ticket')
def query_transfer_ticket():
    if len(request.args) == 0:
        return render_template('query_ticket.html', user=query_current_user())  # no specific ticket chosen
    para = ('loc1', 'loc2', 'date', 'catalog')
    ret = runCommand(' '.join(['query_transfer'] + get_args(para, request.args)))
    if len(ret) == 0:
        return msg['invalid_query'], 400
    ret = parse_ticket(ret)
    logger.debug('ticket info: %s' % ret)
    return render_template('query_result.html', user=query_current_user()[0], num=1, ticket=ret, transfer=True)


@TicketSystem.route('/query_order')
def query_order():
    para = ('id', 'date', 'catalog')
    ret = runCommand(' '.join(['query_order'] + get_args(para, request.args)))
    num = int(ret[0])
    ret = parse_ticket(ret[1:])
    return render_template('query_ticket.html', num=num, ticket=ret)


@TicketSystem.route('/api/buy_ticket', methods=['POST'])
def buy_ticket():
    para = ('id', 'num', 'train_id', 'loc1', 'loc2', 'date', 'ticket_kind')
    logger.debug(get_args(para, request.form))
    ret = runCommand(' '.join(['buy_ticket'] + get_args(para, request.form)))
    if len(ret) == 0 or ret[0] == '0':
        return msg['buy_ticket_failed'], 400
    else:
        return ret[0]
