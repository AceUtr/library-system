/**
 * ui.h — 命令行用户界面模块
 *
 * 负责所有菜单显示和用户输入处理
 * 作为各功能模块与用户之间的"控制器"
 */

#ifndef UI_H
#define UI_H

#include "types.h"

/* ===== 主菜单 ===== */
void ui_main_menu(void);

/* ===== 图书管理子菜单 ===== */
void ui_book_menu(void);

/* ===== 借阅管理子菜单 ===== */
void ui_borrow_menu(void);

/* ===== 采购管理子菜单 ===== */
void ui_purchase_menu(void);

/* ===== 统计查询子菜单 ===== */
void ui_stats_menu(void);

/* ===== 输入辅助函数 ===== */
/* 清除输入缓冲区 */
void ui_clear_input(void);
/* 读取一行字符串 (去除换行符) */
void ui_read_line(char *buf, int size);
/* 暂停等待用户按回车继续 */
void ui_pause(void);

#endif /* UI_H */
