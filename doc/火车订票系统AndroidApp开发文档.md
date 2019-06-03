# 火车订票系统Android App开发文档

## 简介
软件名称：Ticket Booking App
适用平台：Android 8.0 - Android 9.0
开放工具：Android Studio 3.4
开发技术（语言）：Java + xml + kotlin
开发人员：张志成
测试平台：Pixel 2 虚拟机

## 模块设计

### 通讯模块

App与服务器之间的通讯使用Java的socket模块。每次通讯会在一个通讯线程上单独执行，保证不会因为网络通讯速度慢或者通讯异常而导致整个App的卡顿现象。
#### 通讯函数如下：

```java
public void send(final String str, final SocketCallbackListener listener) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    socket = new Socket();
                    socket.setSoTimeout(5000); //change to 5000ms if using remote server, thanks to GFW!
                    socket.connect(new InetSocketAddress(host, port), 5000);

                    Log.d("NetworkActivity", "Sent: " + str);
                    OutputStream os = socket.getOutputStream();
                    os.write(str.getBytes("UTF-8"));
                    socket.shutdownOutput();

                    InputStream is = socket.getInputStream();
                    BufferedReader reader = new BufferedReader(new InputStreamReader(is));
                    StringBuilder ret = new StringBuilder();
                    String tmp;
                    while ((tmp = reader.readLine()) != null) {
                        ret.append(tmp + '\n');
                    }
                    listener.onFinish(ret.toString().trim());
                } catch (Exception e) {
                    listener.onError(e);
                } finally {
                    try {
                        socket.close();
                    } catch (Exception e) {
                        Log.e("NetworkActivity", "connection error" + e.toString());
                    }
                }
            }
        }).start();
```
#### 通讯后的回调函数接口
```java
public interface SocketCallbackListener {
    void onFinish(String result);
    void onError(Exception e);
}
```
发送一条消息的时候只需创建一个新的Client对象，然后再传入一个回调函数的实现即可。以下为实现用户查询自己购票信息的通讯模块实现：
```kotlin
        Client().send(toSend, object : SocketCallbackListener {
            override fun onFinish(result: String?) {
                runOnUiThread {
                    ticketList = parseTicket(result)
                    if (ticketList.isEmpty()) {
                        Snackbar.make(myTicket, R.string.no_ticket, Snackbar.LENGTH_SHORT).show()
                    } else {
                        val viewManager = LinearLayoutManager(this@MyTicket)
                        val viewAdapter = TicketAdapter(ticketList, this@MyTicket)
                        findViewById<RecyclerView>(R.id.myTicketRecycler).apply {
                            layoutManager = viewManager
                            adapter = viewAdapter
                        }
                    }
                }
            }

            override fun onError(e: Exception?) {
                Snackbar.make(myTicket, R.string.error, Snackbar.LENGTH_SHORT).show()
            }
        })
```
### 用户

用户模块主要由LoginActivity、RegisterActivity、UserSetting这三个Activity组成。
当前的用户信息保存在SharedPreference里面，会保存除了密码之外的所有信息。这些信息被用在
1. 显示左侧划出的抽屉中的信息
2. 判断操作权限，以此来显示不同的控件，允许不同的操作
LoginAcitivity用于登陆，登陆成功后会自动跳转回主界面
RegisterActivity用于注册，注册成功后会自动跳转至登陆界面，方便用户
UserSetting允许用户修改自己的信息，还包括修改自己的头像的功能
### 火车票
火车票模块有```QueryTicket```,``` TicketInfo```和```MyTicke```三个Activity和```Ticket```类与```TicketAdapter```类这两个辅助类组成。
QueryTicket用来根据给定的信息查询对应的火车票的简略信息
TicketInfo用来显示用户点击的火车票的详细信息，并且支持在其中购买不同座位的票
MyTicket用来查询并且显示自己的购票信息
Ticket与TicketAdapter为一个辅助类，用来显示能够滑动的RecyclerView，对于符合要求的票比较多的情况很友好。
### 火车管理
火车模块主要有QueryTrain, ModifyTrainPre, ModifyTrain三个Activity和两个辅助类Station和对应的Adapter。
QueryTrain为查询给定的id的火车所有经过的站点
修改火车（包括添加一个火车）比较复杂，所以我分成了两个步骤，第一步是ModifyTrainPre的Activity，用来设置火车的id，名字，以及有哪些座位等级；第二步是ModifyTrain的Activity，用来具体设置火车的站点，包括始发、停留时间、各个座位的价格等。
## UI设计
我们的UI设计对于用户十分友好，做了以下的设计
### Material Design初步尝试
1. 导航
我们使用了Material Design设计的NavigationView控件，使得用户能够通过从屏幕边缘右滑或者点击左上角的图片来进行导航。布局文件nav_header_main和activity_base组成了这个NavigationView。
2. 输入框
我们（在部分地方）采用了符合Material Design的输入框，有好看的动画和错误提示。
3. 动画
我们设计了交换出发地和到达地的按钮的旋转动画。动画十分Robust，多次点击也毫无问题（逃
4. 其他
日期选择、时间选择均使用了标准的控件，并且将其返回的信息编码后现在在文本框内
### 用户友好
1. 修改火车
     修改火车时经常会出现添加站点顺序有误甚至添加了错误的站点。为了解决该问题，我们设计的添加站点的列表支持长按拖动交换两个站点，或者左滑直接删除一个站点

2. 错误提示
     基本的错误都会在屏幕下方进行提示。（对于一些不可预见的错误，选择把锅推给用户，提示操作不合法

3. **自动补全**
    输入站点名称是十分麻烦的，所以我们增加了自动补全功能。只要输入中文拼音或者中文汉字或者对应的拼音首字母，就能在下拉列表中看到提示的站点，直接选择即可。
    具体我先从数据中提取出所有的站点，去重后再转为拼音，用来进行匹配。每次输入的时候自动提取中符合的项目在下拉列表中提示。

### 广告

>一个Android应用的程序是广告             ——Gabriel 

本着这一思想，我们对于登陆、注册、查询车票、查询车次的按钮都添加了显示来自Google AdMob广告的功能。但是为了保证最佳的用户体验，我们决定采用每次以1/3的概率弹出广告。

### 不足: 设计不够Material Design

Material Design不只是使用一些控件，更多的是一种设计规范。这种规范对很多的设计都做出了规范与限制。在这次的App开发之中，我并没有很好地遵守这些限制。

## 收获

积累了Android开发经验
熟悉了Java，初步了解了Kotlin，并且发现就不应该使用Java的555
更加了解了什么才是Material Design