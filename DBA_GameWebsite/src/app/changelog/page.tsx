/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：Next.js App Router 页面，负责官网路由、内容组织和响应式展示。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

import type { Metadata } from 'next';
import ChangelogList from '@/components/ChangelogList';
import type { ChangelogEntry } from '@/components/ChangelogList';

export const metadata: Metadata = {
  title: '更新日志 - 五灵争霸：神兽觉醒',
  description: '五灵争霸平台和客户端开发更新记录。',
};

const changelog: ChangelogEntry[] = [
  {
    version: '0.2.0',
    date: '2026-05-22',
    type: 'minor',
    changes: [
      '新增后端启动器清单接口和运营状态接口',
      '新增官网反馈提交到数据库',
      '管理后台仪表盘接入真实运营状态',
      '启动器前端替换为真实检查和启动流程',
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

export default function ChangelogPage() {
  return (
    <div className="min-h-screen bg-slate-950 px-4 py-24">
      <div className="mx-auto max-w-4xl">
        <h1 className="mb-4 text-4xl font-bold text-white md:text-5xl">更新日志</h1>
        <p className="mb-12 text-xl text-slate-400">记录平台、启动器、官网、管理后台和客户端的关键变更。</p>
        <ChangelogList entries={changelog} />
      </div>
    </div>
  );
}
