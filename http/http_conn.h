#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
class http_conn
{
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    void init(int sockfd, const sockaddr_in &addr);  //初始化新建立的连接
    void close_conn(bool real_close = true);    //关闭此连接
    void process();    //处理客户请求
    bool read_once();    //读取HTTP请求
    bool write();    //写HTTP请求得到的结果

    sockaddr_in *get_address()
    {
        return &m_address;
    }

    void initmysql_result(connection_pool *connPool);

private:
    void init();    //初始化连接
    HTTP_CODE process_read();   //解析HTTP请求
    bool process_write(HTTP_CODE ret);  //填充HTTP请求

    //下面的一组函数用于分析HTTP请求
    HTTP_CODE parse_request_line(char *text);   //解析HTTP请求行
    HTTP_CODE parse_headers(char *text);    //解析HTTP请求头部
    HTTP_CODE parse_content(char *text);    //解析HTTP请求主体
    HTTP_CODE do_request();     //解析得到一个正确HTTP请求后，分析目标文件属性并mmap映射到内存中，告诉调用者获得文件成功
    char *get_line() { return m_read_buf + m_start_line; };     //获取当前行
    LINE_STATUS parse_line();    //解析出一行内容，每行的\r\n替换成\0\0

    void unmap();

    //下面的一组函数用于填充HTTP应答
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;

private:
    int m_sockfd;
    sockaddr_in m_address;

    char m_read_buf[READ_BUFFER_SIZE];
    int m_read_idx;

    int m_checked_idx;
    int m_start_line;

    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;

    CHECK_STATE m_check_state;
    METHOD m_method;

    char m_real_file[FILENAME_LEN];
    char *m_url;
    char *m_version;
    char *m_host;
    int m_content_length;
    bool m_linger;

    char *m_file_address;
    struct stat m_file_stat;
    struct iovec m_iv[2];
    int m_iv_count;

    int cgi;        //是否启用的POST
    char *m_string; //存储请求头数据
    int bytes_to_send;
    int bytes_have_send;
};

#endif
