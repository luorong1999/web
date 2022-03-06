#ifndef LST_TIMER
#define LST_TIMER

#include <time.h>
#include "../log/log.h"

class util_timer;

//�û����ݽṹ
struct client_data
{
    sockaddr_in address;  //�ͻ���socket��ַ
    int sockfd;  //������������socket�ļ�������
    util_timer *timer;  //��ʱ��
};

//��ʱ����
class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    time_t expire;  //����ĳ�ʱʱ�䣬�õľ���ʱ��
    void (*cb_func)(client_data *);  //һ������ָ�루ָ������ص�������
    client_data *user_data;  //���ӵĿͻ���Դ
    util_timer *prev;  //ָ��ǰһ����ʱ��
    util_timer *next;  //ָ���һ����ʱ��
};

//��ʱ��������һ������ġ�˫����������ͷ�ڵ��β�ڵ�
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

    //��Ŀ�궨ʱ��timer���뵽������
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
        add_timer(timer, head);  //�������λ��������ͷ�ڵ�֮�������add_timer����
    }

    //����ĳ�ʱʱ�䷢���仯������Ҫ�����������ʱ����λ�ã��ڱ���Ŀ��ֻ���ǳ�ʱʱ���ӳ��������
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

    //ɾ���������ʱ��
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

    //SIGALRM�ź�ÿ�α������������źŴ����������ʹ��ͳһ�¼�Դ����������������ִ��һ��tick����
    // �Դ��������ϵ��ڵ�����
    void tick()
    {
        //�����ʱ������Ϊ�գ���û�ж�ʱ����
        if (!head) 
        {
            return;
        }
        //д����־�ļ�
        LOG_INFO("%s", "timer tick");
        Log::get_instance()->flush();

        //��ȡϵͳ��ǰʱ��
        time_t cur = time(NULL);
        util_timer *tmp = head;
        //��ͷ�ڵ㿪ʼһ�δ���ÿ����ʱ����ֱ������һ����δ���ڵĶ�ʱ��������Ƕ�ʱ���ĺ����߼�
        while (tmp)
        {
            // �������ǿ��԰Ѷ�ʱ���ĳ�ʱʱ���ϵͳ��ǰʱ��Ƚ����ж϶�ʱ���Ƿ���
            if (cur < tmp->expire)
            {
                break;
            }
            //��ʱ����ʱ��ִ�лص�����ִ�ж�ʱ����
            tmp->cb_func(tmp->user_data);
            //ִ���궨ʱ����󽫶�ʱ����������ɾ��������������ͷ�ڵ�
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
    //����λ��������ͷ�ڵ�֮�������add_timer���в���
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
    //ָ�������ͷ�ڵ��β�ڵ�
    util_timer *head;
    util_timer *tail;
};

#endif
