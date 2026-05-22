/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：前端可复用 UI 组件，负责将页面拆成可维护的展示/交互单元。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

'use client';

import { type FC, useState, type FormEvent, type ChangeEvent } from 'react';

interface FormData {
  nickname: string;
  email: string;
  feedbackType: string;
  title: string;
  content: string;
}

interface FormErrors {
  nickname?: string;
  email?: string;
  feedbackType?: string;
  title?: string;
  content?: string;
}

const apiBaseUrl = process.env.NEXT_PUBLIC_API_BASE_URL ?? 'http://localhost:8080';

const validateEmail = (email: string): boolean => {
  if (!email.trim()) return true;
  return /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email);
};

const FeedbackForm: FC = () => {
  const [formData, setFormData] = useState<FormData>({
    nickname: '',
    email: '',
    feedbackType: 'BUG',
    title: '',
    content: '',
  });
  const [errors, setErrors] = useState<FormErrors>({});
  const [isSubmitting, setIsSubmitting] = useState(false);
  const [submitMessage, setSubmitMessage] = useState('');

  const validateForm = (): boolean => {
    const nextErrors: FormErrors = {};

    if (!formData.title.trim()) nextErrors.title = '请输入标题';
    if (!formData.feedbackType.trim()) nextErrors.feedbackType = '请选择类型';
    if (!validateEmail(formData.email)) nextErrors.email = '邮箱格式不正确';
    if (formData.content.trim().length < 10) nextErrors.content = '反馈内容至少需要 10 个字符';

    setErrors(nextErrors);
    return Object.keys(nextErrors).length === 0;
  };

  const handleChange = (
    event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement | HTMLSelectElement>,
  ) => {
    const { name, value } = event.target;
    setFormData((prev) => ({ ...prev, [name]: value }));
    if (errors[name as keyof FormErrors]) {
      setErrors((prev) => ({ ...prev, [name]: undefined }));
    }
  };

  const handleSubmit = async (event: FormEvent<HTMLFormElement>) => {
    event.preventDefault();
    setSubmitMessage('');
    if (!validateForm()) return;

    setIsSubmitting(true);
    try {
      const response = await fetch(`${apiBaseUrl}/api/feedback/`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(formData),
      });

      if (!response.ok) {
        throw new Error('提交失败，请稍后重试');
      }

      setSubmitMessage('反馈已提交，运营团队会尽快查看。');
      setFormData({
        nickname: '',
        email: '',
        feedbackType: 'BUG',
        title: '',
        content: '',
      });
    } catch (error) {
      setSubmitMessage(error instanceof Error ? error.message : '提交失败，请稍后重试');
    } finally {
      setIsSubmitting(false);
    }
  };

  return (
    <form onSubmit={handleSubmit} className="space-y-6 rounded-lg border border-slate-700 bg-slate-900 p-8">
      <div className="grid gap-5 md:grid-cols-2">
        <label className="block">
          <span className="mb-2 block text-sm font-medium text-slate-300">昵称</span>
          <input
            name="nickname"
            value={formData.nickname}
            onChange={handleChange}
            className="w-full rounded-lg border border-slate-700 bg-slate-950 px-4 py-3 text-white outline-none focus:border-teal-300"
            placeholder="可选"
          />
        </label>
        <label className="block">
          <span className="mb-2 block text-sm font-medium text-slate-300">邮箱</span>
          <input
            name="email"
            value={formData.email}
            onChange={handleChange}
            className="w-full rounded-lg border border-slate-700 bg-slate-950 px-4 py-3 text-white outline-none focus:border-teal-300"
            placeholder="可选，用于回复"
          />
          {errors.email && <p className="mt-1 text-sm text-red-300">{errors.email}</p>}
        </label>
      </div>

      <label className="block">
        <span className="mb-2 block text-sm font-medium text-slate-300">反馈类型</span>
        <select
          name="feedbackType"
          value={formData.feedbackType}
          onChange={handleChange}
          className="w-full rounded-lg border border-slate-700 bg-slate-950 px-4 py-3 text-white outline-none focus:border-teal-300"
        >
          <option value="BUG">问题反馈</option>
          <option value="FEATURE">功能建议</option>
          <option value="ACCOUNT">账号问题</option>
          <option value="PAYMENT">支付问题</option>
          <option value="GENERAL">其他</option>
        </select>
      </label>

      <label className="block">
        <span className="mb-2 block text-sm font-medium text-slate-300">标题</span>
        <input
          name="title"
          value={formData.title}
          onChange={handleChange}
          className="w-full rounded-lg border border-slate-700 bg-slate-950 px-4 py-3 text-white outline-none focus:border-teal-300"
          placeholder="简要描述问题或建议"
        />
        {errors.title && <p className="mt-1 text-sm text-red-300">{errors.title}</p>}
      </label>

      <label className="block">
        <span className="mb-2 block text-sm font-medium text-slate-300">内容</span>
        <textarea
          name="content"
          value={formData.content}
          onChange={handleChange}
          rows={6}
          className="w-full resize-none rounded-lg border border-slate-700 bg-slate-950 px-4 py-3 text-white outline-none focus:border-teal-300"
          placeholder="请写明发生时间、账号、角色名、问题表现和复现方式。"
        />
        {errors.content && <p className="mt-1 text-sm text-red-300">{errors.content}</p>}
      </label>

      {submitMessage && <p className="rounded-lg bg-slate-800 px-4 py-3 text-sm text-slate-100">{submitMessage}</p>}

      <button
        type="submit"
        disabled={isSubmitting}
        className="w-full rounded-lg bg-teal-400 px-6 py-3 font-semibold text-slate-950 transition hover:bg-teal-300 disabled:cursor-wait disabled:opacity-60"
      >
        {isSubmitting ? '提交中...' : '提交反馈'}
      </button>
    </form>
  );
};

export default FeedbackForm;
