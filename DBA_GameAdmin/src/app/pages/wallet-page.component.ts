import { CommonModule } from '@angular/common';
import { Component, inject } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { AdminApiService } from '../core/admin-api.service';
import {
  AdminAdjustWalletRequest,
  AdminWalletBalanceItem,
  AdminWalletLedgerItem
} from '../core/models';

type WalletTab = 'balances' | 'ledgers';

@Component({
  selector: 'dba-wallet-page',
  standalone: true,
  imports: [CommonModule, FormsModule],
  template: `
    <section class="page">
      <div class="section-title">
        <div><span class="eyebrow">钱包管理</span><h2>钱包</h2></div>
        <button type="button" (click)="load()">刷新</button>
      </div>

      <form class="inline-form" (ngSubmit)="adjust()">
        <input name="playerId" [(ngModel)]="adjustDraft.playerId" placeholder="玩家ID">
        <input name="currencyType" [(ngModel)]="adjustDraft.currencyType" placeholder="货币类型">
        <input name="amount" [(ngModel)]="adjustDraft.amount" type="number" placeholder="调整金额（正/负）">
        <input name="reason" [(ngModel)]="adjustDraft.reason" placeholder="调整原因">
        <button type="submit">调整余额</button>
      </form>

      <div class="inline-form">
        <button type="button" [class.active]="tab === 'balances'" (click)="switchTab('balances')">余额列表</button>
        <button type="button" [class.active]="tab === 'ledgers'" (click)="switchTab('ledgers')">流水列表</button>
      </div>

      <ng-container *ngIf="tab === 'balances'">
        <table>
          <thead>
            <tr><th>玩家ID</th><th>货币类型</th><th>余额</th><th>更新时间</th></tr>
          </thead>
          <tbody>
            <tr *ngFor="let item of balances">
              <td class="mono">{{ item.playerId | slice:0:8 }}</td>
              <td>{{ item.currencyType }}</td>
              <td>{{ item.balance | number }}</td>
              <td>{{ item.updatedAt | date:'yyyy-MM-dd HH:mm' }}</td>
            </tr>
          </tbody>
        </table>
        <p class="muted" *ngIf="!balances.length">暂无数据</p>
      </ng-container>

      <ng-container *ngIf="tab === 'ledgers'">
        <table>
          <thead>
            <tr>
              <th>玩家ID</th><th>货币类型</th><th>变化</th>
              <th>变动前</th><th>变动后</th><th>业务类型</th><th>业务ID</th><th>时间</th>
            </tr>
          </thead>
          <tbody>
            <tr *ngFor="let item of ledgers">
              <td class="mono">{{ item.playerId | slice:0:8 }}</td>
              <td>{{ item.currencyType }}</td>
              <td [class.danger]="item.amount < 0">{{ item.amount | number }}</td>
              <td>{{ item.balanceBefore | number }}</td>
              <td>{{ item.balanceAfter | number }}</td>
              <td>{{ item.bizType }}</td>
              <td class="mono">{{ item.bizId | slice:0:8 }}</td>
              <td>{{ item.createdAt | date:'yyyy-MM-dd HH:mm' }}</td>
            </tr>
          </tbody>
        </table>
        <p class="muted" *ngIf="!ledgers.length">暂无数据</p>
      </ng-container>
    </section>
  `
})
export class WalletPageComponent {
  private readonly api = inject(AdminApiService);
  tab: WalletTab = 'balances';
  balances: AdminWalletBalanceItem[] = [];
  ledgers: AdminWalletLedgerItem[] = [];
  adjustDraft: AdminAdjustWalletRequest = {
    playerId: '',
    currencyType: '',
    amount: 0,
    reason: ''
  };

  constructor() { this.load(); }

  switchTab(tab: WalletTab): void {
    this.tab = tab;
    this.load();
  }

  load(): void {
    if (this.tab === 'balances') {
      this.api.walletBalances(1, 50).subscribe({
        next: (page) => this.balances = page.items,
        error: () => this.balances = []
      });
    } else {
      this.api.walletLedgers(1, 50).subscribe({
        next: (page) => this.ledgers = page.items,
        error: () => this.ledgers = []
      });
    }
  }

  adjust(): void {
    if (!this.adjustDraft.playerId.trim()
      || !this.adjustDraft.currencyType.trim()
      || !this.adjustDraft.reason.trim()) {
      alert('玩家ID、货币类型和原因必填');
      return;
    }
    this.api.adjustWallet({
      ...this.adjustDraft,
      amount: Number(this.adjustDraft.amount)
    }).subscribe({
      next: () => {
        alert('调整成功');
        this.adjustDraft = { playerId: '', currencyType: '', amount: 0, reason: '' };
        this.load();
      },
      error: (err) => alert('调整失败：' + (err.message || err))
    });
  }
}
