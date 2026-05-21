/**
 * purchase.c — 图书采购管理模块实现
 *
 * 【数据结构】
 * PurchaseNode双向链表: 按采购日期顺序存储
 * 采购金额 = 采购数量 * 采购单价 (自动计算)
 */

#include "purchase.h"
#include "fileio.h"

/* ===== 全局变量 ===== */
PurchaseNode *purchase_list_head = NULL;
PurchaseNode *purchase_list_tail = NULL;

/* ===== 初始化 ===== */

void purchase_init(void)
{
    purchase_list_head = load_purchases();
    if (purchase_list_head) {
        purchase_list_tail = purchase_list_head;
        while (purchase_list_tail->next)
            purchase_list_tail = purchase_list_tail->next;
    }
    printf("[初始化] 采购模块就绪。\n");
}

/* ===== 采购操作 ===== */

int purchase_add(PurchaseInfo info)
{
    /* 自动计算采购金额 */
    info.total_amount = info.quantity * info.unit_price;

    PurchaseNode *node = (PurchaseNode*)malloc(sizeof(PurchaseNode));
    node->info = info;
    node->prev = purchase_list_tail;
    node->next = NULL;

    if (!purchase_list_head) {
        purchase_list_head = purchase_list_tail = node;
    } else {
        purchase_list_tail->next = node;
        purchase_list_tail = node;
    }

    printf("[成功] 采购记录已添加。书名: %s, 数量: %d, 金额: ¥%.2f\n",
           info.book_title, info.quantity, info.total_amount);
    save_purchases(purchase_list_head);
    return 1;
}

void purchase_list_all(void)
{
    if (!purchase_list_head) {
        printf("[提示] 暂无采购记录。\n");
        return;
    }
    printf("\n  ╔════════════════════════════════════════════════════════════════════╗\n");
    printf("  ║                     全部采购记录                                   ║\n");
    printf("  ╠════════════════════════════════════════════════════════════════════╣\n");
    printf("  ║ 书名        作者      日期       数量  单价    金额   发票号       ║\n");
    printf("  ╠════════════════════════════════════════════════════════════════════╣\n");
    PurchaseNode *p = purchase_list_head;
    while (p) {
        printf("  ║ %-10s  %-8s  %-10s  %-4d  ¥%-5.2f  ¥%-6.2f  %-10s ║\n",
               p->info.book_title,
               p->info.author,
               p->info.purchase_date,
               p->info.quantity,
               p->info.unit_price,
               p->info.total_amount,
               p->info.invoice_num);
        p = p->next;
    }
    printf("  ╚════════════════════════════════════════════════════════════════════╝\n");
}

PurchaseNode* purchase_find_by_invoice(const char *invoice)
{
    PurchaseNode *p = purchase_list_head;
    while (p) {
        if (strcmp(p->info.invoice_num, invoice) == 0)
            return p;
        p = p->next;
    }
    return NULL;
}

double purchase_total_spent(void)
{
    double total = 0.0;
    PurchaseNode *p = purchase_list_head;
    while (p) {
        total += p->info.total_amount;
        p = p->next;
    }
    return total;
}
