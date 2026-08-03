import { CommonModule } from '@angular/common';
import { Component, inject } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { AdminApiService } from '../core/admin-api.service';
import { AdminCreateQuestRequest, AdminQuestItem } from '../core/models';

@Component({
  selector: 'dba-quests-page',
  standalone: true,
  imports: [CommonModule, FormsModule],
  template: `
    <section class="page">
      <div class="section-title">
        <div><span class="eyebrow">任务管理</span><h2>任务列表</h2></div>
        <button type="button" (click)="load()">刷新</button>
      </div>

      <form class="inline-form" (ngSubmit)="save()">
        <input name="questKey" [(ngModel)]="draft.questKey" placeholder="任务Key">
        <input name="title" [(ngModel)]="draft.title" placeholder="标题">
        <input name="questType" [(ngModel)]="draft.questType" placeholder="类型">
        <input name="category" [(ngModel)]="draft.category" placeholder="分类">
        <input name="targetProgress" [(ngModel)]="draft.targetProgress" type="number" min="1" placeholder="目标进度">
        <input name="sortOrder" [(ngModel)]="draft.sortOrder" type="number" min="0" placeholder="排序">
        <input name="description" [(ngModel)]="draft.description" placeholder="描述">
        <input name="rewardJson" [(ngModel)]="draft.rewardJson" placeholder="奖励JSON">
        <button type="submit">{{ editingId ? '更新' : '创建' }}</button>
        <button *ngIf="editingId" type="button" (click)="resetDraft()">取消</button>
      </form>

      <table>
        <thead>
          <tr>
            <th>任务Key</th><th>标题</th><th>类型</th><th>分类</th>
            <th>目标进度</th><th>排序</th><th>状态</th><th>创建时间</th><th>操作</th>
          </tr>
        </thead>
        <tbody>
          <tr *ngFor="let quest of quests">
            <td class="mono">{{ quest.questKey }}</td>
            <td>{{ quest.title }}</td>
            <td>{{ quest.questType }}</td>
            <td>{{ quest.category }}</td>
            <td>{{ quest.targetProgress }}</td>
            <td>{{ quest.sortOrder }}</td>
            <td><span class="badge" [class.ok]="quest.isActive">{{ quest.isActive ? '启用' : '停用' }}</span></td>
            <td>{{ quest.createdAt | date:'yyyy-MM-dd HH:mm' }}</td>
            <td>
              <button type="button" (click)="edit(quest)">编辑</button>
              <button *ngIf="quest.isActive" type="button" class="danger" (click)="deactivate(quest)">停用</button>
            </td>
          </tr>
        </tbody>
      </table>
      <p class="muted" *ngIf="!quests.length">暂无数据</p>
    </section>
  `
})
export class QuestsPageComponent {
  private readonly api = inject(AdminApiService);
  quests: AdminQuestItem[] = [];
  editingId: string | null = null;
  draft: AdminCreateQuestRequest = this.emptyDraft();

  constructor() { this.load(); }

  load(): void {
    this.api.quests().subscribe({
      next: (page) => this.quests = page.items,
      error: () => this.quests = []
    });
  }

  edit(quest: AdminQuestItem): void {
    this.editingId = quest.id;
    this.draft = {
      questKey: quest.questKey,
      title: quest.title,
      description: quest.description,
      questType: quest.questType,
      category: quest.category,
      targetProgress: quest.targetProgress,
      rewardJson: quest.rewardJson,
      sortOrder: quest.sortOrder
    };
  }

  resetDraft(): void {
    this.editingId = null;
    this.draft = this.emptyDraft();
  }

  save(): void {
    if (!this.draft.questKey.trim() || !this.draft.title.trim()) {
      alert('任务Key 和标题必填');
      return;
    }

    const request: AdminCreateQuestRequest = {
      ...this.draft,
      targetProgress: Number(this.draft.targetProgress),
      sortOrder: Number(this.draft.sortOrder)
    };

    if (this.editingId) {
      this.api.updateQuest(this.editingId, request).subscribe({
        next: () => { alert('更新成功'); this.resetDraft(); this.load(); },
        error: (err) => alert('更新失败：' + (err.message || err))
      });
    } else {
      this.api.createQuest(request).subscribe({
        next: () => { alert('创建成功'); this.resetDraft(); this.load(); },
        error: (err) => alert('创建失败：' + (err.message || err))
      });
    }
  }

  deactivate(quest: AdminQuestItem): void {
    const reason = prompt(`请输入停用任务 ${quest.questKey} 的原因：`);
    if (!reason) return;
    this.api.deactivateQuest(quest.id, reason).subscribe({
      next: () => { alert('已停用'); this.load(); },
      error: (err) => alert('停用失败：' + (err.message || err))
    });
  }

  private emptyDraft(): AdminCreateQuestRequest {
    return {
      questKey: '',
      title: '',
      description: '',
      questType: 'DAILY',
      category: 'general',
      targetProgress: 1,
      rewardJson: '{}',
      sortOrder: 0
    };
  }
}
