/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：Next.js App Router 页面，负责官网路由、内容组织和响应式展示。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

import type { Metadata } from 'next';

export const metadata: Metadata = {
  title: 'Privacy Policy - MyGamePlatform',
  description: 'Privacy Policy for MyGamePlatform.',
};

export default function PrivacyPage() {
  return (
    <div className="min-h-screen bg-gray-900 py-24 px-4">
      <article className="max-w-4xl mx-auto">
        <h1 className="text-4xl md:text-5xl font-bold text-white mb-8">
          Privacy Policy
        </h1>
        <p className="text-gray-500 mb-8">Last updated: May 16, 2026</p>

        <div className="prose prose-invert prose-lg max-w-none space-y-6">
          <p className="text-gray-300">
            MyGamePlatform (&quot;we,&quot; &quot;our,&quot; or &quot;us&quot;) is committed to protecting your privacy. This Privacy Policy explains how we collect, use, disclose, and safeguard your information when you play our game or use our services.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Information We Collect
          </h2>
          <p className="text-gray-300">
            We collect information that you provide directly to us, including:
          </p>
          <ul className="list-disc list-inside text-gray-300 space-y-2">
            <li>Account information (username, email, password)</li>
            <li>Profile information and preferences</li>
            <li>Gameplay data and statistics</li>
            <li>Communications and feedback you send to us</li>
          </ul>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            How We Use Your Information
          </h2>
          <p className="text-gray-300">
            We use the information we collect to:
          </p>
          <ul className="list-disc list-inside text-gray-300 space-y-2">
            <li>Provide, maintain, and improve our services</li>
            <li>Track gameplay statistics and update leaderboards</li>
            <li>Communicate with you about updates and events</li>
            <li>Prevent cheating and ensure fair play</li>
            <li>Protect the rights and safety of our players</li>
          </ul>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Information Sharing
          </h2>
          <p className="text-gray-300">
            We do not sell your personal information. We may share your information with third-party service providers who assist us in operating our services, and when required by law.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Data Security
          </h2>
          <p className="text-gray-300">
            We implement appropriate technical and organizational measures to protect your personal information against unauthorized access, alteration, disclosure, or destruction.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Your Rights
          </h2>
          <p className="text-gray-300">
            You have the right to access, update, or delete your personal information. You may also opt out of certain data collection or processing. Contact us through our Feedback page to exercise these rights.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Changes to This Policy
          </h2>
          <p className="text-gray-300">
            We may update this Privacy Policy from time to time. We will notify you of any changes by posting the new policy on this page and updating the &quot;Last updated&quot; date.
          </p>

          <h2 className="text-2xl font-bold text-white mt-8 mb-4">
            Contact Us
          </h2>
          <p className="text-gray-300">
            If you have any questions about this Privacy Policy, please contact us through our Feedback page.
          </p>
        </div>
      </article>
    </div>
  );
}