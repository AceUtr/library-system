/**
 * fileio.h — 文件读写模块
 * 负责程序启动时加载数据、退出时保存数据
 * 数据格式为文本文件，使用'|'作为字段分隔符
 */

#ifndef FILEIO_H
#define FILEIO_H

#include "types.h"

/* 数据文件路径 */
#define BOOKS_FILE      "books.dat"
#define BORROWS_FILE    "borrows.dat"
#define PURCHASES_FILE  "purchases.dat"

/* ===== 图书数据文件操作 ===== */
/* 从文件加载图书链表，返回头节点指针 */
BookNode* load_books(void);
/* 将图书链表保存到文件 */
void save_books(BookNode *head);

/* ===== 借阅数据文件操作 ===== */
/* 从文件加载借阅记录链表 */
BorrowNode* load_borrows(void);
/* 将借阅记录链表保存到文件 */
void save_borrows(BorrowNode *head);

/* ===== 采购数据文件操作 ===== */
/* 从文件加载采购记录链表 */
PurchaseNode* load_purchases(void);
/* 将采购记录链表保存到文件 */
void save_purchases(PurchaseNode *head);

#endif /* FILEIO_H */
