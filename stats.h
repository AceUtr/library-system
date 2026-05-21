/**
 * stats.h — 统计分析模块
 *
 * 实现题目要求的全部统计功能 + 2个创新功能:
 *   (1) 馆藏书籍总数、已借出、在馆统计
 *   (2) 馆藏总金额、平均价格
 *   (3) 借阅次数最多的Top10图书
 *   (4) 每月逾期罚款统计 + 罚款最多Top5借阅者
 *   (5) 借书最多的Top5读者
 *   (6) 其他统计
 *
 * 创新功能:
 *   A) 图书智能推荐 — 基于图结构的共借分析
 *   B) 借阅热度分析 + 新书到馆提醒订阅
 */

#ifndef STATS_H
#define STATS_H

#include "types.h"

/* ===== 基本统计 ===== */
/* (1) 馆藏总数、已借出数、在馆数 */
void stats_count_books(void);
/* (2) 馆藏总金额 + 平均价格 */
void stats_total_value(void);

/* ===== 排行榜统计 (使用快速排序) ===== */
/* (3) 借阅次数最多的Top10图书 */
void stats_top_borrowed_books(int top_n);
/* (4) 每月逾期罚款总额 + 罚款最多Top5借阅者 */
void stats_top_fined_readers(int top_n);
/* 月度罚款总额 (按年月筛选) */
void stats_monthly_fine(const char *year_month);
/* (5) 借阅书籍最多Top5读者 */
void stats_top_borrowers(int top_n);

/* ===== 创新功能A: 图书推荐 (图数据结构) ===== */
/* 基于借阅记录构建图书共借图 */
BookGraph* stats_build_graph(void);
/* 根据书名推荐共借频率最高的N本书 */
void stats_recommend_books(const char *book_title, int top_n);

/* ===== 创新功能B: 新书提醒订阅 ===== */
/* 读者订阅分类通知 */
void stats_subscribe(const char *reader_name, const char *class_prefix);
/* 新书入库时通知相关订阅者 */
void stats_notify_subscribers(const char *class_id, const char *book_title);
/* 显示所有订阅 */
void stats_show_subscriptions(void);
/* 月度借阅趋势分析 */
void stats_monthly_trend(void);

#endif /* STATS_H */
