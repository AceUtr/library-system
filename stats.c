/**
 * stats.c — 统计分析模块实现
 *
 * 【排序算法说明】
 * 使用快速排序 (Quick Sort) 进行Top N排行:
 *   - 平均时间复杂度: O(n log n)
 *   - 空间复杂度: O(log n) (递归栈)
 *   - 对每种排行定义比较函数，传入qsort标准库
 *
 * 【图算法说明 — 创新功能A】
 * 构建"图书共借图":
 *   - 顶点: 每本有过借阅记录的图书
 *   - 边: 若两本书被同一读者借阅过，则存在一条边
 *   - 权重: 共借次数 (被同一读者借阅的次数累加)
 * 推荐: 对于目标图书，找到权重最大的5条边 = 推荐Top5
 *
 * 【订阅队列 — 创新功能B】
 * 读者可订阅"分类号"前缀，当新书入库时遍历订阅队列通知匹配者
 */

#include "stats.h"
#include "book.h"
#include "borrow.h"

/* ===== 排序辅助结构 ===== */

/* 用于图书借阅次数排序 */
typedef struct {
    BookNode *book;
    int count;
} BookRank;

/* 用于读者罚款/借阅次数排序 */
typedef struct {
    char reader_name[MAX_NAME_LEN];
    char card_id[MAX_ID_LEN];
    double total;
} ReaderRank;

/* 用于图构建的读者去重 */
typedef struct {
    char card_id[MAX_ID_LEN];
} ReaderInfo;

/* 用于推荐排序的边权重 */
typedef struct {
    int idx;
    int weight;
} EdgeRank;

/* 用于月度趋势统计 */
typedef struct {
    char month[8];
    int count;
} MonthCount;

/* ===== 快速排序的比较函数 ===== */

/* 按借阅次数降序 */
static int cmp_book_rank_desc(const void *a, const void *b)
{
    const BookRank *ra = (const BookRank*)a;
    const BookRank *rb = (const BookRank*)b;
    return rb->count - ra->count;
}

/* 按罚款金额降序 */
static int cmp_reader_fine_desc(const void *a, const void *b)
{
    const ReaderRank *ra = (const ReaderRank*)a;
    const ReaderRank *rb = (const ReaderRank*)b;
    if (rb->total > ra->total) return 1;
    if (rb->total < ra->total) return -1;
    return 0;
}

/* 按借阅次数降序 */
static int cmp_reader_count_desc(const void *a, const void *b)
{
    return cmp_reader_fine_desc(a, b); /* 复用同样的降序逻辑 */
}

/* ===== (1) 馆藏总数/已借出/在馆 ===== */

void stats_count_books(void)
{
    int total_books = 0;      /* 馆藏总册数 */
    int total_borrowed = 0;   /* 已借出册数 */
    int total_titles = 0;     /* 图书种类数 */

    BookNode *p = book_list_head;
    while (p) {
        total_books += p->info.total_copies;
        total_borrowed += p->info.borrowed_count;
        total_titles++;
        p = p->next;
    }

    printf("\n  ╔══════════════════════════════════╗\n");
    printf("  ║      馆藏统计概览                ║\n");
    printf("  ╠══════════════════════════════════╣\n");
    printf("  ║  图书种类 (书目数): %8d 种   ║\n", total_titles);
    printf("  ║  馆藏总册数:        %8d 册   ║\n", total_books);
    printf("  ║  已借出册数:        %8d 册   ║\n", total_borrowed);
    printf("  ║  在馆册数:          %8d 册   ║\n", total_books - total_borrowed);
    printf("  ║  借出率:            %7.1f%%      ║\n",
           total_books > 0 ? 100.0 * total_borrowed / total_books : 0);
    printf("  ╚══════════════════════════════════╝\n");
}

/* ===== (2) 馆藏总金额 + 平均价格 ===== */

