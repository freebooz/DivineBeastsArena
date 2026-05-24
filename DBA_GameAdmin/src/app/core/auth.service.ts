import { Injectable, computed, signal } from '@angular/core';
import { AdminLoginResponse } from './models';

export interface AdminSession {
  accessToken: string;
  adminId: string;
  username: string;
  role: string;
  expiresAt?: number | null;
}

@Injectable({ providedIn: 'root' })
export class AuthService {
  private readonly storageKey = 'dba.admin.session';
  private readonly sessionState = signal<AdminSession | null>(this.loadSession());

  readonly session = this.sessionState.asReadonly();
  readonly isAuthenticated = computed(() => {
    const session = this.sessionState();
    return !!session?.accessToken && !this.isExpired(session);
  });

  token(): string | null {
    const session = this.sessionState();
    return session && !this.isExpired(session) ? session.accessToken : null;
  }

  signIn(login: AdminLoginResponse): void {
    const session: AdminSession = {
      accessToken: login.accessToken,
      adminId: login.adminId,
      username: login.username,
      role: login.role,
      expiresAt: this.readJwtExpiry(login.accessToken)
    };
    localStorage.setItem(this.storageKey, JSON.stringify(session));
    this.sessionState.set(session);
  }

  signOut(): void {
    localStorage.removeItem(this.storageKey);
    this.sessionState.set(null);
  }

  private loadSession(): AdminSession | null {
    try {
      const raw = localStorage.getItem(this.storageKey);
      if (!raw) {
        return null;
      }
      const session = JSON.parse(raw) as AdminSession;
      return this.isExpired(session) ? null : session;
    } catch {
      return null;
    }
  }

  private isExpired(session: AdminSession): boolean {
    return typeof session.expiresAt === 'number' && session.expiresAt <= Date.now();
  }

  private readJwtExpiry(token: string): number | null {
    try {
      const payload = token.split('.')[1];
      if (!payload) {
        return null;
      }
      const normalized = payload.replace(/-/g, '+').replace(/_/g, '/');
      const decoded = JSON.parse(atob(normalized));
      return typeof decoded.exp === 'number' ? decoded.exp * 1000 : null;
    } catch {
      return null;
    }
  }
}
