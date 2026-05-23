/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：Next.js App Router 页面，负责官网路由、内容组织和响应式展示。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

import type { Metadata } from 'next';
import FAQList from '@/components/FAQList';
import { faqItems } from '@/data/siteContent';

export const metadata: Metadata = {
  title: '常见问题 - 五灵争霸：神兽觉醒',
  description: '五灵争霸账号、启动器、反馈和测试相关常见问题。',
};

export default function FAQPage() {
  return (
    <div className="min-h-screen bg-slate-950 px-4 py-24">
      <div className="mx-auto max-w-4xl">
        <h1 className="mb-4 text-4xl font-bold text-white md:text-5xl">常见问题</h1>
        <p className="mb-12 text-xl text-slate-400">围绕账号、角色、启动器、下载和反馈的开发运营说明。</p>
        <FAQList items={faqItems} />
      </div>
    </div>
  );
}
