/*
中文阅读说明：
- 所属应用：DBA_GameBackend 上线前压测。
- 文件职责：用 k6 压测玩家登录接口，验证认证链路吞吐、延迟和错误率。
- 使用方式：BASE_URL=http://localhost:8080 k6 run load-tests/k6-login.js
- 修改提示：如测试账号变化，请同步 DEV_USERNAME / DEV_PASSWORD 默认值或通过环境变量覆盖。
*/

import http from 'k6/http';
import { check, sleep } from 'k6';

export const options = {
  scenarios: {
    login_smoke: {
      executor: 'ramping-vus',
      stages: [
        { duration: '30s', target: 10 },
        { duration: '1m', target: 30 },
        { duration: '30s', target: 0 },
      ],
    },
  },
  thresholds: {
    http_req_failed: ['rate<0.01'],
    http_req_duration: ['p(95)<500'],
  },
};

const baseUrl = __ENV.BASE_URL || 'http://localhost:8080';
const username = __ENV.DEV_USERNAME || 'dba_dev_01';
const password = __ENV.DEV_PASSWORD || 'Dev@123456';

export default function () {
  const response = http.post(
    `${baseUrl}/api/auth/dev-login`,
    JSON.stringify({ username, password }),
    { headers: { 'Content-Type': 'application/json' } },
  );

  check(response, {
    'login status is 200': (r) => r.status === 200,
    'login returns access token': (r) => String(r.body).includes('accessToken'),
  });

  sleep(1);
}
