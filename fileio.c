/**
 * fileio.c — 文件读写模块实现
 *
 * 数据存储采用文本格式，每行一条记录，字段用'|'分隔。
 * 这种格式的优点：
 *   1. 可直接用文本编辑器查看和修改
 *   2. 跨平台兼容性好
 *   3. 便于调试
 *
 * 作者链表在文件中用逗号拼接存储，如 "张三,李四,王五"
 */

#include "fileio.h"

/* ===== 辅助函数：作者链表与字符串互转 ===== */

/* 将作者链表转换为逗号分隔的字符串 */
static void authors_to_string(AuthorNode *head, char *buf, int buf_size)
{
    buf[0] = '\0';
    AuthorNode *p = head;
    while (p) {
        if (strlen(buf) > 0) strcat(buf, ",");
        strncat(buf, p->name, buf_size - strlen(buf) - 1);
        p = p->next;
    }
    /* 如果没有作者，写入"未知" */
    if (strlen(buf) == 0) strcpy(buf, "未知");
}

/* 将逗号分隔的字符串还原为作者链表 */
static AuthorNode* string_to_authors(const char *str)
{
    AuthorNode *head = NULL, *tail = NULL;
    if (!str || strlen(str) == 0) return NULL;

    char *copy = (char*)malloc(strlen(str) + 1);
    if (!copy) return NULL;
    strcpy(copy, str);
    char *token = strtok(copy, ",");
    while (token) {
        AuthorNode *node = (AuthorNode*)malloc(sizeof(AuthorNode));
        strncpy(node->name, token, MAX_NAME_LEN - 1);
        node->name[MAX_NAME_LEN - 1] = '\0';
        node->next = NULL;
        if (!head) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
        token = strtok(NULL, ",");
    }
    free(copy);
    return head;
}

/* 统计作者数量 */
static int count_authors(AuthorNode *head)
{
    int count = 0;
    while (head) { count++; head = head->next; }
    return count;
}

/* ==================== 图书数据操作 ==================== */

BookNode* load_books(void)
{
    FILE *fp = fopen(BOOKS_FILE, "r");
    if (!fp) {
        printf("[提示] 未找到 %s，将创建新数据文件。\n", BOOKS_FILE);
        return NULL;
    }

    BookNode *head = NULL, *tail = NULL;
    char line[2048];

    while (fgets(line, sizeof(line), fp)) {
        /* 移除末尾换行符 */
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        BookNode *node = (BookNode*)malloc(sizeof(BookNode));
        memset(&node->info, 0, sizeof(BookInfo));
        node->prev = tail;
        node->next = NULL;

        char authors_str[512] = "";

        /* 格式: class_id|book_id|title|authors|publisher|pub_date|isbn|edition|price|total|borrowed */
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%d|%lf|%d|%d",
               node->info.class_id,
               node->info.book_id,
               node->info.title,
               authors_str,
               node->info.publisher,
               node->info.pub_date,
               node->info.isbn,
               &node->info.edition,
               &node->info.price,
               &node->info.total_copies,
               &node->info.borrowed_count);

        node->info.authors = string_to_authors(authors_str);
        node->info.author_count = count_authors(node->info.authors);

        if (!head) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }
    fclose(fp);
    printf("[加载] 已从 %s 读取图书数据。\n", BOOKS_FILE);
    return head;
}

void save_books(BookNode *head)
{
    FILE *fp = fopen(BOOKS_FILE, "w");
    if (!fp) {
        printf("[错误] 无法写入 %s！\n", BOOKS_FILE);
        return;
    }

    BookNode *p = head;
    while (p) {
        char authors_str[512];
        authors_to_string(p->info.authors, authors_str, sizeof(authors_str));

        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|%d|%.2f|%d|%d\n",
                p->info.class_id,
                p->info.book_id,
                p->info.title,
                authors_str,
                p->info.publisher,
                p->info.pub_date,
                p->info.isbn,
                p->info.edition,
                p->info.price,
                p->info.total_copies,
                p->info.borrowed_count);
        p = p->next;
    }
    fclose(fp);
    printf("[保存] 图书数据已写入 %s。\n", BOOKS_FILE);
}

