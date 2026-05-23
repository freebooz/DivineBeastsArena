/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：集中维护官网静态内容，后续可替换为 CMS 或 Game.Api 内容接口。
- 阅读重点：首页能力点、导航链接、FAQ、更新日志和新闻详情共享同一份数据。
- 修改提示：新增公开内容先改这里，页面组件只负责展示。
*/

import type { ChangelogEntry } from '@/components/ChangelogList';
import type { FAQItem } from '@/components/FAQList';
import type { NewsItem } from '@/components/NewsList';

export const homeFeatures = [
  {
    title: '真实账号与角色',
    body: '账号登录、游客登录、创建角色和选择角色均通过后端 API 验证，并由数据库持久化。',
  },
  {
    title: '专用游戏服务端',
    body: '对局服务端通过内部接口注册、心跳和上报状态，方便大厅、匹配和运维后台统一管理。',
  },
  {
    title: '可扩展运营系统',
    body: '公告、活动、背包、邮件、工单、版本检查和数据分析接口已经纳入平台后端规划。',
  },
];

export const footerLinks = [
  { href: '/news', label: '新闻' },
  { href: '/changelog', label: '更新日志' },
  { href: '/faq', label: '常见问题' },
  { href: '/feedback', label: '反馈' },
  { href: '/privacy', label: '隐私政策' },
  { href: '/terms', label: '服务条款' },
];

export const faqItems: FAQItem[] = [
  {
    question: '现在可以使用哪些账号测试？',
    answer: '开发账号记录在 DBA_GameBackend/docs/dev-login-accounts.md，登录需要启动 Game.Api 并通过真实 API 验证。',
  },
  {
    question: '游客登录后会进入哪里？',
    answer: '游客登录后会进入角色选择或创建角色流程，不会跳过角色数据直接进入大厅。',
  },
  {
    question: '启动器现在支持哪些能力？',
    answer: '启动器支持检查本地版本、拉取远端清单、检查更新、校验/修复文件、打开日志和启动游戏客户端。',
  },
  {
    question: '反馈会保存在哪里？',
    answer: '官网反馈会调用 Game.Api 的 /api/feedback 接口，并写入数据库中的 PlayerFeedbacks 表。',
  },
  {
    question: '正式下载地址什么时候开放？',
    answer: '正式发布包需要由版本发布流水线生成下载地址、校验值和文件清单后，再同步给官网和启动器。',
  },
];

export const changelogEntries: ChangelogEntry[] = [
  {
    version: '0.2.0',
    date: '2026-05-22',
    type: 'minor',
    changes: [
      '新增后端启动器清单接口和运营状态接口',
      '新增官网反馈提交到数据库',
      '管理后台仪表盘接入真实运营状态',
      '启动器前端替换为真实检查、修复和启动流程',
      'DBA_GameClient 增加启动画面图片资源',
    ],
  },
  {
    version: '0.1.0',
    date: '2026-05-21',
    type: 'patch',
    changes: [
      '开发账号、游客登录、角色创建和角色选择接入真实 API',
      '角色数据写入数据库并支持下次登录读取',
      '登录流程修复为先选角或创角再进入大厅',
    ],
  },
];

export type Article = NewsItem & {
  content: string;
};

export const newsArticles: Article[] = [
  {
    slug: 'platform-login',
    title: '平台登录与角色数据接入真实后端',
    excerpt: '账号登录、游客登录、创建角色和选择角色已接入 Game.Api，并通过数据库持久化。',
    date: '2026-05-22',
    category: '开发公告',
    content: `账号登录、游客登录、创建角色和选择角色已接入 Game.Api，并通过 PostgreSQL 持久化。

本轮改造统一了客户端登录流程：玩家登录后会进入角色选择或创建角色流程，不再绕过角色数据直接进入大厅。

后续重点是用真实客户端包验证登录、角色列表、创建角色、选择角色和重新登录后的状态恢复。`,
  },
  {
    slug: 'launcher-update',
    title: '启动器接入版本清单与文件校验流程',
    excerpt: 'Windows 启动器已支持本地版本检查、远端清单拉取、文件校验/修复、打开日志和启动游戏。',
    date: '2026-05-22',
    category: '启动器',
    content: `Windows 启动器已接入后端版本清单接口，可以检查本地版本、拉取远端 manifest、判断更新状态并启动游戏。

文件校验流程基于 SHA256。正式发布时，版本流水线需要写入真实下载地址、文件大小和校验值，官网与启动器会读取同一份清单。

当前重点是补齐真实 CDN 文件、断点续传、安装包签名和自动更新策略。`,
  },
  {
    slug: 'ops-dashboard',
    title: '管理后台增加运营状态仪表盘',
    excerpt: '运营仪表盘可查看账号、角色、服务端、工单、举报、公告、活动和客户端版本状态。',
    date: '2026-05-22',
    category: '运营',
    content: `管理后台已接入运营状态视图，用于查看账号、角色、Dedicated Server、工单、举报、公告、活动和客户端版本状态。

后台页面通过 Game.Api 的管理接口读取数据，并对高危操作保留 reason 和审计记录。

下一步建议补充后台 UI 自动化测试，并在预生产账号下完成真实运营验收。`,
  },
];

export const newsItems: NewsItem[] = newsArticles.map(({ content, ...item }) => item);

export function getArticle(slug: string) {
  return newsArticles.find((article) => article.slug === slug);
}
