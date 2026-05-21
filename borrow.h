/**
 * borrow.h — 借阅管理模块
 *
 * 使用双向链表存储所有借阅记录
 * 使用FIFO队列管理图书预约排队
 */

#ifndef BORROW_H
#define BORROW_H

#include "types.h"

/* ===== 全局变量声明 ===== */
extern BorrowNode *borrow_list_head;
extern BorrowNode *borrow_list_tail;
extern ReserveQueue *reserve_queue;     /* 全局预约队列 (可改进为每本书一个队列) */

/* ===== 初始化 ===== */
void borrow_init(void);

/* ===== 借阅操作 ===== */
/* 借书: 检查在馆数 > 0，创建借阅记录 */
int borrow_book(const char *card_id, const char *reader_name,
                const char *unit, const char *book_id);
/* 还书: 计算逾期罚款，更新图书状态 */
int return_book(const char *card_id, const char *book_id);

/* ===== 预约操作 (队列实现) ===== */
/* 预约排队 (书被借光时可预约) */
int reserve_enqueue(const char *reader_name, const char *card_id,
                    const char *book_id);
/* 显示预约队列 */
void reserve_display(void);
/* 处理还书后的预约通知 */
void reserve_process_return(const char *book_id);

/* ===== 查询操作 ===== */
/* 根据借书证号查找所有借阅记录 */
BorrowNode** borrow_find_by_card(const char *card_id, int *count);
/* 查找某读者的未还书记录 */
BorrowNode** borrow_find_unreturned(const char *card_id, int *count);
/* 显示所有借阅记录 */
void borrow_list_all(void);

/* ===== 辅助函数 ===== */
/* 计算两个日期之间的天数差 */
int date_diff_days(const char *date1, const char *date2);
/* 获取今天的日期字符串 */
void get_today_str(char *buf, int buf_size);
/* 打印单条借阅记录 */
void borrow_print(BorrowNode *node);

#endif /* BORROW_H */
