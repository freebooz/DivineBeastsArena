/*
中文阅读说明：
- 所属应用：DBA_GameBackend 上线前压测。
- 文件职责：用 k6 压测登录后创建匹配票据的最小链路。
- 使用方式：BASE_URL=http://localhost:8080 k6 run load-tests/k6-matchmaking.js
- 修改提示：当前脚本使用开发账号登录，生产压测应改为预创建的一组压测账号。
*/

import http from 'k6/http';
import { check, sleep } from 'k6';

export const options = {
  scenarios: {
    matchmaking_smoke: {
      executor: 'constant-vus',
      vus: 10,
      duration: '1m',
    },
  },
  thresholds: {
    http_req_failed: ['rate<0.02'],
    http_req_duration: ['p(95)<800'],
  },
};

const baseUrl = __ENV.BASE_URL || 'http://localhost:8080';
const username = __ENV.DEV_USERNAME || 'dba_dev_01';
const password = __ENV.DEV_PASSWORD || 'Dev@123456';

function login() {
  const response = http.post(
    `${baseUrl}/api/auth/dev-login`,
    JSON.stringify({ username, password }),
    { headers: { 'Content-Type': 'application/json' } },
  );
  const body = response.json();
  return body?.data?.accessToken;
}

export default function () {
  const token = login();
  check(token, { 'token exists': (value) => Boolean(value) });
  if (!token) return;

  const ticket = http.post(
    `${baseUrl}/api/matchmaking/tickets`,
    JSON.stringify({ mode: 'classic', region: 'cn', mmr: 1000 }),
    {
      headers: {
        'Content-Type': 'application/json',
        Authorization: `Bearer ${token}`,
      },
    },
  );

  check(ticket, {
    'ticket create is 200 or duplicate guarded': (r) => r.status === 200 || r.status === 400,
  });

  sleep(1);
}
