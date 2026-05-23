/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：代理官网反馈提交到 Game.Api，减少浏览器端直接依赖后端域名和 CORS。
- 阅读重点：只透传公开反馈接口，不在官网层保存玩家数据。
- 修改提示：生产环境通过 GAME_API_BASE_URL 指向后端公网或内网地址。
*/

import { NextResponse } from 'next/server';

const apiBaseUrl =
  process.env.GAME_API_BASE_URL ??
  process.env.NEXT_PUBLIC_GAME_API_BASE_URL ??
  process.env.NEXT_PUBLIC_API_BASE_URL ??
  'http://localhost:8080';

export async function POST(request: Request) {
  let payload: unknown;
  try {
    payload = await request.json();
  } catch {
    return NextResponse.json({ message: '请求内容不是有效 JSON。' }, { status: 400 });
  }

  try {
    const response = await fetch(`${apiBaseUrl}/api/feedback/`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload),
      cache: 'no-store',
    });

    const text = await response.text();
    const contentType = response.headers.get('content-type') ?? 'application/json';
    return new Response(text, {
      status: response.status,
      headers: { 'Content-Type': contentType },
    });
  } catch {
    return NextResponse.json({ message: '反馈服务暂时不可用，请稍后重试。' }, { status: 502 });
  }
}
