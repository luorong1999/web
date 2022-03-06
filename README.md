**Linux下C++轻量级Web服务器**

* 使用线程池 + epoll(ET模式) + 同步I/O模拟Proactor模式的并发模型
* 使用有限状态机解析HTTP请求报文，支持解析GET和POST请求
* 通过访问服务器数据库实现web端用户注册、登录功能，可以请求服务器图片和视频文件
* 实现异步日志系统记录服务器运行状态

基础测试
------------
* 服务器测试环境
	* Ubuntu版本20.04
	* MySQL版本5.7.29
* 浏览器测试环境
	* Linux
	* Chrome
	* FireFox


* 测试前确认已安装MySQL数据库

    ```C++
    // 建立yourdb库
    create database yourdb;

    // 创建user表
    USE yourdb;
    CREATE TABLE user(
        username char(50) NULL,
        passwd char(50) NULL
    )ENGINE=InnoDB;

    // 添加数据
    INSERT INTO user(username, passwd) VALUES('name', 'passwd');
    ```

* 修改main.c中的数据库初始化信息

    C++
    // root root修改为服务器数据库的登录名和密码
    connPool->init("localhost", "root", "123456", "yourdb", 3306, 8);
    ```

* 修改http_conn.cpp中的root路径

    C++
    // 修改为root文件夹所在路径
    const char* doc_root="/home/luo/WEBSERVER/root";
    ```

* 生成server

    make server

* 启动server

    ./server 8888

* 浏览器端

    127.0.0.1:8888