void stats_total_value(void)
{
    double total_value = 0.0;
    int total_copies = 0;
    int title_count = 0;

    BookNode *p = book_list_head;
    while (p) {
        total_value += p->info.price * p->info.total_copies;
        total_copies += p->info.total_copies;
        title_count++;
        p = p->next;
    }

    printf("\n  ╔══════════════════════════════════╗\n");
    printf("  ║      馆藏价值统计                ║\n");
    printf("  ╠══════════════════════════════════╣\n");
    printf("  ║  图书种类:       %8d 种      ║\n", title_count);
    printf("  ║  馆藏总册数:     %8d 册      ║\n", total_copies);
    printf("  ║  馆藏总金额:   ¥%10.2f       ║\n", total_value);
    printf("  ║  单册平均价格: ¥%10.2f       ║\n",
           total_copies > 0 ? total_value / total_copies : 0);
    printf("  ╚══════════════════════════════════╝\n");
}

/* ===== (3) 借阅次数最多Top N 图书 ===== */

void stats_top_borrowed_books(int top_n)
{
    if (!book_list_head) {
        printf("[提示] 无图书记录。\n");
        return;
    }

    /* 收集所有图书的借阅数据 */
    int total = book_total_count;
    BookRank *ranks = (BookRank*)malloc(sizeof(BookRank) * total);
    BookNode *p = book_list_head;
    int i = 0;
    while (p) {
        ranks[i].book = p;
        /* 统计历史借阅总次数 (当前借出 + 历史归还) */
        int hist_count = p->info.borrowed_count;
        /* 扫描借阅记录统计历史借阅数 */
        BorrowNode *bp = borrow_list_head;
        while (bp) {
            if (strcmp(bp->info.book_id, p->info.book_id) == 0)
                hist_count++;
            bp = bp->next;
        }
        ranks[i].count = hist_count;
        i++;
        p = p->next;
    }

    /* 快速排序 (降序) */
    qsort(ranks, total, sizeof(BookRank), cmp_book_rank_desc);

    int n = (top_n < total) ? top_n : total;
    printf("\n  ╔══════════════════════════════════════════════════════════╗\n");
    printf("  ║       借阅次数最多 TOP %-2d 图书                         ║\n", n);
    printf("  ╠══════════════════════════════════════════════════════════╣\n");
    printf("  ║ 排名  书名                作者           借阅次数        ║\n");
    printf("  ╠══════════════════════════════════════════════════════════╣\n");
    for (i = 0; i < n; i++) {
        char author_display[20] = "";
        if (ranks[i].book->info.authors)
            strncpy(author_display, ranks[i].book->info.authors->name, 18);
        else
            strcpy(author_display, "未知");
        printf("  ║ %-4d  %-18s  %-14s  %-10d     ║\n",
               i + 1, ranks[i].book->info.title, author_display, ranks[i].count);
    }
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    free(ranks);
}

/* ===== (4) 罚款统计 ===== */

void stats_monthly_fine(const char *year_month)
{
    /* year_month 格式: "yyyy-mm" */
    double total = 0.0;
    int count = 0;

    BorrowNode *p = borrow_list_head;
    while (p) {
        /* 判断归还日期是否匹配月份 */
        if (p->info.is_returned && strncmp(p->info.return_date, year_month, 7) == 0) {
            total += p->info.fine;
            if (p->info.fine > 0) count++;
        }
        p = p->next;
    }
    printf("\n  %s 月统计: 逾期罚款 %d 笔，总金额 ¥%.2f\n", year_month, count, total);
}

