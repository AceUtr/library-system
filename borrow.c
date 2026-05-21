/**
 * borrow.c — 借阅管理模块实现
 *
 * 【数据结构说明】
 * 1. BorrowNode双向链表: 存储所有借阅记录，支持按读者/图书查询
 * 2. ReserveQueue (FIFO队列):
 *    - 队首(front): 最早预约的读者，优先获得通知
 *    - 队尾(rear):  最新预约的读者
 *    - 当图书被还回时，从队首取出等待最久的人
 *
 * 【罚款计算规则】
 * - 借阅期限: BORROW_DAYS (30天)
 * - 逾期罚款: FINE_PER_DAY (0.1元/天)
 * - 公式: fine = max(0, (归还日期 - 借阅日期 - 30)) * 0.1
 */

#include "borrow.h"
#include "book.h"
#include "fileio.h"

/* ===== 全局变量 ===== */
BorrowNode *borrow_list_head = NULL;
BorrowNode *borrow_list_tail = NULL;
ReserveQueue *reserve_queue = NULL;

/* ===== 初始化 ===== */

void borrow_init(void)
{
    borrow_list_head = load_borrows();
    if (borrow_list_head) {
        borrow_list_tail = borrow_list_head;
        while (borrow_list_tail->next)
            borrow_list_tail = borrow_list_tail->next;
    }

    /* 初始化预约队列 */
    reserve_queue = (ReserveQueue*)malloc(sizeof(ReserveQueue));
    reserve_queue->front = reserve_queue->rear = NULL;
    reserve_queue->count = 0;

    printf("[初始化] 借阅模块就绪。\n");
}

/* ===== 日期辅助函数 ===== */

/* 将 "yyyy-mm-dd" 字符串转换为自1970-01-01的天数 */
/* 使用 struct tm 进行天数计算，避免手动实现闰年逻辑 */
int date_diff_days(const char *date1, const char *date2)
{
    struct tm tm1 = {0}, tm2 = {0};
    int y1, m1, d1, y2, m2, d2;
    sscanf(date1, "%d-%d-%d", &y1, &m1, &d1);
    sscanf(date2, "%d-%d-%d", &y2, &m2, &d2);
    tm1.tm_year = y1 - 1900; tm1.tm_mon = m1 - 1; tm1.tm_mday = d1;
    tm2.tm_year = y2 - 1900; tm2.tm_mon = m2 - 1; tm2.tm_mday = d2;
    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);
    double diff_sec = difftime(t2, t1);
    return (int)(diff_sec / (60 * 60 * 24));
}

void get_today_str(char *buf, int buf_size)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, buf_size, "%Y-%m-%d", t);
}

/* ===== 借阅操作 ===== */

int borrow_book(const char *card_id, const char *reader_name,
                const char *unit, const char *book_id)
{
    /* 查找图书 */
    BookNode *book = book_find_by_id(book_id);
    if (!book) {
        printf("[错误] 图书编号 %s 不存在。\n", book_id);
        return 0;
    }
    /* 检查在馆数量 */
    if (book_available(book) <= 0) {
        printf("[提示] 图书《%s》已全部借出，可预约排队等待。\n", book->info.title);
        return 0;
    }

    /* 创建借阅记录 */
    BorrowNode *node = (BorrowNode*)malloc(sizeof(BorrowNode));
    memset(&node->info, 0, sizeof(BorrowInfo));
    strcpy(node->info.card_id, card_id);
    strcpy(node->info.reader_name, reader_name);
    strcpy(node->info.unit, unit);
    strcpy(node->info.book_id, book_id);
    strcpy(node->info.book_title, book->info.title);
    get_today_str(node->info.borrow_date, sizeof(node->info.borrow_date));
    strcpy(node->info.return_date, "--");
    node->info.is_returned = 0;
    node->info.fine = 0.0;
    node->prev = borrow_list_tail;
    node->next = NULL;

    /* 插入借阅链表 */
    if (!borrow_list_head) {
        borrow_list_head = borrow_list_tail = node;
    } else {
        borrow_list_tail->next = node;
        borrow_list_tail = node;
    }

    /* 更新图书状态 */
    book->info.borrowed_count++;

    printf("[成功] 《%s》借阅成功！借阅人: %s, 借阅日期: %s, 应还日期: ",
           book->info.title, reader_name, node->info.borrow_date);

    /* 计算应还日期 */
    struct tm tm = {0};
    int y, m, d;
    sscanf(node->info.borrow_date, "%d-%d-%d", &y, &m, &d);
    tm.tm_year = y - 1900; tm.tm_mon = m - 1; tm.tm_mday = d;
    tm.tm_mday += BORROW_DAYS;
    mktime(&tm);
    char due_date[16];
    strftime(due_date, sizeof(due_date), "%Y-%m-%d", &tm);
    printf("%s\n", due_date);

    save_books(book_list_head);
    save_borrows(borrow_list_head);
    return 1;
}

