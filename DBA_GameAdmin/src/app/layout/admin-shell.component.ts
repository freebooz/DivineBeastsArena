import { CommonModule } from '@angular/common';
import { Component, inject } from '@angular/core';
import { Router, RouterLink, RouterLinkActive, RouterOutlet } from '@angular/router';
import { AuthService } from '../core/auth.service';

interface NavItem {
  label: string;
  path: string;
}

@Component({
  selector: 'dba-admin-shell',
  standalone: true,
  imports: [CommonModule, RouterLink, RouterLinkActive, RouterOutlet],
  template: `
    <div class="admin-shell">
      <aside class="sidebar">
        <div class="brand">
          <span class="brand-mark">DBA</span>
          <div>
            <strong>GameAdmin</strong>
            <small>运营管理控制台</small>
          </div>
        </div>

        <nav>
          <a *ngFor="let item of navItems" [routerLink]="item.path" routerLinkActive="active">
            {{ item.label }}
          </a>
        </nav>
      </aside>

      <main class="content">
        <header class="topbar">
          <div>
            <span class="eyebrow">Divine Beasts Arena</span>
            <h1>生产运营后台</h1>
          </div>
          <div class="user-chip" *ngIf="auth.session() as session">
            <span>{{ session.username }}</span>
            <strong>{{ session.role }}</strong>
            <button type="button" (click)="logout()">退出</button>
          </div>
        </header>
        <router-outlet />
      </main>
    </div>
  `
})
export class AdminShellComponent {
  readonly auth = inject(AuthService);
  private readonly router = inject(Router);

  readonly navItems: NavItem[] = [
    { label: 'Dashboard', path: '/dashboard' },
    { label: '玩家', path: '/players' },
    { label: '对局', path: '/matches' },
    { label: '服务器', path: '/servers' },
    { label: '背包', path: '/inventory' },
    { label: '配置', path: '/configs' },
    { label: '客户端版本', path: '/client-versions' },
    { label: '反馈', path: '/feedback' },
    { label: '工单', path: '/support' },
    { label: '审计', path: '/audit' },
    { label: '平台', path: '/platform' }
  ];

  logout(): void {
    this.auth.signOut();
    void this.router.navigate(['/login']);
  }
}
