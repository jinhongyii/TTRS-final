# Web TTRS开发文档

## 简介

软件名称：GVJS订票系统

开发环境：windows10、ubuntu16.04

测试环境：windows10、ubuntu16.04

开发技术：HTML+CSS+Javascript+AJAX+Python3.7

使用框架：Flask 1.0.2（Werkzeug0.14.1 Jinja2.10) 、Bootstrap4、Jquery、Vue.js

开发人员：张志成，吕优

## 文件功能说明

```
/static/               #储存静态文件
/template/             #储存网页模板文件
/blueprint             #网站的各个功能的模块实现    
/app.py                #网站主要功能
/client.py             #与后端通信
```

## 具体功能

###  用户系统

  文件为/blueprint/UserSystem.py

- 注册：/register  /api/register，密码使用前端JS md5加密传输
- 登陆：/login  /api/login，用户的登陆状态保存在session里 
- 登出：/api/logout
- 查询用户信息：/profile 同时会返回购票信息，以及退票操作
- 修改用户信息：/settings，/api/mo'dify_profile
- 修改用户权限：/api/modify_privilege
- 退票：/api/refund

### 车票系统

文件为/blueprint/TicketSystem.py

- 查询车票：/query_ticket，并会将信息发送到query_result.html中显示
- 购票：/api/buy_ticket

### 车次系统

文件为/blueprint/TrainSystem.py

- 发售车次：/sale_train，/api/sale_train
- 查询车次：/query_train，/api/query_train，并会将信息发送到train_result.html中显示
- 添加车次：/add_train，/api/add_train
- 删除车次：/delete_train，/api/delete_train
- ~~弃用了的修改车次~~：/modify_train，/api/modify_train

### 管理系统

文件为/blueprint/AdminSystem.py

- 删库跑路：/api/clean

## 前端开发

前端主要利用的是Bootstrap4进行布局，利用Vue.js进行数据处理，利用AJAX进行数据传输。

其中的颜值巅峰为/query_train和/profile

## 前后端通讯

前后端通讯使用了Jinja2，通过runcommand()函数直接将命令发送到后端

## 开发心得

~~模板真好用~~。

吐槽一下修改车次的操作，按要求只能修改未发售的列车，而又不能查询未发售列车的信息，所以这个操作实际上毫无实际作用。。