/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：Next.js App Router 页面，负责官网路由、内容组织和响应式展示。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

import type { Metadata } from 'next';

interface PageProps {
  params: Promise<{
    slug: string;
  }>;
}

const articles: Record<string, {
  title: string;
  date: string;
  category: string;
  content: string;
}> = {
  'launch-day': {
    title: 'MyGamePlatform Launches Today',
    date: '2026-05-16',
    category: 'Announcement',
    content: `We are thrilled to announce that MyGamePlatform is officially launching today! This marks a significant milestone for our team, and we cannot wait for you to join us on this adventure.

MyGamePlatform represents years of hard work, dedication, and passion for creating an unforgettable gaming experience. From our initial concept to where we are today, we have remained committed to delivering a game that players truly enjoy.

## What Awaits You

- **Real-time Combat**: Experience intense PvP battles with responsive controls
- **Global Leaderboards**: Compete with players worldwide
- **Cross-Platform Play**: Play with friends on any platform

Thank you for being part of this journey with us. We look forward to seeing you in-game!`,
  },
  'release-notes-v1': {
    title: 'Version 1.0.0 Release Notes',
    date: '2026-05-15',
    category: 'Update',
    content: `The first major release of MyGamePlatform is here! Version 1.0.0 brings a complete gaming experience with all the features you have been waiting for.

## New Features

- Real-time combat system with skill-based matchmaking
- Global leaderboards with seasonal rankings
- Cross-platform support for Windows, Mac, and Linux
-好友 system for connecting with players worldwide
- Comprehensive settings menu for customization

## Bug Fixes

- Fixed issue with connection drops during peak hours
- Resolved visual glitches on certain map configurations
- Corrected leaderboard display issues

We hope you enjoy this release!`,
  },
  'open-beta': {
    title: 'Open Beta Now Available',
    date: '2026-05-10',
    category: 'Beta',
    content: `The open beta for MyGamePlatform is now available to all players! This is your chance to get a head start and help us shape the final product.

## How to Participate

1. Download the game from our website
2. Create your account
3. Start playing!

We will be monitoring all feedback carefully and implementing changes based on your suggestions. Thank you for helping us make MyGamePlatform the best it can be!`,
  },
  'development-update': {
    title: 'Development Update: What is Next',
    date: '2026-05-01',
    category: 'Dev Update',
    content: `As we continue to develop MyGamePlatform, we wanted to share some exciting things we are working on for future updates.

## Upcoming Features

- **Tournament Mode**: Compete in organized tournaments with prizes
- **Clan System**: Create or join clans with friends
- **Customization**: More options for personalizing your experience
- **New Maps**: Additional battlegrounds to explore

We will share more details as we get closer to releasing these features. Stay tuned!`,
  },
};

export async function generateMetadata({ params }: PageProps): Promise<Metadata> {
  const { slug } = await params;
  const article = articles[slug];
  if (!article) {
    return { title: 'Article Not Found' };
  }
  return {
    title: `${article.title} - MyGamePlatform`,
    description: article.content.substring(0, 160),
  };
}

export default async function NewsArticlePage({ params }: PageProps) {
  const { slug } = await params;
  const article = articles[slug];

  if (!article) {
    return (
      <div className="min-h-screen bg-gray-900 py-24 px-4">
        <div className="max-w-4xl mx-auto text-center">
          <h1 className="text-4xl font-bold text-white mb-4">
            Article Not Found
          </h1>
          <p className="text-gray-400">
            The article you are looking for does not exist.
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
          ← Back to News
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