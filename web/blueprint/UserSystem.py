from flask import request, render_template, session, abort, json, Blueprint
from wtforms import Form, StringField, PasswordField, IntegerField
from wtforms.validators import DataRequired, Email

from client import runCommand
from .utility import msg, get_args, logger, query_current_user, get_privilege

UserSystem = Blueprint('UserSystem', __name__)


class LoginForm(Form):
    id = IntegerField(validators=[DataRequired()])
    password = PasswordField(validators=[DataRequired()])


class RegisterForm(Form):
    name = StringField(validators=[DataRequired()])
    password = PasswordField(validators=[DataRequired()])
    email = StringField(validators=[DataRequired(), Email()])
    phone = IntegerField(validators=[DataRequired()])


@UserSystem.route('/')
def home_page():
    current_user = session.get('current_user', None)
    if current_user is None:
        return '', 302, {'Location': '/login'}  # asks the user to login if not already
    return render_template('main.html', user=query_current_user()[0])

@UserSystem.route('/admin')
def admin_page():
    return render_template("admin.html", admin=query_current_user()[3], user=query_current_user()[0]);

@UserSystem.route('/login')
def login_page():
    if 'logged_in' in session:
        return '', 302, {'Location': '/'}  # if logged in already, redirect to main page
    return render_template("login.html")


@UserSystem.route('/register')
def register_page():
    if 'logged_in' in session:
        return '', 302, {'Location': '/'}  # if logged in already, redirect to main page
    return render_template('register.html')


@UserSystem.route('/api/register', methods=['POST'])
def register():
    if not RegisterForm(request.form).validate():
        return msg['register_failed'], 400

    para = ('name', 'password', 'email', 'phone')
    logger.info('register: %s' % get_args(para, request.form))
    ret_id = runCommand(' '.join(['register'] + get_args(para, request.form)))
    if len(ret_id) > 0 and ret_id[0] != '-1':
        return 'Your id: ' + ret_id[0]
    else:
        return msg['register_failed'], 400


@UserSystem.route('/api/login', methods=['POST'])
def login():
    if not LoginForm(request.form).validate():
        return msg['login_failed'], 400

    para = ('id', 'password', 'remember')
    current_user, password, remember = get_args(para, request.form)
    ret = runCommand(' '.join(['login', current_user, password]))
    if len(ret) == 0 or ret[0] == '0':
        return msg['login_failed'], 400

    session.permanent = remember
    session['logged_in'] = True
    session['current_user'] = current_user
    return json.dumps(ret)


@UserSystem.route('/api/logout', methods=['POST'])
def logout():
    if 'logged_in' in session:
        session.pop('logged_in')
        session.pop('current_user')
        session.permanent = False
        return '', 200
    else:
        return msg['logout_failed'], 400

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

@UserSystem.route('/profile')
def profile_page():
    if 'logged_in' not in session:
        return '', 302, {'Location': '/login'}
    logger.debug('current user: %s' % query_current_user())
    t1 = query_current_user()[0]
    t2 = query_current_user()
    t3 = runCommand('query_order ' + session['current_user'])
    t3 = parse_ticket(t3[1:])
    return render_template('profile.html', user=t1, user_full=t2, t=t3,admin=int(query_current_user()[3]) - 1)

@UserSystem.route('/settings')
def settings_page():
    if 'logged_in' not in session:
        return '', 302, {'Location': '/login'}
    return render_template('settings.html', user=query_current_user()[0], admin=int(query_current_user()[3]) - 1)


@UserSystem.route('/api/modify_profile', methods=['POST'])
def modify_profile():
    if 'logged_in' not in session:
        return msg['insult'], 400
    para = ('user_id', 'name', 'password', 'email', 'phone')
    tmp = get_args(para, request.form)
    tmp[0] = session['current_user']
    ret = runCommand(' '.join(['modify_profile'] + tmp))
    ret = ret[0]
    if int(ret) == 1:
        return ret
    else:
        return msg['modify_failed'], 400


@UserSystem.route('/api/modify_privilege', methods=['POST'])  # currently with no privilege checking
def modify_privilege():
    para = ('id1', 'id2', 'change')
    tmp = get_args(para, request.form)
    tmp[0] = session['current_user']
    ret = runCommand(' '.join(['modify_privilege'] + tmp))
    ret = ret[0]
    print(ret)
    if int(ret) == 1:
        return ret
    else:
        abort(401)

@UserSystem.route('/api/refund', methods=['POST'])
def refund_api():
    para = ('id', 'num', 'train_id', 'loc1', 'loc2', 'date', 'ticket_kind')
    ret = runCommand(' '.join(['refund'] + get_args(para, request.form)))
    if ret == 0:
        return msg['refund_failed'], 400
    else:
        return ret