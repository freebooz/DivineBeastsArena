/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：Next.js App Router 页面，负责官网路由、内容组织和响应式展示。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

import type { Metadata } from 'next';
import DownloadCard from '@/components/DownloadCard';
import type { Platform } from '@/components/DownloadCard';

export const metadata: Metadata = {
  title: '下载客户端 - 五灵争霸：神兽觉醒',
  description: '下载五灵争霸 Windows 客户端和启动器。',
};

const platforms: Platform[] = [
  {
    name: 'Windows 启动器',
    label: 'WIN',
    downloadUrl: '#',
    version: '0.1.0',
    status: '等待发布包',
  },
  {
    name: 'macOS 客户端',
    label: 'MAC',
    downloadUrl: '#',
    version: '规划中',
    status: '暂未开放',
  },
  {
    name: 'Linux 客户端',
    label: 'LIN',
    downloadUrl: '#',
    version: '规划中',
    status: '暂未开放',
  },
];

export default function DownloadPage() {
  return (
    <div className="min-h-screen bg-slate-950 px-4 py-24">
      <div className="mx-auto max-w-6xl">
        <div className="mb-14 text-center">
          <h1 className="mb-4 text-4xl font-bold text-white md:text-5xl">下载客户端</h1>
          <p className="mx-auto max-w-2xl text-xl text-slate-400">
            当前开发环境以 Windows 启动器为主。启动器会通过 Game.Api 检查版本、拉取清单、校验文件并启动游戏。
          </p>
        </div>

        <div className="mx-auto grid max-w-5xl grid-cols-1 gap-6 md:grid-cols-3">
          {platforms.map((platform) => (
            <DownloadCard key={platform.name} platform={platform} />
          ))}
        </div>

        <div className="mx-auto mt-12 max-w-3xl rounded-lg border border-slate-700 bg-slate-900 p-6 text-slate-300">
          <h2 className="mb-3 text-xl font-semibold text-white">运营发布说明</h2>
          <p className="leading-7">
            正式上线前，下载地址应由后端版本发布接口生成，并同步给官网和启动器。当前页面保留入口结构，避免玩家误下载未发布构建。
          </p>
        </div>
      </div>
    </div>
  );
}
