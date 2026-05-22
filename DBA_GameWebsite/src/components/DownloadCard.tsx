/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：前端可复用 UI 组件，负责将页面拆成可维护的展示/交互单元。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

'use client';

import { type FC } from 'react';

export interface Platform {
  name: string;
  label: string;
  downloadUrl: string;
  version: string;
  status: string;
  size?: number;
  sha256?: string;
  releaseNotes?: string;
  mandatory?: boolean;
}

interface DownloadCardProps {
  platform: Platform;
}

const DownloadCard: FC<DownloadCardProps> = ({ platform }) => {
  const canDownload = platform.downloadUrl && platform.downloadUrl !== '#';
  const sizeText = platform.size && platform.size > 0 ? formatBytes(platform.size) : '等待发布';

  return (
    <div className="rounded-lg border border-slate-700 bg-slate-900 p-6 transition hover:border-teal-300/70">
      <div className="mb-5 flex items-center gap-4">
        <span className="inline-flex h-12 w-12 items-center justify-center rounded-lg bg-slate-800 text-sm font-bold text-amber-200">
          {platform.label}
        </span>
        <div>
          <h3 className="text-xl font-bold text-white">{platform.name}</h3>
          <p className="text-sm text-slate-400">版本 {platform.version}</p>
        </div>
      </div>
      <div className="mb-5 space-y-2 text-sm text-slate-300">
        <p>文件大小：{sizeText}</p>
        <p className="break-all">SHA256：{platform.sha256 || '等待发布'}</p>
        {platform.mandatory && <p className="font-semibold text-amber-200">强制更新版本</p>}
        {platform.releaseNotes && <p className="leading-6 text-slate-400">{platform.releaseNotes}</p>}
      </div>
      {canDownload ? (
        <a
          href={platform.downloadUrl}
          className="inline-flex w-full items-center justify-center rounded-lg bg-teal-400 px-6 py-3 font-semibold text-slate-950 transition hover:bg-teal-300"
        >
          {platform.status}
        </a>
      ) : (
        <span className="inline-flex w-full cursor-not-allowed items-center justify-center rounded-lg bg-slate-700 px-6 py-3 font-semibold text-slate-300">
          {platform.status}
        </span>
      )}
    </div>
  );
};

function formatBytes(value: number) {
  if (value < 1024) return `${value} B`;
  if (value < 1024 * 1024) return `${(value / 1024).toFixed(1)} KB`;
  if (value < 1024 * 1024 * 1024) return `${(value / 1024 / 1024).toFixed(1)} MB`;
  return `${(value / 1024 / 1024 / 1024).toFixed(2)} GB`;
}

export default DownloadCard;
