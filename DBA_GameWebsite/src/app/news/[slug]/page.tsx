/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：Next.js App Router 页面，负责官网路由、内容组织和响应式展示。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

import type { Metadata } from 'next';
import { getArticle } from '@/data/siteContent';

interface PageProps {
  params: Promise<{
    slug: string;
  }>;
}

export async function generateMetadata({ params }: PageProps): Promise<Metadata> {
  const { slug } = await params;
  const article = getArticle(slug);
  if (!article) {
    return { title: '文章不存在 - 五灵争霸：神兽觉醒' };
  }
  return {
    title: `${article.title} - 五灵争霸：神兽觉醒`,
    description: article.excerpt,
  };
}

export default async function NewsArticlePage({ params }: PageProps) {
  const { slug } = await params;
  const article = getArticle(slug);

  if (!article) {
    return (
      <div className="min-h-screen bg-gray-900 py-24 px-4">
        <div className="max-w-4xl mx-auto text-center">
          <h1 className="text-4xl font-bold text-white mb-4">
            文章不存在
          </h1>
          <p className="text-gray-400">
            你要查看的新闻不存在或已经下线。
          </p>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-gray-900 py-24 px-4">
      <article className="max-w-4xl mx-auto">
        <a
          href="/news"
          className="inline-flex items-center gap-2 text-purple-400 hover:text-purple-300 mb-8"
        >
          ← 返回新闻
        </a>
        <div className="mb-6">
          <span className="px-3 py-1 text-sm font-medium text-purple-400 bg-purple-500/20 rounded-full">
            {article.category}
          </span>
          <span className="ml-4 text-gray-500 text-sm">{article.date}</span>
        </div>
        <h1 className="text-4xl md:text-5xl font-bold text-white mb-8">
          {article.title}
        </h1>
        <div className="prose prose-invert prose-lg max-w-none">
          {article.content.split('\n\n').map((paragraph, index) => (
            <p key={index} className="text-gray-300 mb-4 leading-relaxed">
              {paragraph}
            </p>
          ))}
        </div>
      </article>
    </div>
  );
}