int return_book(const char *card_id, const char *book_id)
{
    /* 查找未归还的借阅记录 */
    BorrowNode *p = borrow_list_head;
    BorrowNode *target = NULL;
    while (p) {
        if (strcmp(p->info.card_id, card_id) == 0 &&
            strcmp(p->info.book_id, book_id) == 0 &&
            !p->info.is_returned) {
            target = p;
            break;
        }
        p = p->next;
    }
    if (!target) {
        printf("[错误] 未找到对应借阅记录。\n");
        return 0;
    }

    /* 记录归还日期 */
    get_today_str(target->info.return_date, sizeof(target->info.return_date));
    target->info.is_returned = 1;

    /* 计算逾期罚款 */
    int days = date_diff_days(target->info.borrow_date, target->info.return_date);
    int overdue = days - BORROW_DAYS;
    if (overdue > 0) {
        target->info.fine = overdue * FINE_PER_DAY;
        printf("[还书] 逾期 %d 天，罚款 ¥%.2f。\n", overdue, target->info.fine);
    } else {
        printf("[还书] 按时归还，无罚款。\n");
    }

    /* 更新图书状态 */
    BookNode *book = book_find_by_id(book_id);
    if (book && book->info.borrowed_count > 0)
        book->info.borrowed_count--;

    /* 检查预约队列 */
    reserve_process_return(book_id);

    save_books(book_list_head);
    save_borrows(borrow_list_head);
    return 1;
}

/* ===== 预约队列操作 ===== */

int reserve_enqueue(const char *reader_name, const char *card_id,
                    const char *book_id)
{
    /* 检查图书是否存在 */
    BookNode *book = book_find_by_id(book_id);
    if (!book) {
        printf("[错误] 图书不存在。\n");
        return 0;
    }
    /* 如果书在馆，直接建议借阅 */
    if (book_available(book) > 0) {
        printf("[提示] 该书在馆，可直接借阅，无需预约。\n");
        return 0;
    }

    ReserveNode *node = (ReserveNode*)malloc(sizeof(ReserveNode));
    strcpy(node->reader_name, reader_name);
    strcpy(node->card_id, card_id);
    strcpy(node->book_id, book_id);
    node->next = NULL;

    /* 入队 (FIFO) */
    if (!reserve_queue->rear) {
        reserve_queue->front = reserve_queue->rear = node;
    } else {
        reserve_queue->rear->next = node;
        reserve_queue->rear = node;
    }
    reserve_queue->count++;
    printf("[成功] %s 已预约《%s》，当前排在第 %d 位。\n",
           reader_name, book->info.title, reserve_queue->count);
    return 1;
}

void reserve_display(void)
{
    if (!reserve_queue || reserve_queue->count == 0) {
        printf("[提示] 当前预约队列为空。\n");
        return;
    }
    printf("\n  ╔══════════════════════════════════════════════╗\n");
    printf("  ║          图书预约队列 (共 %d 人)              ║\n", reserve_queue->count);
    printf("  ╠══════════════════════════════════════════════╣\n");
    printf("  ║ 序号  读者       借书证号      预约图书编号   ║\n");
    printf("  ╠══════════════════════════════════════════════╣\n");
    ReserveNode *p = reserve_queue->front;
    int i = 1;
    while (p) {
        printf("  ║ %-4d  %-10s  %-12s  %-14s ║\n",
               i++, p->reader_name, p->card_id, p->book_id);
        p = p->next;
    }
    printf("  ╚══════════════════════════════════════════════╝\n");
}

