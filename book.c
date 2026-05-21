/**
 * book.c — 图书管理模块实现
 *
 * 【数据结构说明】
 * 1. 双向链表: 存储所有图书记录，支持O(n)遍历，O(1)头尾插入删除
 * 2. BST (二叉搜索树): 以ISBN作为键值建立索引
 *    - 插入: 平均O(log n)，最坏O(n) (退化为链表)
 *    - 查找: 平均O(log n)
 *    - 删除: 平均O(log n)
 *
 * 【BST实现细节】
 * - 每个BST节点持有指向BookNode的指针，不复制数据
 * - ISBN字符串作为比较键值 (使用strcmp)
 * - 删除时用右子树最小节点替代 (Hibbard删除法)
 */

#include "book.h"
#include "fileio.h"

/* ===== 全局变量 ===== */
BookNode *book_list_head = NULL;
BookNode *book_list_tail = NULL;
BSTNode *bst_root = NULL;
int book_total_count = 0;

/* ===== 内部辅助函数 ===== */

/* 释放单本图书的作者链表 */
void book_free_authors(BookNode *book_node)
{
    AuthorNode *p = book_node->info.authors;
    while (p) {
        AuthorNode *tmp = p;
        p = p->next;
        free(tmp);
    }
    book_node->info.authors = NULL;
    book_node->info.author_count = 0;
}

/* ===== 初始化 ===== */

void book_init(void)
{
    book_list_head = load_books();
    /* 找到链表尾部并构建BST */
    if (book_list_head) {
        BookNode *p = book_list_head;
        book_list_tail = book_list_head;
        while (p) {
            book_list_tail = p;
            book_total_count++;
            bst_insert(p);
            p = p->next;
        }
    }
    printf("[初始化] 图书模块就绪，共 %d 本图书。\n", book_total_count);
}

/* ===== BST操作实现 ===== */

void bst_insert(BookNode *book)
{
    BSTNode **curr = &bst_root;
    while (*curr) {
        int cmp = strcmp(book->info.isbn, (*curr)->book->info.isbn);
        if (cmp < 0)
            curr = &((*curr)->left);
        else if (cmp > 0)
            curr = &((*curr)->right);
        else
            return; /* 相同ISBN，不重复插入 */
    }
    BSTNode *node = (BSTNode*)malloc(sizeof(BSTNode));
    node->book = book;
    node->left = node->right = NULL;
    *curr = node;
}

void bst_remove(const char *isbn)
{
    BSTNode **curr = &bst_root;
    /* 查找要删除的节点 */
    while (*curr && strcmp(isbn, (*curr)->book->info.isbn) != 0) {
        if (strcmp(isbn, (*curr)->book->info.isbn) < 0)
            curr = &((*curr)->left);
        else
            curr = &((*curr)->right);
    }
    if (!*curr) return; /* 未找到 */

    BSTNode *target = *curr;
    /* Case 1 & 2: 无左子树或无右子树 (含叶子节点) */
    if (!target->left) {
        *curr = target->right;
    } else if (!target->right) {
        *curr = target->left;
    } else {
        /* Case 3: 左右子树均存在，找右子树最小节点替代 */
        BSTNode *min_parent = target;
        BSTNode *min_node = target->right;
        while (min_node->left) {
            min_parent = min_node;
            min_node = min_node->left;
        }
        /* 将min_node从原位置移除 */
        if (min_parent != target) {
            min_parent->left = min_node->right;
            min_node->right = target->right;
        }
        min_node->left = target->left;
        *curr = min_node;
    }
    free(target);
}

BookNode* bst_search(const char *isbn)
{
    BSTNode *curr = bst_root;
    while (curr) {
        int cmp = strcmp(isbn, curr->book->info.isbn);
        if (cmp == 0) return curr->book;
        curr = (cmp < 0) ? curr->left : curr->right;
    }
    return NULL;
}

/* ===== 图书链表操作 ===== */

BookNode* book_add(BookInfo *info)
{
    /* 检查ISBN是否重复 */
    if (bst_search(info->isbn)) {
        printf("[错误] ISBN %s 已存在，无法重复添加。\n", info->isbn);
        return NULL;
    }

    /* 检查图书编号是否重复 */
    if (book_find_by_id(info->book_id)) {
        printf("[错误] 图书编号 %s 已存在。\n", info->book_id);
        return NULL;
    }

    BookNode *node = (BookNode*)malloc(sizeof(BookNode));
    node->info = *info;
    node->prev = book_list_tail;
    node->next = NULL;

    if (!book_list_head) {
        book_list_head = book_list_tail = node;
    } else {
        book_list_tail->next = node;
        book_list_tail = node;
    }

    bst_insert(node);
    book_total_count++;
    printf("[成功] 图书《%s》已添加。\n", info->title);
    save_books(book_list_head);
    return node;
}

int book_delete(const char *book_id)
{
    BookNode *node = book_find_by_id(book_id);
    if (!node) {
        printf("[错误] 未找到图书编号 %s。\n", book_id);
        return 0;
    }
    /* 检查是否有未归还的借阅 */
    if (node->info.borrowed_count > 0) {
        printf("[错误] 图书《%s》尚有 %d 本未归还，无法删除。\n",
               node->info.title, node->info.borrowed_count);
        return 0;
    }

    /* 从BST中移除 */
    bst_remove(node->info.isbn);

    /* 从链表中移除 */
    if (node->prev) node->prev->next = node->next;
    else book_list_head = node->next;
    if (node->next) node->next->prev = node->prev;
    else book_list_tail = node->prev;

    book_free_authors(node);
    free(node);
    book_total_count--;
    printf("[成功] 图书已删除。\n");
    save_books(book_list_head);
    return 1;
}

