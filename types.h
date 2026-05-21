/**
 * types.h — 图书管理系统核心数据结构定义
 *
 * 本文件定义了系统所需的所有数据结构：
 * - 作者链表 (多个作者场景)
 * - 图书信息 + 双向链表 (主要存储结构)
 * - BST二叉搜索树 (按ISBN快速查找)
 * - 借阅记录链表 + 预约队列
 * - 采购记录链表
 * - 图邻接表 (图书推荐系统)
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ===== 常量定义 ===== */
#define MAX_NAME_LEN      64      /* 人名/书名最大长度 */
#define MAX_UNIT_LEN      128     /* 单位名称最大长度 */
#define MAX_PUB_LEN       128     /* 出版社名称最大长度 */
#define MAX_CLASS_LEN     32      /* 中图法分类号最大长度 */
#define MAX_ISBN_LEN      20      /* ISBN最大长度 */
#define MAX_ID_LEN        16      /* 图书编号最大长度 */
#define MAX_INVOICE_LEN   32      /* 发票号码最大长度 */
#define BORROW_DAYS       30      /* 借阅期限(天) */
#define FINE_PER_DAY      0.1     /* 逾期每天罚款(元) */

/* ===== 作者链表 ===== */
/* 支持一本书有多个作者 */
typedef struct AuthorNode {
    char name[MAX_NAME_LEN];          /* 作者姓名 */
    struct AuthorNode *next;          /* 指向下一位作者 */
} AuthorNode;

/* ===== 图书基本信息 ===== */
typedef struct {
    char class_id[MAX_CLASS_LEN];     /* 中图法分类号 */
    char book_id[MAX_ID_LEN];         /* 图书编号 (唯一) */
    char title[MAX_NAME_LEN];         /* 书名 */
    AuthorNode *authors;              /* 作者链表头指针 */
    int author_count;                 /* 作者数量 */
    char publisher[MAX_PUB_LEN];      /* 出版社 */
    char pub_date[16];                /* 出版日期 yyyy-mm-dd */
    char isbn[MAX_ISBN_LEN];          /* ISBN号 (唯一索引) */
    int edition;                      /* 版次 */
    double price;                     /* 定价 */
    int total_copies;                 /* 馆藏数 */
    int borrowed_count;               /* 已借出数量 */
} BookInfo;

/* ===== 图书链表节点 (双向链表) ===== */
typedef struct BookNode {
    BookInfo info;                    /* 图书信息 */
    struct BookNode *prev;            /* 前驱指针 */
    struct BookNode *next;            /* 后继指针 */
} BookNode;

/* ===== BST二叉搜索树节点 ===== */
/* 按ISBN建立索引，支持O(log n)查找 */
typedef struct BSTNode {
    BookNode *book;                   /* 指向对应图书链表节点 */
    struct BSTNode *left;             /* 左子树 */
    struct BSTNode *right;            /* 右子树 */
} BSTNode;

/* ===== 图书预约队列节点 ===== */
/* 当某书全部借出时，读者可排队预约 */
typedef struct ReserveNode {
    char reader_name[MAX_NAME_LEN];   /* 预约人姓名 */
    char card_id[MAX_ID_LEN];         /* 借书证号 */
    char book_id[MAX_ID_LEN];         /* 预约图书编号 */
    struct ReserveNode *next;         /* 队列下一节点 */
} ReserveNode;

/* ===== 预约队列 (FIFO) ===== */
typedef struct {
    ReserveNode *front;               /* 队首 */
    ReserveNode *rear;                /* 队尾 */
    int count;                        /* 队列长度 */
} ReserveQueue;

/* ===== 借阅记录 ===== */
typedef struct {
    char reader_name[MAX_NAME_LEN];   /* 借阅人姓名 */
    char unit[MAX_UNIT_LEN];          /* 借阅人所在单位 */
    char card_id[MAX_ID_LEN];         /* 借书证号 */
    char book_id[MAX_ID_LEN];         /* 所借图书编号 */
    char book_title[MAX_NAME_LEN];    /* 所借书名 */
    char borrow_date[16];             /* 借阅日期 yyyy-mm-dd */
    char return_date[16];             /* 归还日期，未还则为"--" */
    int is_returned;                  /* 是否已还 0-未还 1-已还 */
    double fine;                      /* 逾期罚款金额 */
} BorrowInfo;

/* ===== 借阅记录链表节点 ===== */
typedef struct BorrowNode {
    BorrowInfo info;                  /* 借阅信息 */
    struct BorrowNode *prev;
    struct BorrowNode *next;
} BorrowNode;

/* ===== 采购记录 ===== */
typedef struct {
    char book_title[MAX_NAME_LEN];    /* 书名 */
    char author[MAX_NAME_LEN];        /* 作者 (采购时只记第一作者) */
    char purchase_date[16];           /* 采购日期 */
    int quantity;                     /* 采购数量 */
    double unit_price;                /* 采购单价 */
    double total_amount;              /* 采购金额 */
    char invoice_num[MAX_INVOICE_LEN];/* 发票号码 */
    char book_id[MAX_ID_LEN];         /* 对应图书编号 */
} PurchaseInfo;

/* ===== 采购记录链表节点 ===== */
typedef struct PurchaseNode {
    PurchaseInfo info;
    struct PurchaseNode *prev;
    struct PurchaseNode *next;
} PurchaseNode;

/* ===== 图结构 (用于图书推荐) ===== */
/* 邻接表节点 */
typedef struct ArcNode {
    int book_index;                   /* 目标图书在数组中的下标 */
    int weight;                       /* 边权重 = 共借次数 */
    struct ArcNode *next_arc;         /* 下一条边 */
} ArcNode;

/* 顶点节点 */
typedef struct VNode {
    char book_id[MAX_ID_LEN];         /* 图书编号 */
    char title[MAX_NAME_LEN];         /* 书名 */
    ArcNode *first_arc;               /* 第一条边 */
} VNode;

/* 图结构 */
typedef struct {
    VNode *vertices;                  /* 顶点数组 */
    int vertex_count;                 /* 顶点数 */
    int capacity;                     /* 数组容量 */
} BookGraph;

/* ===== 新书提醒订阅队列 ===== */
/* 读者可订阅某个分类，该分类有新书入库时收到提醒 */
typedef struct SubNode {
    char reader_name[MAX_NAME_LEN];   /* 读者名 */
    char class_prefix[8];            /* 关注的分类号前缀 */
    struct SubNode *next;
} SubNode;

#endif /* TYPES_H */
