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
    print('sale_train ' + request.form['train_id'])
    print(ret)
    if ret[0] == 0:
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
  #  print(ret)
    #ret = [['5600000G4270', 'G42(杭州东-北京南)', 'G', '12', '4', '高级软卧', '商务座', '软卧', '一等座', '', '', '', '', '', '', '', '', ''], ['杭州东', 'xx:xx', '09:24', 'xx:xx', '￥0', '￥0', '￥0', '￥0', ''], ['桐乡', '09:41', '09:43', '00:02', '￥1366.86', '￥2822.44', '￥870.47', '￥1390.84', ''], ['嘉兴南', '09:54', '09:56', '00:02', '￥1330.56', '￥2880.76', '￥798.33', '￥1003.2', ''], ['上海虹桥', '10:24', '10:26', '00:02', '￥762.79', '￥1398.45', '￥603.01', '￥738.74', ''], ['无锡东', '10:55', '10:57', '00:02', '￥665.18', '￥1301.58', '￥415.74', '￥511.68', ''], ['镇江南', '11:26', '11:28', '00:02', '￥1071.84', '￥1185.8', '￥346.5', '￥785.4', ''], ['南京南', '11:47', '11:51', '00:04', '￥1562.65', '￥2179.25', '￥627.73', '￥1380.12', ''], ['徐州东', '13:05', '13:08', '00:03', '￥824.32', '￥1073.42', '￥304.3', '￥744.72', ''], ['泰安', '13:59', '14:01', '00:02', '￥1061.42', '￥2162.16', '￥655.19', '￥1343.16', ''], [' 济南西', '14:18', '14:27', '00:09', '￥1356', '￥2162.82', '￥833.94', '￥1446.4', ''], ['天津南', '15:30', '15:32', '00:02', '￥1036.12', '￥1546.16', '￥512.04', '￥1134.52', ''], ['北京 南', '16:06', '16:06', 'xx:xx', '￥392.4', '￥462', '￥156.6', '￥216', '']]
    if len(ret) and ret[0] == '0':
        return msg['invalid_query'], 400
    ret = parse_train(ret)
    print(ret[0])
    print(ret[1])
    print(ret[2])
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
    if get_privilege() != 1:
        return msg['permission_denied'], 400
    ret = runCommand('delete_train ' + request.form['train_id'])
    if ret[0] == 0:
        return msg['delete_failed'], 400
    return ret[0]


def encode_train(train):  # encode from a JSON object
    n = len(train)
    t = int(train[n-1]['price_kind'])
    print(type(t))
    print(train)
    s = train[n-1]['name'] + ' ' + train[n-1]['time_arrival'] + ' ' + train[n-1]['time_start'] + ' ' + train[n-1]['time_stopover'] + ' ' + train[n-1]['price_kind']
    for i in range(0, t):
        s += ' ' + train[i]['price_kind']
    i = 0
    while i < (n -1):
        s += '\n'
        s += train[i]['name'] + ' ' + train[i]['time_arrival'] + ' ' + train[i]['time_start'] + ' ' + train[i]['time_stopover']
        for j in range(0, t):
            s += ' '  + train[i+j]['price']
        i += t
        
    return s 
 

@TrainSystem.route('/api/add_train', methods=['POST'])
def add_train(): 
    if get_privilege() != 1:
        return msg['permission_denied'] ,400
    a = request.get_data().decode('utf-8')
    a = json.loads(a)
    ret = runCommand('add_train ' + encode_train(a))
    return ret[0]

@TrainSystem.route('/modify_train', methods=['POST'])
def modify_train_page():
    ret = runCommand('query_train ' + request.form['train_id'])
    print(ret)
    if len(ret) and ret[0] == '0':
        return msg['invalid_query'], 400
    ret = parse_train(ret)
    print(ret[0])
    print(ret[1])
    print(ret[2])
    return render_template('modify_train.html',train=mark_safe(ret[0]),list=mark_safe(ret[1]),price=mark_safe(ret[2]),user=query_current_user()[0], admin=get_privilege())

@TrainSystem.route('/api/modify_train', methods=['POST'])
def modify_train():
    if get_privilege() != 1:
        return msg['permission_denied'], 400
    a = request.get_data().decode('utf-8')
    a = json.loads(a)
    ret = runCommand('modify_train ' + encode_train(a))
    if ret[0] == 0:
        return msg['modify_failed'], 400
    return ret[0]
