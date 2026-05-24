import { CommonModule } from '@angular/common';
import { Component, inject } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { Router } from '@angular/router';
import { AdminApiService } from '../core/admin-api.service';
import { AuthService } from '../core/auth.service';

@Component({
  selector: 'dba-login-page',
  standalone: true,
  imports: [CommonModule, FormsModule],
  template: `
    <section class="login-page">
      <form class="login-panel" (ngSubmit)="login()">
        <span class="eyebrow">Operations Console</span>
        <h1>GameAdmin</h1>
        <label>
          账号
          <input name="username" [(ngModel)]="username" autocomplete="username" required>
        </label>
        <label>
          密码
          <input name="password" [(ngModel)]="password" type="password" autocomplete="current-password" required>
        </label>
        <p class="error" *ngIf="error">{{ error }}</p>
        <button type="submit" [disabled]="loading">{{ loading ? '登录中...' : '登录' }}</button>
      </form>
    </section>
  `
})
export class LoginPageComponent {
  private readonly api = inject(AdminApiService);
  private readonly auth = inject(AuthService);
  private readonly router = inject(Router);

  username = '';
  password = '';
  loading = false;
  error = '';

  login(): void {
    if (!this.username.trim() || !this.password) {
      this.error = '请输入账号和密码';
      return;
    }

    this.loading = true;
    this.error = '';
    this.api.login({ username: this.username.trim(), password: this.password }).subscribe({
      next: (login) => {
        this.auth.signIn(login);
        void this.router.navigate(['/dashboard']);
      },
      error: () => {
        this.error = '登录失败，请检查账号、密码或后台服务状态';
        this.loading = false;
      }
    });
  }
}
