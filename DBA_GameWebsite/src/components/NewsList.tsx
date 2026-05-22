/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：前端可复用 UI 组件，负责将页面拆成可维护的展示/交互单元。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

'use client';

import { type FC } from 'react';
import Link from 'next/link';

export interface NewsItem {
  slug: string;
  title: string;
  excerpt: string;
  date: string;
  category: string;
}

interface NewsListProps {
  news: NewsItem[];
}

const NewsList: FC<NewsListProps> = ({ news }) => {
  return (
    <div className="space-y-6">
      {news.map((item) => (
        <Link
          key={item.slug}
          href={`/news/${item.slug}`}
          className="block bg-gray-800 rounded-xl p-6 hover:bg-gray-700 transition-colors duration-200"
        >
          <div className="flex items-center gap-3 mb-3">
            <span className="px-3 py-1 text-sm font-medium text-purple-400 bg-purple-500/20 rounded-full">
              {item.category}
            </span>
            <span className="text-gray-500 text-sm">{item.date}</span>
          </div>
          <h3 className="text-xl font-bold text-white mb-2">{item.title}</h3>
          <p className="text-gray-400">{item.excerpt}</p>
        </Link>
      ))}
    </div>
  );
};

export default NewsList;