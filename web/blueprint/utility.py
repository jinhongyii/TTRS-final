import logging

from flask import session

from client import runCommand

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

msg = {
    'invalid_query': '查询不合法',
    'buy_ticket_failed': '购买失败',
    'refund_ticket_failed': '退票失败',
    'permission_denied': '权限不足',
    'clean_failed': '删库失败',
    'delete_failed': '删除失败',
    'modify_failed': '修改失败',
    'sale_failed': "发售失败",
    'add_train': "添加火车失败",
    'login_failed': "登陆失败",
    'logout_failed': "登出失败",
    'register_failed': "注册失败",
    'insult': 'Stupid Hacker NM$L',
    'refund_failed': '退票失败'
}


def get_args(para, d):
    ret = []
    for p in para:
        ret.append(d.get(p, None))
    return ret


def get_privilege(user=None):
    if 'logged_in' not in session:
        return 0
    if user is None:
        user = session['current_user']
    privilege = runCommand('query_profile ' + user)[3]
    return int(privilege)-1


def query_current_user():
    if 'logged_in' not in session:
        return [None,]
    ret = runCommand('query_profile ' + session['current_user'])
    return ret
