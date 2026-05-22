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

type LauncherManifestFile = {
  name: string;
  sha256: string;
  size: number;
};

type LauncherManifest = {
  version: string;
  channel: string;
  platform: string;
  downloadUrl: string;
  mandatory: boolean;
  releaseNotes?: string | null;
  files: LauncherManifestFile[];
};

type ApiEnvelope<T> = {
  success?: boolean;
  data?: T;
};

async function getWindowsPlatform(): Promise<Platform> {
  const apiBaseUrl =
    process.env.GAME_API_BASE_URL ?? process.env.NEXT_PUBLIC_GAME_API_BASE_URL ?? 'http://localhost:8080';

  try {
    const response = await fetch(
      `${apiBaseUrl}/api/launcher/manifest?channel=stable&platform=Windows`,
      { next: { revalidate: 60 } },
    );
    if (!response.ok) {
      throw new Error(`manifest request failed: ${response.status}`);
    }

    const envelope = (await response.json()) as ApiEnvelope<LauncherManifest> | LauncherManifest;
    const manifest = 'data' in envelope && envelope.data ? envelope.data : (envelope as LauncherManifest);
    const file = manifest.files[0];

    return {
      name: 'Windows 启动器',
      label: 'WIN',
      downloadUrl: manifest.downloadUrl || '#',
      version: manifest.version,
      status: manifest.downloadUrl ? '下载安装器' : '等待发布包',
      size: file?.size,
      sha256: file?.sha256,
      releaseNotes: manifest.releaseNotes ?? undefined,
      mandatory: manifest.mandatory,
    };
  } catch {
    return {
      name: 'Windows 启动器',
      label: 'WIN',
      downloadUrl: '#',
      version: '0.1.0',
      status: '等待发布包',
      releaseNotes: '暂时无法连接版本发布接口，请稍后重试。',
    };
  }
}

export default async function DownloadPage() {
  const platforms: Platform[] = [
    await getWindowsPlatform(),
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
            Windows 下载信息已接入 Game.Api 版本清单接口。正式发布时，运营流水线需要写入真实下载地址、文件大小和 SHA256，官网与启动器会自动读取同一份清单。
          </p>
        </div>
      </div>
    </div>
  );
}
