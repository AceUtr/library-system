/**
 * purchase.h — 图书采购管理模块
 *
 * 使用双向链表存储图书采购记录
 */

#ifndef PURCHASE_H
#define PURCHASE_H

#include "types.h"

/* ===== 全局变量声明 ===== */
extern PurchaseNode *purchase_list_head;
extern PurchaseNode *purchase_list_tail;

/* ===== 初始化 ===== */
void purchase_init(void);

/* ===== 采购操作 ===== */
/* 添加采购记录 */
int purchase_add(PurchaseInfo info);
/* 显示所有采购记录 */
void purchase_list_all(void);
/* 根据发票号查找采购记录 */
PurchaseNode* purchase_find_by_invoice(const char *invoice);
/* 统计总采购支出 */
double purchase_total_spent(void);

#endif /* PURCHASE_H */
