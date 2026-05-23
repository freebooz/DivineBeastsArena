/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：Next.js App Router 页面，负责官网路由、内容组织和响应式展示。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

import type { Metadata } from 'next';
import Image from 'next/image';
import { footerLinks, homeFeatures } from '@/data/siteContent';

export const metadata: Metadata = {
  title: '五灵争霸：神兽觉醒',
  description: '五行神兽题材的多人竞技游戏，提供客户端下载、新闻、FAQ 与反馈入口。',
};

export default function HomePage() {
  return (
    <main className="min-h-screen bg-[#10151c] text-white">
      <section className="relative min-h-[84vh] overflow-hidden px-6 py-10">
        <Image
          src="/divine-beasts-login-bg.png"
          alt="五灵争霸登录背景"
          fill
          priority
          sizes="100vw"
          className="object-cover"
        />
        <div className="absolute inset-0 bg-black/58" />
        <div className="relative z-10 mx-auto flex min-h-[72vh] max-w-6xl flex-col justify-center">
          <p className="mb-4 text-sm font-semibold uppercase text-amber-300">Divine Beasts Arena</p>
          <h1 className="max-w-3xl text-5xl font-bold leading-tight md:text-7xl">
            五灵争霸：神兽觉醒
          </h1>
          <p className="mt-6 max-w-2xl text-lg leading-8 text-slate-100">
            五行神兽、十二生肖灵契者与多人竞技战场汇聚一体。登录、选角、启动器和运营后台正在接入同一套 DBA_GameBackend 服务。
          </p>
          <div className="mt-8 flex flex-wrap gap-3">
            <a
              href="#download"
              className="rounded-lg bg-amber-400 px-6 py-3 font-semibold text-slate-950 transition hover:bg-amber-300"
            >
              下载客户端
            </a>
            <a
              href="#platform"
              className="rounded-lg border border-white/35 px-6 py-3 font-semibold text-white transition hover:bg-white/10"
            >
              查看平台能力
            </a>
          </div>
        </div>
      </section>

      <section id="platform" className="px-6 py-16">
        <div className="mx-auto max-w-6xl">
          <h2 className="text-3xl font-bold">平台能力</h2>
          <div className="mt-8 grid gap-5 md:grid-cols-3">
            {homeFeatures.map((feature) => (
              <article key={feature.title} className="rounded-lg border border-slate-700 bg-slate-900 p-6">
                <h3 className="text-xl font-semibold text-amber-200">{feature.title}</h3>
                <p className="mt-4 leading-7 text-slate-300">{feature.body}</p>
              </article>
            ))}
          </div>
        </div>
      </section>

      <section id="download" className="border-y border-slate-800 bg-slate-950 px-6 py-16">
        <div className="mx-auto grid max-w-6xl gap-8 md:grid-cols-[1fr_360px]">
          <div>
            <h2 className="text-3xl font-bold">客户端下载</h2>
            <p className="mt-4 max-w-2xl leading-8 text-slate-300">
              Windows 客户端通过 DBA_GameLauncher 启动。启动器会检查本地版本、拉取远端清单、校验文件并带入后端 API 地址。
            </p>
          </div>
          <div className="rounded-lg border border-slate-700 bg-slate-900 p-6">
            <p className="text-sm text-slate-400">当前开发版本</p>
            <p className="mt-2 text-3xl font-bold text-amber-200">0.1.0</p>
            <a
              href="#"
              className="mt-5 inline-flex w-full justify-center rounded-lg bg-teal-400 px-5 py-3 font-semibold text-slate-950 transition hover:bg-teal-300"
            >
              等待发布包
            </a>
          </div>
        </div>
      </section>

      <footer className="px-6 py-10">
        <div className="mx-auto flex max-w-6xl flex-col gap-4 md:flex-row md:items-center md:justify-between">
          <nav className="flex flex-wrap gap-4 text-sm text-slate-400">
            {footerLinks.map((link) => (
              <a key={link.href} href={link.href} className="hover:text-white">
                {link.label}
              </a>
            ))}
          </nav>
          <p className="text-sm text-slate-500">© 2026 五灵争霸：神兽觉醒</p>
        </div>
      </footer>
    </main>
  );
}
