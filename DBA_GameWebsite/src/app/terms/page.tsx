/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：Next.js App Router 页面，负责官网路由、内容组织和响应式展示。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

import type { Metadata } from 'next';

export const metadata: Metadata = {
  title: 'Terms of Service - MyGamePlatform',
  description: 'Terms of Service for MyGamePlatform.',
};

export default function TermsPage() {
  return (
    <div className="min-h-screen bg-gray-900 py-24 px-4">
      <article className="max-w-4xl mx-auto">
        <h1 className="text-4xl md:text-5xl font-bold text-white mb-8">
          Terms of Service
        </h1>
        <p className="text-gray-500 mb-8">Last updated: May 16, 2026</p>

        <div className="prose prose-invert prose-lg max-w-none space-y-6">
          <p className="text-gray-300">
            Welcome to MyGamePlatform. By playing our game or using our services, you agree to these Terms of Service. Please read them carefully.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Acceptance of Terms
          </h2>
          <p className="text-gray-300">
            By accessing or using MyGamePlatform, you agree to be bound by these Terms of Service and all applicable laws and regulations. If you do not agree with any part of these terms, you may not use our services.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Game License
          </h2>
          <p className="text-gray-300">
            We grant you a limited, non-exclusive, non-transferable license to play MyGamePlatform for your personal, non-commercial entertainment purposes. You may not reverse engineer, decompile, or disassemble the game.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            User Conduct
          </h2>
          <p className="text-gray-300">
            You agree not to:
          </p>
          <ul className="list-disc list-inside text-gray-300 space-y-2">
            <li>Use cheats, exploits, or unauthorized third-party software</li>
            <li>Harass, threaten, or intimidate other players</li>
            <li>Impersonate any person or entity</li>
            <li>Attempt to gain unauthorized access to our systems</li>
            <li>Use the game for any illegal purpose</li>
            <li>Spam or flood our chat systems</li>
          </ul>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Intellectual Property
          </h2>
          <p className="text-gray-300">
            MyGamePlatform and all related content, including but not limited to text, graphics, logos, and software, are the property of MyGamePlatform and are protected by intellectual property laws.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Virtual Items
          </h2>
          <p className="text-gray-300">
            Virtual items, currency, and other in-game assets have no real-world value and cannot be exchanged for money or other goods. These items are not transferable and may be removed if you violate these terms.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Disclaimer of Warranties
          </h2>
          <p className="text-gray-300">
            Our services are provided &quot;as is&quot; and &quot;as available&quot; without warranties of any kind, either express or implied, including but not limited to implied warranties of merchantability or fitness for a particular purpose.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Limitation of Liability
          </h2>
          <p className="text-gray-300">
            To the fullest extent permitted by law, MyGamePlatform shall not be liable for any indirect, incidental, special, consequential, or punitive damages resulting from your use of or inability to use our services.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Changes to Terms
          </h2>
          <p className="text-gray-300">
            We reserve the right to modify these terms at any time. We will notify players of significant changes through the game or our website. Continued use of the service after changes constitutes acceptance of the modified terms.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Contact Us
          </h2>
          <p className="text-gray-300">
            If you have any questions about these Terms of Service, please contact us through our Feedback page.
          </p>
        </div>
      </article>
    </div>
  );
}