void stats_top_fined_readers(int top_n)
{
    if (!borrow_list_head) {
        printf("[提示] 无借阅记录。\n");
        return;
    }

    /* 收集所有读者的罚款总计 */
    int capacity = 100;
    ReaderRank *ranks = (ReaderRank*)malloc(sizeof(ReaderRank) * capacity);
    memset(ranks, 0, sizeof(ReaderRank) * capacity);
    int rank_count = 0;

    BorrowNode *p = borrow_list_head;
    while (p) {
        if (p->info.fine > 0) {
            /* 查找是否已有此读者 */
            int found = -1;
            for (int i = 0; i < rank_count; i++) {
                if (strcmp(ranks[i].card_id, p->info.card_id) == 0) {
                    found = i;
                    break;
                }
            }
            if (found >= 0) {
                ranks[found].total += p->info.fine;
            } else {
                if (rank_count >= capacity) {
                    capacity *= 2;
                    ranks = (ReaderRank*)realloc(ranks, sizeof(ReaderRank) * capacity);
                    memset(ranks + rank_count, 0, sizeof(ReaderRank) * (capacity - rank_count));
                }
                strcpy(ranks[rank_count].reader_name, p->info.reader_name);
                strcpy(ranks[rank_count].card_id, p->info.card_id);
                ranks[rank_count].total = p->info.fine;
                rank_count++;
            }
        }
        p = p->next;
    }

    /* 快速排序 (罚款降序) */
    qsort(ranks, rank_count, sizeof(ReaderRank), cmp_reader_fine_desc);

    int n = (top_n < rank_count) ? top_n : rank_count;
    printf("\n  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║     罚款金额最多 TOP %-2d 借阅者                  ║\n", n);
    printf("  ╠══════════════════════════════════════════════════╣\n");
    printf("  ║ 排名  读者          借书证号      罚款总额        ║\n");
    printf("  ╠══════════════════════════════════════════════════╣\n");
    for (int i = 0; i < n; i++) {
        printf("  ║ %-4d  %-12s  %-12s  ¥%-10.2f     ║\n",
               i + 1, ranks[i].reader_name, ranks[i].card_id, ranks[i].total);
    }
    printf("  ╚══════════════════════════════════════════════════╝\n");
    free(ranks);
}

/* ===== (5) 借书最多 Top N 读者 ===== */

void stats_top_borrowers(int top_n)
{
    if (!borrow_list_head) {
        printf("[提示] 无借阅记录。\n");
        return;
    }

    int capacity = 100;
    ReaderRank *ranks = (ReaderRank*)malloc(sizeof(ReaderRank) * capacity);
    memset(ranks, 0, sizeof(ReaderRank) * capacity);
    int rank_count = 0;

    BorrowNode *p = borrow_list_head;
    while (p) {
        /* 查找是否已有此读者 */
        int found = -1;
        for (int i = 0; i < rank_count; i++) {
            if (strcmp(ranks[i].card_id, p->info.card_id) == 0) {
                found = i;
                break;
            }
        }
        if (found >= 0) {
            ranks[found].total += 1.0;  /* 用total字段统计次数 */
        } else {
            if (rank_count >= capacity) {
                capacity *= 2;
                ranks = (ReaderRank*)realloc(ranks, sizeof(ReaderRank) * capacity);
                memset(ranks + rank_count, 0, sizeof(ReaderRank) * (capacity - rank_count));
            }
            strcpy(ranks[rank_count].reader_name, p->info.reader_name);
            strcpy(ranks[rank_count].card_id, p->info.card_id);
            ranks[rank_count].total = 1.0;
            rank_count++;
        }
        p = p->next;
    }

    /* 快速排序 (次数降序) */
    qsort(ranks, rank_count, sizeof(ReaderRank), cmp_reader_count_desc);

    int n = (top_n < rank_count) ? top_n : rank_count;
    printf("\n  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║     借阅书籍最多 TOP %-2d 读者                    ║\n", n);
    printf("  ╠══════════════════════════════════════════════════╣\n");
    printf("  ║ 排名  读者          借书证号      借阅次数        ║\n");
    printf("  ╠══════════════════════════════════════════════════╣\n");
    for (int i = 0; i < n; i++) {
        printf("  ║ %-4d  %-12s  %-12s  %-10.0f         ║\n",
               i + 1, ranks[i].reader_name, ranks[i].card_id, ranks[i].total);
    }
    printf("  ╚══════════════════════════════════════════════════╝\n");
    free(ranks);
}

