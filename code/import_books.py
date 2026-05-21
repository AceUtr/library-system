#!/usr/bin/env python3
"""解析图书列表200册.txt，生成books.dat文件"""

import re
import os

# 输入输出路径
input_file = r"c:\Users\wang\Desktop\程序设计训练作业\图书列表200册.txt"
output_file = r"c:\Users\wang\Desktop\程序设计训练作业\Library system\data\books.dat"

with open(input_file, 'r', encoding='utf-8') as f:
    content = f.read()

# 按空行分割每个图书块
blocks = content.split('\n\n')

books = []
for block in blocks:
    lines = block.strip().split('\n')
    if len(lines) < 9:  # 跳过标题和不够的行
        continue

    fields = {}
    for line in lines:
        line = line.strip()
        if line.startswith('中图法分类号:'):
            fields['class_id'] = line.split(':', 1)[1].strip()
        elif line.startswith('图书编号:'):
            fields['book_id'] = line.split(':', 1)[1].strip()
        elif line.startswith('书名:'):
            fields['title'] = line.split(':', 1)[1].strip()
        elif line.startswith('作者:'):
            fields['authors'] = line.split(':', 1)[1].strip()
        elif line.startswith('出版社:'):
            fields['publisher'] = line.split(':', 1)[1].strip()
        elif line.startswith('出版日期:'):
            fields['pub_date'] = line.split(':', 1)[1].strip()
        elif line.startswith('ISBN:'):
            fields['isbn'] = line.split(':', 1)[1].strip()
        elif line.startswith('版次:'):
            raw = line.split(':', 1)[1].strip()
            # 去除"第"和"版" -> 如 "第6版" -> "6"
            raw = raw.replace('第', '').replace('版', '').strip()
            fields['edition'] = raw
        elif line.startswith('定价:'):
            raw = line.split(':', 1)[1].strip()
            # 去除"元"
            raw = raw.replace('元', '').strip()
            fields['price'] = raw
        elif line.startswith('馆藏数:'):
            fields['total_copies'] = line.split(':', 1)[1].strip()

    books.append(fields)

# 写入 books.dat
with open(output_file, 'w', encoding='utf-8') as f:
    for b in books:
        line = '|'.join([
            b.get('class_id', ''),
            b.get('book_id', ''),
            b.get('title', ''),
            b.get('authors', ''),
            b.get('publisher', ''),
            b.get('pub_date', ''),
            b.get('isbn', ''),
            b.get('edition', '1'),
            b.get('price', '0'),
            b.get('total_copies', '1'),
            '0'  # borrowed_count
        ])
        f.write(line + '\n')

print(f"已导入 {len(books)} 本图书到 {output_file}")
for b in books[:3]:
    print(f"  {b.get('book_id')}: {b.get('title')} - {b.get('authors')}")
print("  ...")