/* ==================== 借阅数据操作 ==================== */

BorrowNode* load_borrows(void)
{
    FILE *fp = fopen(BORROWS_FILE, "r");
    if (!fp) {
        printf("[提示] 未找到 %s，将创建新数据文件。\n", BORROWS_FILE);
        return NULL;
    }

    BorrowNode *head = NULL, *tail = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        BorrowNode *node = (BorrowNode*)malloc(sizeof(BorrowNode));
        memset(&node->info, 0, sizeof(BorrowInfo));
        node->prev = tail;
        node->next = NULL;

        /* 格式: reader_name|unit|card_id|book_id|book_title|borrow_date|return_date|is_returned|fine */
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%d|%lf",
               node->info.reader_name,
               node->info.unit,
               node->info.card_id,
               node->info.book_id,
               node->info.book_title,
               node->info.borrow_date,
               node->info.return_date,
               &node->info.is_returned,
               &node->info.fine);

        if (!head) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }
    fclose(fp);
    printf("[加载] 已从 %s 读取借阅数据。\n", BORROWS_FILE);
    return head;
}

void save_borrows(BorrowNode *head)
{
    FILE *fp = fopen(BORROWS_FILE, "w");
    if (!fp) {
        printf("[错误] 无法写入 %s！\n", BORROWS_FILE);
        return;
    }

    BorrowNode *p = head;
    while (p) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|%d|%.2f\n",
                p->info.reader_name,
                p->info.unit,
                p->info.card_id,
                p->info.book_id,
                p->info.book_title,
                p->info.borrow_date,
                p->info.return_date,
                p->info.is_returned,
                p->info.fine);
        p = p->next;
    }
    fclose(fp);
    printf("[保存] 借阅数据已写入 %s。\n", BORROWS_FILE);
}

/* ==================== 采购数据操作 ==================== */

PurchaseNode* load_purchases(void)
{
    FILE *fp = fopen(PURCHASES_FILE, "r");
    if (!fp) {
        printf("[提示] 未找到 %s，将创建新数据文件。\n", PURCHASES_FILE);
        return NULL;
    }

    PurchaseNode *head = NULL, *tail = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        PurchaseNode *node = (PurchaseNode*)malloc(sizeof(PurchaseNode));
        memset(&node->info, 0, sizeof(PurchaseInfo));
        node->prev = tail;
        node->next = NULL;

        /* 格式: book_title|author|purchase_date|quantity|unit_price|total_amount|invoice_num|book_id */
        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%lf|%lf|%[^|]|%[^|]",
               node->info.book_title,
               node->info.author,
               node->info.purchase_date,
               &node->info.quantity,
               &node->info.unit_price,
               &node->info.total_amount,
               node->info.invoice_num,
               node->info.book_id);

        if (!head) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }
    fclose(fp);
    printf("[加载] 已从 %s 读取采购数据。\n", PURCHASES_FILE);
    return head;
}

void save_purchases(PurchaseNode *head)
{
    FILE *fp = fopen(PURCHASES_FILE, "w");
    if (!fp) {
        printf("[错误] 无法写入 %s！\n", PURCHASES_FILE);
        return;
    }

    PurchaseNode *p = head;
    while (p) {
        fprintf(fp, "%s|%s|%s|%d|%.2f|%.2f|%s|%s\n",
                p->info.book_title,
                p->info.author,
                p->info.purchase_date,
                p->info.quantity,
                p->info.unit_price,
                p->info.total_amount,
                p->info.invoice_num,
                p->info.book_id);
        p = p->next;
    }
    fclose(fp);
    printf("[保存] 采购数据已写入 %s。\n", PURCHASES_FILE);
}
