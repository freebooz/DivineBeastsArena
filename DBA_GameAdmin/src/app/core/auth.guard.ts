import { inject } from '@angular/core';
import { CanActivateChildFn, CanActivateFn, Router, RouterStateSnapshot } from '@angular/router';
import { AuthService } from './auth.service';

export const authGuard: CanActivateFn & CanActivateChildFn = (_route, state: RouterStateSnapshot) => {
  const auth = inject(AuthService);
  if (auth.isAuthenticated()) {
    return true;
  }

  return inject(Router).createUrlTree(['/login'], {
    queryParams: {
      returnUrl: state.url
    }
  });
};
