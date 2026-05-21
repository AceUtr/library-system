/**
 * ui.c — 命令行用户界面实现
 *
 * 【设计说明】
 * 每个子菜单对应一个功能模块，菜单函数内部完成：
 *   1. 显示选项
 *   2. 读取用户输入
 *   3. 调用对应模块函数
 *   4. 显示结果
 * 输入验证由各模块函数内部完成。
 */

#include "ui.h"
#include "book.h"
#include "borrow.h"
#include "purchase.h"
#include "stats.h"

/* ===== 输入辅助函数 ===== */

void ui_clear_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void ui_read_line(char *buf, int size)
{
    fgets(buf, size, stdin);
    buf[strcspn(buf, "\n")] = '\0';
    /* 如果输入超过size，清除剩余缓冲区 */
    if (strlen(buf) == (size_t)(size - 1)) {
        ui_clear_input();
    }
}

void ui_pause(void)
{
    int c;
    printf("\n  [按回车键返回上级菜单...]");
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ===== 主菜单 ===== */

void ui_main_menu(void)
{
    int choice;
    do {
        printf("\n");
        printf("  ╔══════════════════════════════════════════╗\n");
        printf("  ║     图书资料信息管理系统  v1.0           ║\n");
        printf("  ╠══════════════════════════════════════════╣\n");
        printf("  ║  1. 图书信息管理                        ║\n");
        printf("  ║  2. 图书借阅管理                        ║\n");
        printf("  ║  3. 图书采购管理                        ║\n");
        printf("  ║  4. 统计与查询                          ║\n");
        printf("  ║  5. 智能推荐 (借过此书的还借过...)      ║\n");
        printf("  ║  6. 新书提醒订阅                        ║\n");
        printf("  ║  0. 退出系统                            ║\n");
        printf("  ╚══════════════════════════════════════════╝\n");
        printf("  请选择 [0-6]: ");
        scanf("%d", &choice);
        ui_clear_input();

        switch (choice) {
            case 1: ui_book_menu(); break;
            case 2: ui_borrow_menu(); break;
            case 3: ui_purchase_menu(); break;
            case 4: ui_stats_menu(); break;
            case 5: {
                char title[MAX_NAME_LEN];
                printf("  请输入书名或图书编号: ");
                ui_read_line(title, sizeof(title));
                stats_recommend_books(title, 5);
                ui_pause();
                break;
            }
            case 6: {
                char name[MAX_NAME_LEN], prefix[8];
                printf("  请输入您的姓名: ");
                ui_read_line(name, sizeof(name));
                printf("  请输入关注的分类号前缀 (如 TP): ");
                ui_read_line(prefix, sizeof(prefix));
                stats_subscribe(name, prefix);
                ui_pause();
                break;
            }
            case 0:
                printf("  感谢使用，再见！\n");
                break;
            default:
                printf("  [错误] 无效选项，请重新选择。\n");
        }
    } while (choice != 0);
}

/* ===== 图书管理子菜单 ===== */

void ui_book_menu(void)
{
    int choice;
    do {
        printf("\n");
        printf("  ┌──────────────────────────────────────────┐\n");
        printf("  │         图书信息管理                      │\n");
        printf("  ├──────────────────────────────────────────┤\n");
        printf("  │  1. 添加新图书                           │\n");
        printf("  │  2. 修改图书信息                         │\n");
        printf("  │  3. 删除图书                             │\n");
        printf("  │  4. 按编号查询图书                       │\n");
        printf("  │  5. 按ISBN查询图书 (BST快速查找)         │\n");
        printf("  │  6. 按书名模糊查找                       │\n");
        printf("  │  7. 按作者查找                           │\n");
        printf("  │  8. 按出版社查找                         │\n");
        printf("  │  9. 显示全部图书                         │\n");
        printf("  │  0. 返回上级菜单                         │\n");
        printf("  └──────────────────────────────────────────┘\n");
        printf("  请选择 [0-9]: ");
        scanf("%d", &choice);
        ui_clear_input();

        switch (choice) {
            case 1: { /* 添加新图书 */
                BookInfo info;
                memset(&info, 0, sizeof(BookInfo));

                printf("  中图法分类号: "); ui_read_line(info.class_id, sizeof(info.class_id));
                printf("  图书编号: ");     ui_read_line(info.book_id, sizeof(info.book_id));
                printf("  书名: ");         ui_read_line(info.title, sizeof(info.title));

                /* 多作者输入 */
                info.authors = NULL;
                info.author_count = 0;
                printf("  作者 (多个作者请用逗号分隔): ");
                char authors_input[512];
                ui_read_line(authors_input, sizeof(authors_input));
                /* 按逗号拆分作者 */
                char *token = strtok(authors_input, ",");
                while (token) {
                    /* 去除首尾空格 */
                    while (*token == ' ') token++;
                    char *end = token + strlen(token) - 1;
                    while (end > token && *end == ' ') *end-- = '\0';
                    if (strlen(token) > 0)
                        author_append(&info, token);
                    token = strtok(NULL, ",");
                }

                printf("  出版社: ");       ui_read_line(info.publisher, sizeof(info.publisher));
                printf("  出版日期(yyyy-mm-dd): "); ui_read_line(info.pub_date, sizeof(info.pub_date));
                printf("  ISBN: ");         ui_read_line(info.isbn, sizeof(info.isbn));
                printf("  版次: ");         scanf("%d", &info.edition); ui_clear_input();
                printf("  定价: ");         scanf("%lf", &info.price); ui_clear_input();
                printf("  馆藏数: ");       scanf("%d", &info.total_copies); ui_clear_input();
                info.borrowed_count = 0;

                book_add(&info);
                /* 新书入库通知订阅者 */
                stats_notify_subscribers(info.class_id, info.title);
                break;
            }
            case 2: { /* 修改图书信息 */
                char book_id[MAX_ID_LEN];
                printf("  请输入要修改的图书编号: ");
                ui_read_line(book_id, sizeof(book_id));

                BookNode *old = book_find_by_id(book_id);
                if (!old) {
                    printf("[错误] 未找到该图书。\n");
                    break;
                }
                printf("  (直接回车保留原值)\n");

                BookInfo info = old->info;
                char input[512];

                printf("  中图法分类号 [%s]: ", info.class_id);
                ui_read_line(input, sizeof(input));
                if (strlen(input) > 0) strcpy(info.class_id, input);

                printf("  书名 [%s]: ", info.title);
                ui_read_line(input, sizeof(input));
                if (strlen(input) > 0) strcpy(info.title, input);

                printf("  作者 [当前%d位作者]: ", info.author_count);
                ui_read_line(input, sizeof(input));
                if (strlen(input) > 0) {
                    /* 释放旧作者链表 */
                    AuthorNode *a = info.authors;
                    while (a) { AuthorNode *tmp = a; a = a->next; free(tmp); }
                    info.authors = NULL;
                    info.author_count = 0;
                    char *token = strtok(input, ",");
                    while (token) {
                        while (*token == ' ') token++;
                        char *end = token + strlen(token) - 1;
                        while (end > token && *end == ' ') *end-- = '\0';
                        if (strlen(token) > 0)
                            author_append(&info, token);
                        token = strtok(NULL, ",");
                    }
                }

                printf("  出版社 [%s]: ", info.publisher);
                ui_read_line(input, sizeof(input));
                if (strlen(input) > 0) strcpy(info.publisher, input);

                printf("  出版日期 [%s]: ", info.pub_date);
                ui_read_line(input, sizeof(input));
                if (strlen(input) > 0) strcpy(info.pub_date, input);

                printf("  ISBN [%s]: ", info.isbn);
                ui_read_line(input, sizeof(input));
                if (strlen(input) > 0) strcpy(info.isbn, input);

                printf("  版次 [%d]: ", info.edition);
                ui_read_line(input, sizeof(input));
                if (strlen(input) > 0) info.edition = atoi(input);

                printf("  定价 [%.2f]: ", info.price);
                ui_read_line(input, sizeof(input));
                if (strlen(input) > 0) info.price = atof(input);

                printf("  馆藏数 [%d]: ", info.total_copies);
                ui_read_line(input, sizeof(input));
                if (strlen(input) > 0) info.total_copies = atoi(input);

                book_modify(book_id, info);
                break;
            }
            case 3: { /* 删除图书 */
                char book_id[MAX_ID_LEN];
                printf("  请输入要删除的图书编号: ");
                ui_read_line(book_id, sizeof(book_id));
                printf("  确认删除? [y/N]: ");
                char confirm;
                scanf("%c", &confirm);
                ui_clear_input();
                if (confirm == 'y' || confirm == 'Y')
                    book_delete(book_id);
                break;
            }
            case 4: { /* 按编号查询 */
                char book_id[MAX_ID_LEN];
                printf("  请输入图书编号: ");
                ui_read_line(book_id, sizeof(book_id));
                book_print(book_find_by_id(book_id));
                ui_pause();
                break;
            }
            case 5: { /* 按ISBN查询 */
                char isbn[MAX_ISBN_LEN];
                printf("  请输入ISBN: ");
                ui_read_line(isbn, sizeof(isbn));
                book_print(book_find_by_isbn(isbn));
                ui_pause();
                break;
            }
            case 6: { /* 按书名模糊查找 */
                char keyword[MAX_NAME_LEN];
                printf("  请输入书名关键词: ");
                ui_read_line(keyword, sizeof(keyword));
                int count;
                BookNode **results = book_search_by_title(keyword, &count);
                if (results) {
                    printf("[结果] 找到 %d 本匹配图书:\n", count);
                    for (int i = 0; i < count; i++)
                        book_print(results[i]);
                    free(results);
                } else {
                    printf("[结果] 未找到匹配图书。\n");
                }
                ui_pause();
                break;
            }
            case 7: { /* 按作者查找 */
                char keyword[MAX_NAME_LEN];
                printf("  请输入作者关键词: ");
                ui_read_line(keyword, sizeof(keyword));
                int count;
                BookNode **results = book_search_by_author(keyword, &count);
                if (results) {
                    printf("[结果] 找到 %d 本匹配图书:\n", count);
                    for (int i = 0; i < count; i++)
                        book_print(results[i]);
                    free(results);
                } else {
                    printf("[结果] 未找到匹配图书。\n");
                }
                ui_pause();
                break;
            }
            case 8: { /* 按出版社查找 */
                char keyword[MAX_NAME_LEN];
                printf("  请输入出版社关键词: ");
                ui_read_line(keyword, sizeof(keyword));
                int count;
                BookNode **results = book_search_by_publisher(keyword, &count);
                if (results) {
                    printf("[结果] 找到 %d 本匹配图书:\n", count);
                    for (int i = 0; i < count; i++)
                        book_print(results[i]);
                    free(results);
                } else {
                    printf("[结果] 未找到匹配图书。\n");
                }
                ui_pause();
                break;
            }
            case 9: { /* 显示全部图书 */
                if (!book_list_head) {
                    printf("[提示] 暂无图书记录。\n");
                } else {
                    BookNode *p = book_list_head;
                    printf("\n  共 %d 本图书:\n", book_total_count);
                    while (p) {
                        printf("  [%s] %s - ", p->info.book_id, p->info.title);
                        AuthorNode *a = p->info.authors;
                        if (a) printf("%s", a->name);
                        printf(" | 在馆:%d/%d\n", book_available(p), p->info.total_copies);
                        p = p->next;
                    }
                }
                ui_pause();
                break;
            }
            case 0: break;
            default: printf("  [错误] 无效选项。\n");
        }
    } while (choice != 0);
}

/* ===== 借阅管理子菜单 ===== */

void ui_borrow_menu(void)
{
    int choice;
    do {
        printf("\n");
        printf("  ┌──────────────────────────────────────────┐\n");
        printf("  │         图书借阅管理                      │\n");
        printf("  ├──────────────────────────────────────────┤\n");
        printf("  │  1. 借阅图书                             │\n");
        printf("  │  2. 归还图书                             │\n");
        printf("  │  3. 预约图书 (书已借出时)                │\n");
        printf("  │  4. 查看预约队列                         │\n");
        printf("  │  5. 查询借阅记录 (按借书证号)            │\n");
        printf("  │  6. 显示全部借阅记录                     │\n");
        printf("  │  0. 返回上级菜单                         │\n");
        printf("  └──────────────────────────────────────────┘\n");
        printf("  请选择 [0-6]: ");
        scanf("%d", &choice);
        ui_clear_input();

        switch (choice) {
            case 1: { /* 借阅 */
                char card_id[MAX_ID_LEN], name[MAX_NAME_LEN], unit[MAX_UNIT_LEN], book_id[MAX_ID_LEN];
                printf("  借书证号: "); ui_read_line(card_id, sizeof(card_id));
                printf("  借阅人姓名: "); ui_read_line(name, sizeof(name));
                printf("  所在单位: "); ui_read_line(unit, sizeof(unit));
                printf("  图书编号: "); ui_read_line(book_id, sizeof(book_id));
                borrow_book(card_id, name, unit, book_id);
                break;
            }
            case 2: { /* 归还 */
                char card_id[MAX_ID_LEN], book_id[MAX_ID_LEN];
                printf("  借书证号: "); ui_read_line(card_id, sizeof(card_id));
                printf("  图书编号: "); ui_read_line(book_id, sizeof(book_id));
                return_book(card_id, book_id);
                break;
            }
            case 3: { /* 预约 */
                char name[MAX_NAME_LEN], card_id[MAX_ID_LEN], book_id[MAX_ID_LEN];
                printf("  借书证号: "); ui_read_line(card_id, sizeof(card_id));
                printf("  预约人姓名: "); ui_read_line(name, sizeof(name));
                printf("  图书编号: "); ui_read_line(book_id, sizeof(book_id));
                reserve_enqueue(name, card_id, book_id);
                break;
            }
            case 4: /* 查看预约队列 */
                reserve_display();
                ui_pause();
                break;
            case 5: { /* 查借阅记录 */
                char card_id[MAX_ID_LEN];
                printf("  请输入借书证号: ");
                ui_read_line(card_id, sizeof(card_id));
                int count;
                BorrowNode **results = borrow_find_by_card(card_id, &count);
                if (results) {
                    printf("[结果] 找到 %d 条借阅记录:\n", count);
                    for (int i = 0; i < count; i++)
                        borrow_print(results[i]);
                    free(results);
                } else {
                    printf("[结果] 未找到该读者的借阅记录。\n");
                }
                ui_pause();
                break;
            }
            case 6: /* 全部借阅记录 */
                borrow_list_all();
                ui_pause();
                break;
            case 0: break;
            default: printf("  [错误] 无效选项。\n");
        }
    } while (choice != 0);
}

/* ===== 采购管理子菜单 ===== */

void ui_purchase_menu(void)
{
    int choice;
    do {
        printf("\n");
        printf("  ┌──────────────────────────────────────────┐\n");
        printf("  │         图书采购管理                      │\n");
        printf("  ├──────────────────────────────────────────┤\n");
        printf("  │  1. 添加采购记录                         │\n");
        printf("  │  2. 查看全部采购记录                     │\n");
        printf("  │  3. 按发票号查询                         │\n");
        printf("  │  0. 返回上级菜单                         │\n");
        printf("  └──────────────────────────────────────────┘\n");
        printf("  请选择 [0-3]: ");
        scanf("%d", &choice);
        ui_clear_input();

        switch (choice) {
            case 1: { /* 添加采购记录 */
                PurchaseInfo info;
                memset(&info, 0, sizeof(PurchaseInfo));
                printf("  书名: ");           ui_read_line(info.book_title, sizeof(info.book_title));
                printf("  作者: ");           ui_read_line(info.author, sizeof(info.author));
                printf("  采购日期: ");       ui_read_line(info.purchase_date, sizeof(info.purchase_date));
                printf("  采购数量: ");       scanf("%d", &info.quantity); ui_clear_input();
                printf("  采购单价: ");       scanf("%lf", &info.unit_price); ui_clear_input();
                printf("  发票号码: ");       ui_read_line(info.invoice_num, sizeof(info.invoice_num));
                printf("  对应图书编号: ");   ui_read_line(info.book_id, sizeof(info.book_id));
                purchase_add(info);
                break;
            }
            case 2: /* 查看全部采购记录 */
                purchase_list_all();
                ui_pause();
                break;
            case 3: { /* 按发票号查询 */
                char invoice[MAX_INVOICE_LEN];
                printf("  请输入发票号码: ");
                ui_read_line(invoice, sizeof(invoice));
                PurchaseNode *p = purchase_find_by_invoice(invoice);
                if (p) {
                    printf("\n  发票号: %s\n", p->info.invoice_num);
                    printf("  书名: %s\n", p->info.book_title);
                    printf("  采购日期: %s\n", p->info.purchase_date);
                    printf("  数量: %d, 单价: ¥%.2f, 金额: ¥%.2f\n",
                           p->info.quantity, p->info.unit_price, p->info.total_amount);
                } else {
                    printf("[结果] 未找到该采购记录。\n");
                }
                ui_pause();
                break;
            }
            case 0: break;
            default: printf("  [错误] 无效选项。\n");
        }
    } while (choice != 0);
}

/* ===== 统计查询子菜单 ===== */

void ui_stats_menu(void)
{
    int choice;
    do {
        printf("\n");
        printf("  ┌──────────────────────────────────────────┐\n");
        printf("  │         统计与查询                        │\n");
        printf("  ├──────────────────────────────────────────┤\n");
        printf("  │  (1) 馆藏总览                            │\n");
        printf("  │  (2) 馆藏价值统计                        │\n");
        printf("  │  (3) 借阅排行榜 TOP10                    │\n");
        printf("  │  (4) 罚款统计 + 罚款排行 TOP5            │\n");
        printf("  │  (5) 借阅达人 TOP5                       │\n");
        printf("  │  (6) 月度借阅趋势分析                    │\n");
        printf("  │  (7) 采购总支出统计                      │\n");
        printf("  │  0. 返回上级菜单                         │\n");
        printf("  └──────────────────────────────────────────┘\n");
        printf("  请选择 [0-7]: ");
        scanf("%d", &choice);
        ui_clear_input();

        switch (choice) {
            case 1: stats_count_books(); ui_pause(); break;
            case 2: stats_total_value(); ui_pause(); break;
            case 3: stats_top_borrowed_books(10); ui_pause(); break;
            case 4:
                stats_top_fined_readers(5);
                {
                    char ym[8];
                    printf("\n  查看月度罚款? 输入年月(yyyy-mm)或直接回车跳过: ");
                    ui_read_line(ym, sizeof(ym));
                    if (strlen(ym) >= 7) stats_monthly_fine(ym);
                }
                ui_pause();
                break;
            case 5: stats_top_borrowers(5); ui_pause(); break;
            case 6: stats_monthly_trend(); ui_pause(); break;
            case 7:
                printf("\n  采购总支出: ¥%.2f\n", purchase_total_spent());
                ui_pause();
                break;
            case 0: break;
            default: printf("  [错误] 无效选项。\n");
        }
    } while (choice != 0);
}