/* ===== 创新功能A: 图书智能推荐 (图数据结构) ===== */

BookGraph* stats_build_graph(void)
{
    BookGraph *graph = (BookGraph*)malloc(sizeof(BookGraph));
    graph->vertex_count = 0;
    graph->capacity = 64;
    graph->vertices = (VNode*)malloc(sizeof(VNode) * graph->capacity);

    if (!borrow_list_head) {
        printf("[提示] 无借阅数据，无法构建推荐图。\n");
        return graph;
    }

    /* 第一步: 收集所有被借阅过的图书作为图的顶点 */
    BorrowNode *p = borrow_list_head;
    while (p) {
        /* 检查此图书是否已添加为顶点 */
        int found = 0;
        for (int i = 0; i < graph->vertex_count; i++) {
            if (strcmp(graph->vertices[i].book_id, p->info.book_id) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            /* 扩容 */
            if (graph->vertex_count >= graph->capacity) {
                graph->capacity *= 2;
                graph->vertices = (VNode*)realloc(graph->vertices,
                    sizeof(VNode) * graph->capacity);
            }
            VNode *v = &graph->vertices[graph->vertex_count];
            strcpy(v->book_id, p->info.book_id);
            strcpy(v->title, p->info.book_title);
            v->first_arc = NULL;
            graph->vertex_count++;
        }
        p = p->next;
    }

    if (graph->vertex_count == 0) {
        printf("[提示] 借阅数据中无有效图书记录。\n");
        return graph;
    }

    /* 第二步: 构建共借边 (同一读者借过的书两两相连) */
    /* 遍历每个读者的借阅记录，建立共借关系 */
    /* 使用邻接矩阵的稀疏版本：只存储存在的边 */

    /* 收集所有不同读者 */
    ReaderInfo *readers = NULL;
    int reader_count = 0;
    int reader_cap = 0;

    p = borrow_list_head;
    while (p) {
        int found = 0;
        for (int i = 0; i < reader_count; i++) {
            if (strcmp(readers[i].card_id, p->info.card_id) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            if (reader_count >= reader_cap) {
                reader_cap = (reader_cap == 0) ? 32 : reader_cap * 2;
                readers = (ReaderInfo*)realloc(readers, sizeof(ReaderInfo) * reader_cap);
            }
            strcpy(readers[reader_count].card_id, p->info.card_id);
            reader_count++;
        }
        p = p->next;
    }

    /* 对每个读者，找到他借过的所有书，两两加边 */
    for (int r = 0; r < reader_count; r++) {
        /* 收集该读者借过的所有书的顶点下标 */
        int *book_indices = NULL;
        int idx_count = 0;

        BorrowNode *bp = borrow_list_head;
        while (bp) {
            if (strcmp(bp->info.card_id, readers[r].card_id) == 0) {
                /* 找到该书的顶点下标 */
                for (int v = 0; v < graph->vertex_count; v++) {
                    if (strcmp(graph->vertices[v].book_id, bp->info.book_id) == 0) {
                        book_indices = (int*)realloc(book_indices,
                            sizeof(int) * (idx_count + 1));
                        book_indices[idx_count++] = v;
                        break;
                    }
                }
            }
            bp = bp->next;
        }

        /* 该读者借过的书两两之间添加边+权重 */
        for (int a = 0; a < idx_count; a++) {
            for (int b = a + 1; b < idx_count; b++) {
                int vi = book_indices[a];
                int vj = book_indices[b];

                /* 在vi的邻接表中查找是否已有到vj的边 */
                ArcNode *arc = graph->vertices[vi].first_arc;
                int found = 0;
                while (arc) {
                    if (arc->book_index == vj) {
                        arc->weight++;
                        found = 1;
                        break;
                    }
                    arc = arc->next_arc;
                }
                if (!found) {
                    /* 添加新边 vi -> vj */
                    ArcNode *new_arc = (ArcNode*)malloc(sizeof(ArcNode));
                    new_arc->book_index = vj;
                    new_arc->weight = 1;
                    new_arc->next_arc = graph->vertices[vi].first_arc;
                    graph->vertices[vi].first_arc = new_arc;

                    /* 无向图: 添加反向边 vj -> vi */
                    ArcNode *rev_arc = (ArcNode*)malloc(sizeof(ArcNode));
                    rev_arc->book_index = vi;
                    rev_arc->weight = 1;
                    rev_arc->next_arc = graph->vertices[vj].first_arc;
                    graph->vertices[vj].first_arc = rev_arc;
                } else {
                    /* 同步更新反向边的权重 */
                    ArcNode *rev = graph->vertices[vj].first_arc;
                    while (rev) {
                        if (rev->book_index == vi) {
                            rev->weight = arc->weight;
                            break;
                        }
                        rev = rev->next_arc;
                    }
                }
            }
        }
        free(book_indices);
    }
    free(readers);

    printf("[图构建] 共 %d 个顶点，构建完成。\n", graph->vertex_count);
    return graph;
}

void stats_recommend_books(const char *book_title, int top_n)
{
    BookGraph *graph = stats_build_graph();
    if (graph->vertex_count == 0) {
        free(graph->vertices);
        free(graph);
        return;
    }

    /* 查找目标图书的顶点下标 */
    int target = -1;
    for (int i = 0; i < graph->vertex_count; i++) {
        if (strstr(graph->vertices[i].title, book_title) ||
            strcmp(graph->vertices[i].book_id, book_title) == 0) {
            target = i;
            break;
        }
    }
    if (target == -1) {
        printf("[提示] 未找到与《%s》相关的借阅数据。\n", book_title);
        /* 释放图 */
        for (int i = 0; i < graph->vertex_count; i++) {
            ArcNode *a = graph->vertices[i].first_arc;
            while (a) { ArcNode *tmp = a; a = a->next_arc; free(tmp); }
        }
        free(graph->vertices);
        free(graph);
        return;
    }

    printf("\n  ╔══════════════════════════════════════════════════════════╗\n");
    printf("  ║  《%s》的推荐阅读 (借过此书的人也借过):     ║\n",
           graph->vertices[target].title);
    printf("  ╠══════════════════════════════════════════════════════════╣\n");

    /* 收集目标书的邻接边，排序取Top N */
    /* 使用简单的冒泡排序 (因为单节点的邻接边通常不会太多) */
    EdgeRank *edges = NULL;
    int edge_count = 0;

    ArcNode *arc = graph->vertices[target].first_arc;
    while (arc) {
        edges = (EdgeRank*)realloc(edges, sizeof(EdgeRank) * (edge_count + 1));
        edges[edge_count].idx = arc->book_index;
        edges[edge_count].weight = arc->weight;
        edge_count++;
        arc = arc->next_arc;
    }

    /* 冒泡排序按权重降序 */
    for (int i = 0; i < edge_count - 1; i++) {
        for (int j = 0; j < edge_count - 1 - i; j++) {
            if (edges[j].weight < edges[j + 1].weight) {
                EdgeRank tmp = edges[j];
                edges[j] = edges[j + 1];
                edges[j + 1] = tmp;
            }
        }
    }

    int n = (top_n < edge_count) ? top_n : edge_count;
    printf("  ║ 排名  书名                      共借次数               ║\n");
    printf("  ╠══════════════════════════════════════════════════════════╣\n");
    for (int i = 0; i < n; i++) {
        printf("  ║ %-4d  %-24s  %-16d     ║\n",
               i + 1,
               graph->vertices[edges[i].idx].title,
               edges[i].weight);
    }
    if (edge_count == 0) {
        printf("  ║         暂无推荐数据                                     ║\n");
    }
    printf("  ╚══════════════════════════════════════════════════════════╝\n");

    free(edges);
    /* 释放图的邻接边 */
    for (int i = 0; i < graph->vertex_count; i++) {
        ArcNode *a = graph->vertices[i].first_arc;
        while (a) { ArcNode *tmp = a; a = a->next_arc; free(tmp); }
    }
    free(graph->vertices);
    free(graph);
}

/* ===== 创新功能B: 新书提醒订阅 ===== */

/* 订阅队列全局变量 */
static SubNode *sub_list_head = NULL;

void stats_subscribe(const char *reader_name, const char *class_prefix)
{
    SubNode *node = (SubNode*)malloc(sizeof(SubNode));
    strncpy(node->reader_name, reader_name, MAX_NAME_LEN - 1);
    node->reader_name[MAX_NAME_LEN - 1] = '\0';
    strncpy(node->class_prefix, class_prefix, 7);
    node->class_prefix[7] = '\0';
    node->next = sub_list_head;
    sub_list_head = node;
    printf("[成功] %s 已订阅分类 %s*，新书入库时您将收到提醒。\n",
           reader_name, class_prefix);
}

void stats_notify_subscribers(const char *class_id, const char *book_title)
{
    int notified = 0;
    SubNode *p = sub_list_head;
    while (p) {
        /* 检查分类号前缀匹配 */
        if (strncmp(class_id, p->class_prefix, strlen(p->class_prefix)) == 0) {
            /* 在实际系统中这里会发送通知，此处仅打印 */
            notified++;
        }
        p = p->next;
    }
    if (notified > 0) {
        printf("\n  [新书提醒] 《%s》(分类号:%s) 已入库，已通知 %d 位订阅者。\n",
               book_title, class_id, notified);
    }
}

void stats_show_subscriptions(void)
{
    if (!sub_list_head) {
        printf("[提示] 当前无订阅记录。\n");
        return;
    }
    printf("\n  ╔══════════════════════════════════╗\n");
    printf("  ║     新书提醒订阅列表             ║\n");
    printf("  ╠══════════════════════════════════╣\n");
    SubNode *p = sub_list_head;
    while (p) {
        printf("  ║  %-12s  分类: %s*         ║\n",
               p->reader_name, p->class_prefix);
        p = p->next;
    }
    printf("  ╚══════════════════════════════════╝\n");
}

void stats_monthly_trend(void)
{
    if (!borrow_list_head) {
        printf("[提示] 无借阅数据，无法分析趋势。\n");
        return;
    }

    /* 统计每月的借阅量 */
    MonthCount *months = NULL;
    int month_count = 0;

    BorrowNode *p = borrow_list_head;
    while (p) {
        char month[8];
        strncpy(month, p->info.borrow_date, 7);
        month[7] = '\0';

        int found = -1;
        for (int i = 0; i < month_count; i++) {
            if (strcmp(months[i].month, month) == 0) {
                found = i;
                break;
            }
        }
        if (found >= 0) {
            months[found].count++;
        } else {
            months = (MonthCount*)realloc(months, sizeof(MonthCount) * (month_count + 1));
            strcpy(months[month_count].month, month);
            months[month_count].count = 1;
            month_count++;
        }
        p = p->next;
    }

    printf("\n  ╔══════════════════════════════════╗\n");
    printf("  ║     月度借阅趋势分析             ║\n");
    printf("  ╠══════════════════════════════════╣\n");
    printf("  ║ 月份        借阅量    热度指示   ║\n");
    printf("  ╠══════════════════════════════════╣\n");

    /* 找最大借阅量做比例尺 */
    int max_count = 0;
    for (int i = 0; i < month_count; i++)
        if (months[i].count > max_count) max_count = months[i].count;

    for (int i = 0; i < month_count; i++) {
        int bar_len = (max_count > 0) ? (months[i].count * 20 / max_count) : 0;
        printf("  ║ %-10s  %-8d  ", months[i].month, months[i].count);
        for (int j = 0; j < bar_len; j++) printf("█");
        printf("\n");
    }
    printf("  ╚══════════════════════════════════╝\n");
    free(months);
}
