import { CommonModule } from '@angular/common';
import { Component, inject } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { AdminApiService } from '../core/admin-api.service';
import { AdminPaymentOrderItem } from '../core/models';

@Component({
  selector: 'dba-payment-orders-page',
  standalone: true,
  imports: [CommonModule, FormsModule],
  template: `
    <section class="page">
      <div class="section-title">
        <div><span class="eyebrow">支付管理</span><h2>支付订单</h2></div>
        <button type="button" (click)="load()">刷新</button>
      </div>
      <div class="inline-form">
        <label>状态筛选：</label>
        <select [(ngModel)]="statusFilter" (ngModelChange)="load()">
          <option value="">全部</option>
          <option value="PENDING">待支付</option>
          <option value="PAID">已支付</option>
          <option value="FAILED">失败</option>
          <option value="REFUNDED">已退款</option>
        </select>
      </div>
      <table>
        <thead>
          <tr>
            <th>订单ID</th><th>玩家ID</th><th>平台</th><th>状态</th>
            <th>金额</th><th>商品</th><th>虚拟币</th><th>创建时间</th><th>支付时间</th><th>操作</th>
          </tr>
        </thead>
        <tbody>
          <tr *ngFor="let order of orders">
            <td class="mono">{{ order.id | slice:0:8 }}</td>
            <td class="mono">{{ order.playerId | slice:0:8 }}</td>
            <td>{{ order.platform }}</td>
            <td><span class="badge" [class.ok]="order.status === 'PAID'" [class.danger]="order.status === 'FAILED'">{{ order.status }}</span></td>
            <td>{{ order.amount / 100 | number:'1.2-2' }} {{ order.currency }}</td>
            <td>{{ order.productName }}</td>
            <td>{{ order.virtualAmount }} {{ order.virtualCurrency }}</td>
            <td>{{ order.createdAt | date:'yyyy-MM-dd HH:mm' }}</td>
            <td>{{ order.paidAt | date:'yyyy-MM-dd HH:mm' }}</td>
            <td>
              <button *ngIf="order.status === 'PAID'" type="button" class="danger" (click)="refund(order.id)">退款</button>
            </td>
          </tr>
        </tbody>
      </table>
      <p class="muted" *ngIf="!orders.length">暂无数据</p>
    </section>
  `
})
export class PaymentOrdersPageComponent {
  private readonly api = inject(AdminApiService);
  orders: AdminPaymentOrderItem[] = [];
  statusFilter = '';

  constructor() { this.load(); }

  load(): void {
    this.api.paymentOrders(1, 50, this.statusFilter).subscribe({
      next: (page) => this.orders = page.items,
      error: () => this.orders = []
    });
  }

  refund(orderId: string): void {
    const reason = prompt('请输入退款原因：');
    if (!reason) return;
    this.api.refundPayment(orderId, { reason }).subscribe({
      next: () => { alert('退款成功'); this.load(); },
      error: (err) => alert('退款失败：' + (err.message || err))
    });
  }
}
