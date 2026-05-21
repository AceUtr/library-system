/**
 * main.c — 图书资料信息管理系统 主程序入口
 *
 * 【系统架构】
 * 本系统采用模块化设计，各模块职责明确：
 *   types.h   — 全部数据结构定义 (链表、队列、树、图等)
 *   book.c    — 图书管理 (双向链表 + BST索引)
 *   borrow.c  — 借阅管理 (链表 + FIFO预约队列)
 *   purchase.c — 采购管理 (链表)
 *   stats.c   — 统计分析 (快速排序 + 图推荐)
 *   fileio.c  — 文件持久化 (/分割文本格式)
 *   ui.c      — 命令行界面 (多级菜单)
 *
 * 【数据流】
 *   用户输入 → UI模块 → 业务模块 → 文件存储
 *   文件存储 → 初始化加载 → 内存数据结构 → UI展示
 *
 * 【数据结构总览】
 *   链表: BookNode (图书列表), BorrowNode (借阅记录), PurchaseNode (采购记录)
 *   BST:  按ISBN索引，O(log n)查找
 *   队列: ReserveQueue (预约排队FIFO)
 *   图:   BookGraph (共借关系图-推荐系统)
 *   栈:   (隐含于递归调用中: BST操作、快速排序、图的DFS)
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <windows.h>

#include "types.h"
#include "book.h"
#include "borrow.h"
#include "purchase.h"
#include "stats.h"
#include "ui.h"
#include "fileio.h"

int main(void)
{
    /* 将工作目录切换到exe所在目录，确保数据文件与程序在同一位置 */
    {
        char exe_path[MAX_PATH];
        GetModuleFileName(NULL, exe_path, MAX_PATH);
        char *last_slash = strrchr(exe_path, '\\');
        if (last_slash) {
            *last_slash = '\0';
            SetCurrentDirectory(exe_path);
        }
    }

    /* 设置控制台编码为UTF-8，解决中文乱码 */
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    setlocale(LC_ALL, ".utf8");

    /* ===== 系统启动欢迎信息 ===== */
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════╗\n");
    printf("  ║                                                          ║\n");
    printf("  ║       图 书 资 料 信 息 管 理 系 统                     ║\n");
    printf("  ║       Library Information Management System              ║\n");
    printf("  ║                                                          ║\n");
    printf("  ║  数据结构: 链表 | 栈 | 队列 | 树(BST) | 图(邻接表)     ║\n");
    printf("  ║  算法: 快速排序 | BST查找 | 顺序查找 | 回溯递归        ║\n");
    printf("  ║  特色: 智能推荐 | 预约排队 | 新书提醒                  ║\n");
    printf("  ║                                                          ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n\n");

    /* ===== 初始化各模块 (从文件加载数据) ===== */
    printf("  [系统] 正在初始化...\n");
    book_init();
    borrow_init();
    purchase_init();
    printf("  [系统] 初始化完成。\n");

    /* ===== 进入主菜单 ===== */
    ui_main_menu();

    /* ===== 退出前保存数据 ===== */
    printf("\n  [系统] 正在保存数据...\n");
    save_books(book_list_head);
    save_borrows(borrow_list_head);
    save_purchases(purchase_list_head);
    printf("  [系统] 数据已保存，系统退出。\n");

    return 0;
}