void reserve_process_return(const char *book_id)
{
    if (!reserve_queue || !reserve_queue->front) return;

    /* 在队列中查找预约此书的第一个人 */
    ReserveNode *prev = NULL;
    ReserveNode *curr = reserve_queue->front;
    while (curr) {
        if (strcmp(curr->book_id, book_id) == 0) {
            printf("[通知] %s 您好，您预约的图书《%s》已归还，请尽快来借阅！\n",
                   curr->reader_name, book_id);
            /* 出队 */
            if (prev) prev->next = curr->next;
            else reserve_queue->front = curr->next;
            if (curr == reserve_queue->rear)
                reserve_queue->rear = prev;
            free(curr);
            reserve_queue->count--;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

/* ===== 查询操作 ===== */

BorrowNode** borrow_find_by_card(const char *card_id, int *count)
{
    *count = 0;
    BorrowNode *p = borrow_list_head;
    while (p) {
        if (strcmp(p->info.card_id, card_id) == 0)
            (*count)++;
        p = p->next;
    }
    if (*count == 0) return NULL;

    BorrowNode **results = (BorrowNode**)malloc(sizeof(BorrowNode*) * (*count));
    int i = 0;
    p = borrow_list_head;
    while (p) {
        if (strcmp(p->info.card_id, card_id) == 0)
            results[i++] = p;
        p = p->next;
    }
    return results;
}

BorrowNode** borrow_find_unreturned(const char *card_id, int *count)
{
    *count = 0;
    BorrowNode *p = borrow_list_head;
    while (p) {
        if (strcmp(p->info.card_id, card_id) == 0 && !p->info.is_returned)
            (*count)++;
        p = p->next;
    }
    if (*count == 0) return NULL;

    BorrowNode **results = (BorrowNode**)malloc(sizeof(BorrowNode*) * (*count));
    int i = 0;
    p = borrow_list_head;
    while (p) {
        if (strcmp(p->info.card_id, card_id) == 0 && !p->info.is_returned)
            results[i++] = p;
        p = p->next;
    }
    return results;
}

void borrow_list_all(void)
{
    if (!borrow_list_head) {
        printf("[提示] 暂无借阅记录。\n");
        return;
    }
    printf("\n  ╔════════════════════════════════════════════════════════════════════════╗\n");
    printf("  ║                         全部借阅记录                                   ║\n");
    printf("  ╠════════════════════════════════════════════════════════════════════════╣\n");
    printf("  ║ 读者      图书编号    借阅日期    归还日期    状态  罚款               ║\n");
    printf("  ╠════════════════════════════════════════════════════════════════════════╣\n");
    BorrowNode *p = borrow_list_head;
    while (p) {
        printf("  ║ %-8s  %-10s  %-10s  %-10s  %-4s  ¥%-6.2f          ║\n",
               p->info.reader_name,
               p->info.book_id,
               p->info.borrow_date,
               p->info.return_date,
               p->info.is_returned ? "已还" : "未还",
               p->info.fine);
        p = p->next;
    }
    printf("  ╚════════════════════════════════════════════════════════════════════════╝\n");
}

void borrow_print(BorrowNode *node)
{
    if (!node) return;
    printf("\n  读者: %s\n", node->info.reader_name);
    printf("  单位: %s\n", node->info.unit);
    printf("  借书证号: %s\n", node->info.card_id);
    printf("  所借书名: %s (%s)\n", node->info.book_title, node->info.book_id);
    printf("  借阅日期: %s\n", node->info.borrow_date);
    printf("  归还日期: %s\n", node->info.return_date);
    printf("  状态: %s\n", node->info.is_returned ? "已归还" : "未归还");
    if (node->info.fine > 0) printf("  罚款: ¥%.2f\n", node->info.fine);
}
