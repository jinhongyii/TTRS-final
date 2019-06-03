from flask import Blueprint

from client import runCommand
from .utility import msg, get_privilege

AdminSystem = Blueprint('AdminSystem', __name__)


@AdminSystem.route('/api/clean')
def clean():
    privilege = get_privilege()
    if privilege != 1:
        return msg['permission denied'], 400
    ret = runCommand('clean')
    if ret == '1':
        return ret
    else:
        return msg['clean_failed'], 400
