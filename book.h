/**
 * book.h — 图书管理模块
 *
 * 使用双向链表存储所有图书记录 (方便增删改遍历)
 * 使用BST按ISBN建立索引 (实现O(log n)快速查找)
 */

#ifndef BOOK_H
#define BOOK_H

#include "types.h"

/* ===== 全局变量声明 ===== */
extern BookNode *book_list_head;      /* 图书链表头 */
extern BookNode *book_list_tail;      /* 图书链表尾 */
extern BSTNode *bst_root;             /* BST根节点 */
extern int book_total_count;          /* 图书总数 */

/* ===== 初始化 ===== */
/* 初始化图书链表和BST，从文件加载数据 */
void book_init(void);

/* ===== BST操作 ===== */
/* 向BST插入节点 */
void bst_insert(BookNode *book);
/* 从BST中删除节点 */
void bst_remove(const char *isbn);
/* 在BST中按ISBN查找，返回图书节点指针 */
BookNode* bst_search(const char *isbn);

/* ===== 图书链表操作 ===== */
/* 添加新图书，同时插入链表和BST */
BookNode* book_add(BookInfo *info);
/* 根据图书编号删除图书 */
int book_delete(const char *book_id);
/* 修改图书信息 */
int book_modify(const char *book_id, BookInfo new_info);
/* 根据图书编号查找 (链表顺序查找) */
BookNode* book_find_by_id(const char *book_id);
/* 根据ISBN查找 (BST查找) */
BookNode* book_find_by_isbn(const char *isbn);
/* 根据书名模糊查找，返回匹配链表 */
BookNode** book_search_by_title(const char *keyword, int *count);
/* 根据作者查找 */
BookNode** book_search_by_author(const char *keyword, int *count);
/* 根据出版社查找 */
BookNode** book_search_by_publisher(const char *keyword, int *count);

/* ===== 辅助函数 ===== */
/* 创建作者链表 */
AuthorNode* author_create(const char *name);
/* 将作者链表追加到图书的作者列表 */
void author_append(BookInfo *info, const char *name);
/* 释放图书链表中的所有作者 */
void book_free_authors(BookNode *node);
/* 打印单本图书的详细信息 */
void book_print(BookNode *book);
/* 获取图书在馆数量 */
int book_available(BookNode *book);

#endif /* BOOK_H */