int book_modify(const char *book_id, BookInfo new_info)
{
    BookNode *node = book_find_by_id(book_id);
    if (!node) {
        printf("[错误] 未找到图书编号 %s。\n", book_id);
        return 0;
    }

    /* 如果ISBN改变，需要更新BST */
    if (strcmp(node->info.isbn, new_info.isbn) != 0) {
        if (bst_search(new_info.isbn)) {
            printf("[错误] 新ISBN %s 已被占用。\n", new_info.isbn);
            return 0;
        }
        bst_remove(node->info.isbn);
        book_free_authors(node);
        node->info = new_info;
        bst_insert(node);
    } else {
        book_free_authors(node);
        node->info = new_info;
    }

    printf("[成功] 图书信息已更新。\n");
    save_books(book_list_head);
    return 1;
}

BookNode* book_find_by_id(const char *book_id)
{
    BookNode *p = book_list_head;
    while (p) {
        if (strcmp(p->info.book_id, book_id) == 0)
            return p;
        p = p->next;
    }
    return NULL;
}

BookNode* book_find_by_isbn(const char *isbn)
{
    return bst_search(isbn);
}

BookNode** book_search_by_title(const char *keyword, int *count)
{
    /* 先统计匹配数量 */
    *count = 0;
    BookNode *p = book_list_head;
    while (p) {
        if (strstr(p->info.title, keyword))
            (*count)++;
        p = p->next;
    }

    if (*count == 0) return NULL;

    /* 分配结果数组 */
    BookNode **results = (BookNode**)malloc(sizeof(BookNode*) * (*count));
    int i = 0;
    p = book_list_head;
    while (p) {
        if (strstr(p->info.title, keyword))
            results[i++] = p;
        p = p->next;
    }
    return results;
}

BookNode** book_search_by_author(const char *keyword, int *count)
{
    *count = 0;
    BookNode *p = book_list_head;
    while (p) {
        AuthorNode *a = p->info.authors;
        while (a) {
            if (strstr(a->name, keyword)) {
                (*count)++;
                break;
            }
            a = a->next;
        }
        p = p->next;
    }

    if (*count == 0) return NULL;

    BookNode **results = (BookNode**)malloc(sizeof(BookNode*) * (*count));
    int i = 0;
    p = book_list_head;
    while (p) {
        AuthorNode *a = p->info.authors;
        int found = 0;
        while (a) {
            if (strstr(a->name, keyword)) { found = 1; break; }
            a = a->next;
        }
        if (found) results[i++] = p;
        p = p->next;
    }
    return results;
}

BookNode** book_search_by_publisher(const char *keyword, int *count)
{
    *count = 0;
    BookNode *p = book_list_head;
    while (p) {
        if (strstr(p->info.publisher, keyword))
            (*count)++;
        p = p->next;
    }

    if (*count == 0) return NULL;

    BookNode **results = (BookNode**)malloc(sizeof(BookNode*) * (*count));
    int i = 0;
    p = book_list_head;
    while (p) {
        if (strstr(p->info.publisher, keyword))
            results[i++] = p;
        p = p->next;
    }
    return results;
}

/* ===== 辅助函数 ===== */

AuthorNode* author_create(const char *name)
{
    AuthorNode *node = (AuthorNode*)malloc(sizeof(AuthorNode));
    strncpy(node->name, name, MAX_NAME_LEN - 1);
    node->name[MAX_NAME_LEN - 1] = '\0';
    node->next = NULL;
    return node;
}

void author_append(BookInfo *info, const char *name)
{
    AuthorNode *node = author_create(name);
    if (!info->authors) {
        info->authors = node;
    } else {
        AuthorNode *p = info->authors;
        while (p->next) p = p->next;
        p->next = node;
    }
    info->author_count++;
}

void book_print(BookNode *book)
{
    if (!book) { printf("[提示] 无此图书。\n"); return; }

    BookInfo *b = &book->info;
    printf("\n  ╔══════════════════════════════════════════════╗\n");
    printf("  ║  图书编号: %-34s ║\n", b->book_id);
    printf("  ║  ISBN:     %-34s ║\n", b->isbn);
    printf("  ║  书名:     %-34s ║\n", b->title);

    /* 打印所有作者 */
    printf("  ║  作者:     ");
    AuthorNode *a = b->authors;
    int first = 1;
    while (a) {
        if (!first) printf(", ");
        printf("%s", a->name);
        first = 0;
        a = a->next;
    }
    printf("\n");

    printf("  ║  分类号:   %-34s ║\n", b->class_id);
    printf("  ║  出版社:   %-34s ║\n", b->publisher);
    printf("  ║  出版日期: %-34s ║\n", b->pub_date);
    printf("  ║  版次:     第%-31d ║\n", b->edition);
    printf("  ║  定价:     ¥%-33.2f ║\n", b->price);
    printf("  ║  馆藏数:   %-34d ║\n", b->total_copies);
    printf("  ║  已借出:   %-34d ║\n", b->borrowed_count);
    printf("  ║  在馆数:   %-34d ║\n", book_available(book));
    printf("  ╚══════════════════════════════════════════════╝\n");
  
}
int book_available(BookNode *book)
{
    return book->info.total_copies - book->info.borrowed_count;
}
