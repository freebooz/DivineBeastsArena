/*
Chinese reading notes:
- Application: DBA_GameBackend pre-production load testing.
- Responsibility: log in, create matchmaking tickets, and collect k6 evidence.
- Usage: BASE_URL=http://localhost:8080 AUTH_MODE=guest k6 run load-tests/k6-matchmaking.js
- AUTH_MODE supports guest, dev, and account. Guest is the default so local
  production-like environments do not depend on pre-seeded test accounts.
*/

import http from 'k6/http';
import { check, sleep } from 'k6';

const baseUrl = __ENV.BASE_URL || 'http://localhost:8080';
const authMode = (__ENV.AUTH_MODE || 'guest').toLowerCase();
const username = __ENV.DEV_USERNAME || __ENV.ACCOUNT_USERNAME || 'dba_dev_01';
const email = __ENV.ACCOUNT_EMAIL || '';
const password = __ENV.DEV_PASSWORD || __ENV.ACCOUNT_PASSWORD || 'Dev@123456';
const matchmakingVus = Number(__ENV.MATCHMAKING_VUS || 10);
const matchmakingDuration = __ENV.MATCHMAKING_DURATION || '1m';
const matchmakingSleepSeconds = Number(__ENV.MATCHMAKING_SLEEP_SECONDS || 1);

export const options = {
  scenarios: {
    matchmaking_smoke: {
      executor: 'constant-vus',
      vus: matchmakingVus,
      duration: matchmakingDuration,
    },
  },
  thresholds: {
    http_req_failed: ['rate<0.02'],
    http_req_duration: ['p(95)<800'],
  },
};

function authRequest() {
  if (authMode === 'guest') {
    const deviceId = `k6-matchmaking-${__VU}-${__ITER}-${Date.now()}`;
    return {
      url: `${baseUrl}/api/auth/guest-login`,
      body: { deviceId, deviceName: 'k6', platform: 'docker-k6' },
    };
  }

  if (authMode === 'account') {
    return {
      url: `${baseUrl}/api/auth/account/login`,
      body: { username, email, password },
    };
  }

  return {
    url: `${baseUrl}/api/auth/dev-login`,
    body: { username, password },
  };
}

function login() {
  const request = authRequest();
  const response = http.post(
    request.url,
    JSON.stringify(request.body),
    { headers: { 'Content-Type': 'application/json' } },
  );

  check(response, {
    'login status is 200': (r) => r.status === 200,
    'login returns access token': (r) => String(r.body).includes('accessToken'),
  });

  if (response.status !== 200) {
    return null;
  }

  const body = response.json();
  return body?.data?.accessToken;
}

export default function () {
  const token = login();
  check(token, { 'token exists': (value) => Boolean(value) });
  if (!token) {
    sleep(matchmakingSleepSeconds);
    return;
  }

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
    'ticket create is 200': (r) => r.status === 200,
  });

  sleep(matchmakingSleepSeconds);
}
