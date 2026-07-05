/*
Chinese reading notes:
- Application: DBA_GameBackend pre-production load testing.
- Responsibility: exercise the authentication path and collect k6 evidence.
- Usage: BASE_URL=http://localhost:8080 AUTH_MODE=guest k6 run load-tests/k6-login.js
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
const loginRampTargetVus = Number(__ENV.LOGIN_RAMP_TARGET_VUS || 30);
const loginRampUpDuration = __ENV.LOGIN_RAMP_UP_DURATION || '30s';
const loginRampHoldDuration = __ENV.LOGIN_RAMP_HOLD_DURATION || '1m';
const loginRampDownDuration = __ENV.LOGIN_RAMP_DOWN_DURATION || '30s';
const loginSleepSeconds = Number(__ENV.LOGIN_SLEEP_SECONDS || 1);

export const options = {
  scenarios: {
    login_smoke: {
      executor: 'ramping-vus',
      stages: [
        { duration: loginRampUpDuration, target: Math.max(1, Math.ceil(loginRampTargetVus / 3)) },
        { duration: loginRampHoldDuration, target: loginRampTargetVus },
        { duration: loginRampDownDuration, target: 0 },
      ],
    },
  },
  thresholds: {
    http_req_failed: ['rate<0.01'],
    http_req_duration: ['p(95)<500'],
  },
};

function authRequest() {
  if (authMode === 'guest') {
    const deviceId = `k6-login-${__VU}-${__ITER}-${Date.now()}`;
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

export default function () {
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

  sleep(loginSleepSeconds);
}
