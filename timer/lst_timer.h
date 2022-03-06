#ifndef LST_TIMER
#define LST_TIMER

#include <time.h>
#include "../log/log.h"

class util_timer;

//用户数据结构
struct client_data
{
    sockaddr_in address;  //客户端socket地址
    int sockfd;  //服务器端连接socket文件描述符
    util_timer *timer;  //定时器
};

//定时器类
class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    time_t expire;  //任务的超时时间，用的绝对时间
    void (*cb_func)(client_data *);  //一个函数指针（指向任务回调函数）
    client_data *user_data;  //连接的客户资源
    util_timer *prev;  //指向前一个定时器
    util_timer *next;  //指向后一个定时器
};

//定时器链表，是一个升序的、双向链表，带有头节点和尾节点
class sort_timer_lst
{
public:
    sort_timer_lst() : head(NULL), tail(NULL) {}  
    ~sort_timer_lst()
    {
        util_timer *tmp = head;
        while (tmp)
        {
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }

    //将目标定时器timer加入到链表中
    void add_timer(util_timer *timer)
    {
        if (!timer)
        {
            return;
        }
        if (!head)
        {
            head = tail = timer;
            return;
        }
        if (timer->expire < head->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer(timer, head);  //如果插入位置在链表头节点之后则调用add_timer插入
    }

    //任务的超时时间发生变化，则需要调整这个任务定时器的位置（在本项目中只考虑超时时间延长的情况）
    void adjust_timer(util_timer *timer)
    {
        if (!timer)
        {
            return;
        }
        util_timer *tmp = timer->next;
        if (!tmp || (timer->expire < tmp->expire))
        {
            return;
        }
        if (timer == head)
        {
            head = head->next;
            head->prev = NULL;
            timer->next = NULL;
            add_timer(timer, head);
        }
        else
        {
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer, timer->next);
        }
    }

    //删除这个任务定时器
    void del_timer(util_timer *timer)
    {
        if (!timer)
        {
            return;
        }
        if ((timer == head) && (timer == tail))
        {
            delete timer;
            head = NULL;
            tail = NULL;
            return;
        }
        if (timer == head)
        {
            head = head->next;
            head->prev = NULL;
            delete timer;
            return;
        }
        if (timer == tail)
        {
            tail = tail->prev;
            tail->next = NULL;
            delete timer;
            return;
        }
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }

    //SIGALRM信号每次被处罚就在其信号处理函数（如果使用统一事件源，则是主函数）中执行一次tick函数
    // 以处理链表上到期的任务
    void tick()
    {
        //如果定时器链表为空，则没有定时任务
        if (!head) 
        {
            return;
        }
        //写入日志文件
        LOG_INFO("%s", "timer tick");
        Log::get_instance()->flush();

        //获取系统当前时间
        time_t cur = time(NULL);
        util_timer *tmp = head;
        //从头节点开始一次处理每个定时器，直到遇到一个尚未到期的定时器，这就是定时器的核心逻辑
        while (tmp)
        {
            // 所以我们可以把定时器的超时时间和系统当前时间比较以判断定时器是否到期
            if (cur < tmp->expire)
            {
                break;
            }
            //定时器超时则执行回调函数执行定时任务
            tmp->cb_func(tmp->user_data);
            //执行完定时任务后将定时器从链表中删除，并重置链表头节点
            head = tmp->next;
            if (head)
            {
                head->prev = NULL;
            }
            delete tmp;
            tmp = head;
        }
    }

private:
    //插入位置在链表头节点之后，则调用add_timer进行插入
    void add_timer(util_timer *timer, util_timer *lst_head)
    {
        util_timer *prev = lst_head;
        util_timer *tmp = prev->next;
        while (tmp)
        {
            if (timer->expire < tmp->expire)
            {
                prev->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                timer->prev = prev;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }
        if (!tmp)
        {
            prev->next = timer;
            timer->prev = prev;
            timer->next = NULL;
            tail = timer;
        }
    }

private:
    //指向链表的头节点和尾节点
    util_timer *head;
    util_timer *tail;
};

#endif
