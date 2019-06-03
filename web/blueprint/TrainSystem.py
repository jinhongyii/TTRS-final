from flask import request, render_template, abort, Blueprint
from django.utils.safestring import mark_safe
from client import runCommand
from .utility import msg, get_privilege, get_args, query_current_user
import json

TrainSystem = Blueprint('TrainSystem', __name__)

@TrainSystem.route('/sale_train')
def sale_train_page():
        return render_template('sale_train.html',user=query_current_user()[0], admin=get_privilege())



@TrainSystem.route('/api/sale_train', methods=['POST'])
def sale_train():
    ret = runCommand('sale_train ' + request.form['train_id'])
    if ret == 0:
        return msg['sale_failed'], 400
    else:
        return ret[0]


def parse_train(train):
    t = train[0]
    ret = {'train_id': t[0], 'name': t[1], 'catalog': t[2], 'num_station': t[3], 'num_price': t[4]}
    x = int(t[4])
    price = t[5:5 + x]
    train.pop(0)
    station = []
    while train:
        t = train[0]
        print(t)
        cur_station = {'name': t[0], 'time_arrival': t[1], 'time_start': t[2], 'time_stopover': t[3]}
        x = int(ret['num_price'])
        for i in range(0, x):
            cur_station[price[i]] = t[4 + i]
        station.append(cur_station)
        train.pop(0)
    return [ret, station, price]

@TrainSystem.route('/query_train') 
def query_train_page():
    return render_template('query_train.html',user=query_current_user()[0], admin=get_privilege()) 

@TrainSystem.route('/api/query_train', methods=['POST'])
def query_train():
    print('WWW')
    if len(request.form) == 0:
        print('AAA')
        return render_template('query_train.html')
    ret = runCommand('query_train ' + request.form['train_id'])
    print(ret)
    #ret = [['abc123456', 'G123456', 'G', 2, 1, '商务座'], ['北京', 'xx:xx', '08:00', '00:00', '￥0.0'], ['夏威夷', '08:01', 'xx:xx', '00:00', '￥1.5']]
    if len(ret) and ret[0] == '0':
        abort(Response(msg['invalid_query']))
    ret = parse_train(ret)
    print(ret);
    return render_template('train_result.html', train=ret[0], list=ret[1], price=ret[2])

@TrainSystem.route('/add_train')
def add_train_page():
    if len(request.args) == 0:
        return render_template('add_train.html',user=query_current_user()[0], admin=get_privilege())

@TrainSystem.route('/delete_train')
def delete_train_page():
    return render_template('delete_train.html',user=query_current_user()[0], admin=get_privilege())

@TrainSystem.route('/api/delete_train', methods=['POST'])
def delete_train():
   # if get_privilege() != 3:
   #     abort(Response(msg['permission_denied']))
    ret = runCommand('delete_train ' + request.form['train_id'])
    if ret == 0:
        abort(Response(msg['delete_failed']))
    return ret[0]


def encode_train(train):  # encode from a JSON object
    n = len(train)
    t = int(train[n-1]['price_kind'])
    print(type(t))
    s = train[n-1]['name'] + ' ' + train[n-1]['time_arrival'] + ' ' + train[n-1]['time_start'] + ' ' + train[n-1]['time_stopover'] + ' ' + train[n-1]['price_kind']
    for i in range(0, t):
        s += ' ' + train[i]['price_kind']
    s += '\n'
    for i in range(0, n - 1):
        s += train[i]['name'] + ' ' + train[i]['time_arrival'] + ' ' + train[i]['time_start'] + ' ' + train[i]['time_stopover']
        for j in range(0, t):
            s += ' '  + train[i+j]['price']
        s += '\n'
    return s 
 

@TrainSystem.route('/api/add_train', methods=['POST'])
def add_train(): 
    #if get_privilege() != 3:
    #    abort(Response(msg['permission_denied']))
	a = request.get_data().decode('utf-8')
	a = json.loads(a)
	encode_train(a)
	print(a)
	print('add_train ' + encode_train(a))
	ret = runCommand('add_train ' + encode_train(a))
	return ret[0]

@TrainSystem.route('/modify_train')
def modify_train_page():
    print("WWWW")
    ret = [request.args['train'], request.args['list'], request.args['price']]
    a = request.get_data().decode('utf-8')
    print(ret[0])
    print(ret[1])
    print(ret[2])
    return render_template('modify_train.html',train=mark_safe(ret[0]),list=mark_safe(ret[1]),price=mark_safe(ret[2]),user=query_current_user()[0], admin=get_privilege())

@TrainSystem.route('/api/modify_train', methods=['POST'])
def modify_train():
    if get_privilege() != 3:
        abort(Response(msg['permission_denied']))
    ret = runCommand('modify_train' + encode_train(request.json))
    if ret == 0:
        abort(Response(msg['modify_failed']))
    return ret
