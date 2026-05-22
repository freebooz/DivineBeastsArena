/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：Next.js App Router 页面，负责官网路由、内容组织和响应式展示。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

import type { Metadata } from 'next';
import FAQList from '@/components/FAQList';
import type { FAQItem } from '@/components/FAQList';

export const metadata: Metadata = {
  title: '常见问题 - 五灵争霸：神兽觉醒',
  description: '五灵争霸账号、启动器、反馈和测试相关常见问题。',
};

const faqs: FAQItem[] = [
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
    answer: '启动器支持检查本地版本、拉取远端清单、检查更新、校验文件、打开日志和启动游戏客户端。',
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

export default function FAQPage() {
  return (
    <div className="min-h-screen bg-slate-950 px-4 py-24">
      <div className="mx-auto max-w-4xl">
        <h1 className="mb-4 text-4xl font-bold text-white md:text-5xl">常见问题</h1>
        <p className="mb-12 text-xl text-slate-400">围绕账号、角色、启动器、下载和反馈的开发运营说明。</p>
        <FAQList items={faqs} />
      </div>
    </div>
  );
}
