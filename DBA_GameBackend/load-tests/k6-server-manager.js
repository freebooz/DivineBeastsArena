/*
中文阅读说明：
- 所属应用：DBA_GameBackend 上线前压测。
- 文件职责：用 k6 压测内部 Game Server Manager 分配接口，验证 Dedicated Server 分配层吞吐。
- 使用方式：BASE_URL=http://localhost:8080 INTERNAL_API_KEY=xxx k6 run load-tests/k6-server-manager.js
- 修改提示：此脚本会创建真实 server allocation 记录，只能在压测环境运行。
*/

import http from 'k6/http';
import { check, sleep } from 'k6';

export const options = {
  scenarios: {
    allocate_servers: {
      executor: 'constant-vus',
      vus: 5,
      duration: '30s',
    },
  },
  thresholds: {
    http_req_failed: ['rate<0.05'],
    http_req_duration: ['p(95)<1000'],
  },
};

const baseUrl = __ENV.BASE_URL || 'http://localhost:8080';
const internalApiKey = __ENV.INTERNAL_API_KEY || '';

export default function () {
  const sessionId = randomUuid();
  const response = http.post(
    `${baseUrl}/internal/game-servers/allocate`,
    JSON.stringify({
      sessionId,
      mode: 'classic',
      mapId: 'arena_01',
      region: 'cn',
      buildVersion: 'load-test',
    }),
    {
      headers: {
        'Content-Type': 'application/json',
        'X-Internal-Api-Key': internalApiKey,
      },
    },
  );

  check(response, {
    'allocation accepted or capacity limited': (r) => r.status === 200 || r.status === 409,
  });

  sleep(1);
}

function randomUuid() {
  const template = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx';
  return template.replace(/[xy]/g, (char) => {
    const value = Math.floor(Math.random() * 16);
    const nibble = char === 'x' ? value : (value & 0x3) | 0x8;
    return nibble.toString(16);
  });
}
