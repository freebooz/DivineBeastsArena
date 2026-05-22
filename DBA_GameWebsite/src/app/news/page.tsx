/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：Next.js App Router 页面，负责官网路由、内容组织和响应式展示。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

import type { Metadata } from 'next';
import NewsList from '@/components/NewsList';
import type { NewsItem } from '@/components/NewsList';

export const metadata: Metadata = {
  title: '新闻公告 - 五灵争霸：神兽觉醒',
  description: '五灵争霸最新新闻、公告和开发进展。',
};

const news: NewsItem[] = [
  {
    slug: 'platform-login',
    title: '平台登录与角色数据接入真实后端',
    excerpt: '账号登录、游客登录、创建角色和选择角色已接入 Game.Api，并通过数据库持久化。',
    date: '2026-05-22',
    category: '开发公告',
  },
  {
    slug: 'launcher-update',
    title: '启动器接入版本清单与文件校验流程',
    excerpt: 'Windows 启动器已支持本地版本检查、远端清单拉取、文件校验、打开日志和启动游戏。',
    date: '2026-05-22',
    category: '启动器',
  },
  {
    slug: 'ops-dashboard',
    title: '管理后台增加运营状态仪表盘',
    excerpt: '运营仪表盘可查看账号、角色、服务端、工单、举报、公告、活动和客户端版本状态。',
    date: '2026-05-22',
    category: '运营',
  },
];

export default function NewsPage() {
  return (
    <div className="min-h-screen bg-slate-950 px-4 py-24">
      <div className="mx-auto max-w-4xl">
        <h1 className="mb-4 text-4xl font-bold text-white md:text-5xl">新闻公告</h1>
        <p className="mb-12 text-xl text-slate-400">查看平台、客户端和运营系统的最新进展。</p>
        <NewsList news={news} />
      </div>
    </div>
  );
}
