import { HttpErrorResponse, HttpInterceptorFn } from '@angular/common/http';
import { inject } from '@angular/core';
import { Router } from '@angular/router';
import { catchError, throwError } from 'rxjs';
import { environment } from '../../environments/environment';
import { AuthService } from './auth.service';

export const authInterceptor: HttpInterceptorFn = (request, next) => {
  const auth = inject(AuthService);
  const router = inject(Router);
  const token = auth.token();
  if (!token || request.headers.has('Authorization') || !shouldAttachToken(request.url)) {
    return next(request);
  }

  const authenticatedRequest = request.clone({
    setHeaders: {
      Authorization: `Bearer ${token}`
    }
  });

  return next(authenticatedRequest).pipe(
    catchError((error: unknown) => {
      if (error instanceof HttpErrorResponse && (error.status === 401 || error.status === 403)) {
        handleAuthFailure(auth, router);
      }

      return throwError(() => error);
    })
  );
};

function handleAuthFailure(auth: AuthService, router: Router): void {
  auth.signOut();
  void router.navigate(['/login'], {
    queryParams: {
      returnUrl: window.location.pathname + window.location.search
    }
  });
}

function shouldAttachToken(url: string): boolean {
  if (url.startsWith('/')) {
    return true;
  }

  const apiBaseUrl = environment.apiBaseUrl;
  if (!apiBaseUrl) {
    return false;
  }

  try {
    const configuredApiOrigin = new URL(apiBaseUrl, window.location.origin).origin;
    const requestOrigin = new URL(url, window.location.origin).origin;
    return requestOrigin === configuredApiOrigin;
  } catch {
    return false;
  }
}
