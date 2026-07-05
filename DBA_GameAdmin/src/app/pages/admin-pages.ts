import { CommonModule } from '@angular/common';
import { Component, inject } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { ActivatedRoute, RouterLink } from '@angular/router';
import { AdminApiService } from '../core/admin-api.service';
import {
  AdminAuditLogItem,
  ClientVersionItem,
  FeedbackItem,
  GameConfigItem,
  GameServerItem,
  InventoryLogItem,
  InventoryMutationRequest,
  LiveOpsStatus,
  MatchDetail,
  MatchListItem,
  PlatformApplication,
  PlayerDetail,
  PlayerListItem,
  SupportTicketItem,
  UpsertClientVersionRequest
} from '../core/models';

type LoadState = 'idle' | 'loading' | 'ready' | 'error';

function prettyJson(value: string): string {
  try {
    return JSON.stringify(JSON.parse(value), null, 2);
  } catch {
    return value;
  }
}

@Component({
  selector: 'dba-dashboard-page',
  standalone: true,
  imports: [CommonModule],
  template: `
    <section class="page">
      <div class="section-title">
        <div>
          <span class="eyebrow">Overview</span>
          <h2>运营总览</h2>
        </div>
        <button type="button" (click)="load()">刷新</button>
      </div>

      <div class="metrics" *ngIf="status">
        <article><span>账号</span><strong>{{ status.totalAccounts }}</strong></article>
        <article><span>玩家</span><strong>{{ status.totalPlayers }}</strong></article>
        <article><span>角色</span><strong>{{ status.totalCharacters }}</strong></article>
        <article><span>活跃服务器</span><strong>{{ status.activeGameServers }}</strong></article>
        <article><span>异常服务器</span><strong>{{ status.staleGameServers }}</strong></article>
        <article><span>未处理工单</span><strong>{{ status.openSupportTickets }}</strong></article>
        <article><span>最新版本</span><strong>{{ status.latestClientVersion || '-' }}</strong></article>
      </div>

      <div class="panel">
        <h3>健康状态</h3>
        <table>
          <thead><tr><th>项目</th><th>状态</th><th>详情</th></tr></thead>
          <tbody>
            <tr *ngFor="let item of status?.healthItems ?? []">
              <td>{{ item.name }}</td>
              <td><span class="badge" [class.ok]="item.status === 'Healthy'">{{ item.status }}</span></td>
              <td>{{ item.detail }}</td>
            </tr>
          </tbody>
        </table>
      </div>

      <p class="error" *ngIf="state === 'error'">无法读取运营状态，请确认 Game.Api 可用且当前账号有权限。</p>
    </section>
  `
})
export class DashboardPageComponent {
  private readonly api = inject(AdminApiService);
  state: LoadState = 'idle';
  status: LiveOpsStatus | null = null;

  constructor() {
    this.load();
  }

  load(): void {
    this.state = 'loading';
    this.api.liveOpsStatus().subscribe({
      next: (status) => {
        this.status = status;
        this.state = 'ready';
      },
      error: () => this.state = 'error'
    });
  }
}

@Component({
  selector: 'dba-players-page',
  standalone: true,
  imports: [CommonModule, RouterLink],
  template: `
    <section class="page">
      <div class="section-title">
        <div><span class="eyebrow">Players</span><h2>玩家管理</h2></div>
        <button type="button" (click)="load()">刷新</button>
      </div>
      <table>
        <thead>
          <tr><th>昵称</th><th>账号</th><th>状态</th><th>等级</th><th>角色</th><th>最后登录</th></tr>
        </thead>
        <tbody>
          <tr *ngFor="let player of players">
            <td><a [routerLink]="['/players', player.playerId]">{{ player.nickname }}</a></td>
            <td>{{ player.email || player.accountType }}</td>
            <td><span class="badge">{{ player.accountStatus }}</span></td>
            <td>{{ player.level }}</td>
            <td>{{ player.characterCount }} / {{ player.selectedCharacterName || '-' }}</td>
            <td>{{ player.lastLoginAt | date:'yyyy-MM-dd HH:mm' }}</td>
          </tr>
        </tbody>
      </table>
      <p class="muted">共 {{ totalCount }} 个玩家档案</p>
    </section>
  `
})
export class PlayersPageComponent {
  private readonly api = inject(AdminApiService);
  players: PlayerListItem[] = [];
  totalCount = 0;

  constructor() {
    this.load();
  }

  load(): void {
    this.api.players().subscribe((page) => {
      this.players = page.items;
      this.totalCount = page.totalCount;
    });
  }
}

@Component({
  selector: 'dba-player-detail-page',
  standalone: true,
  imports: [CommonModule],
  template: `
    <section class="page" *ngIf="player">
      <div class="section-title">
        <div><span class="eyebrow">Player</span><h2>{{ player.nickname }}</h2></div>
      </div>
      <div class="metrics compact">
        <article><span>等级</span><strong>{{ player.level }}</strong></article>
        <article><span>经验</span><strong>{{ player.exp }}</strong></article>
        <article><span>胜场</span><strong>{{ player.statistics.wins }}</strong></article>
        <article><span>KDA</span><strong>{{ player.statistics.kills }}/{{ player.statistics.deaths }}/{{ player.statistics.assists }}</strong></article>
      </div>
      <div class="two-column">
        <div class="panel">
          <h3>背包</h3>
          <table>
            <thead><tr><th>物品</th><th>数量</th><th>过期</th></tr></thead>
            <tbody><tr *ngFor="let item of player.inventory"><td>{{ item.itemId }}</td><td>{{ item.quantity }}</td><td>{{ item.expiresAt || '-' }}</td></tr></tbody>
          </table>
        </div>
        <div class="panel">
          <h3>解锁</h3>
          <table>
            <thead><tr><th>类型</th><th>ID</th><th>来源</th></tr></thead>
            <tbody><tr *ngFor="let unlock of player.unlocks"><td>{{ unlock.unlockType }}</td><td>{{ unlock.unlockId }}</td><td>{{ unlock.source }}</td></tr></tbody>
          </table>
        </div>
      </div>
    </section>
  `
})
export class PlayerDetailPageComponent {
  private readonly api = inject(AdminApiService);
  private readonly route = inject(ActivatedRoute);
  player: PlayerDetail | null = null;

  constructor() {
    const playerId = this.route.snapshot.paramMap.get('playerId');
    if (playerId) {
      this.api.player(playerId).subscribe((player) => this.player = player);
    }
  }
}

@Component({
  selector: 'dba-servers-page',
  standalone: true,
  imports: [CommonModule, FormsModule],
  template: `
    <section class="page">
      <div class="section-title">
        <div><span class="eyebrow">Dedicated Servers</span><h2>游戏服务器</h2></div>
        <button type="button" (click)="load()">刷新</button>
      </div>
      <table>
        <thead><tr><th>状态</th><th>地图</th><th>区服</th><th>地址</th><th>会话</th><th>心跳</th><th>操作</th></tr></thead>
        <tbody>
          <tr *ngFor="let server of servers">
            <td><span class="badge">{{ server.status }}</span></td>
            <td>{{ server.mode || '-' }} / {{ server.mapId || '-' }}</td>
            <td>{{ server.region || '-' }}</td>
            <td>{{ server.ip }}:{{ server.port }}</td>
            <td class="mono">{{ server.sessionId || '-' }}</td>
            <td>{{ server.lastHeartbeatAt | date:'HH:mm:ss' }}</td>
            <td><button type="button" class="danger" (click)="kill(server)">Kill</button></td>
          </tr>
        </tbody>
      </table>
    </section>
  `
})
export class ServersPageComponent {
  private readonly api = inject(AdminApiService);
  servers: GameServerItem[] = [];

  constructor() {
    this.load();
  }

  load(): void {
    this.api.servers().subscribe((page) => this.servers = page.items);
  }

  kill(server: GameServerItem): void {
    const reason = prompt(`请输入 Kill 服务器 ${server.id} 的原因`);
    if (!reason?.trim()) {
      return;
    }
    this.api.killServer(server.id, reason.trim()).subscribe(() => this.load());
  }
}

@Component({
  selector: 'dba-matches-page',
  standalone: true,
  imports: [CommonModule, RouterLink],
  template: `
    <section class="page">
      <div class="section-title"><div><span class="eyebrow">Matches</span><h2>对局记录</h2></div></div>
      <table>
        <thead><tr><th>对局</th><th>模式</th><th>地图</th><th>玩家</th><th>时长</th><th>结果</th><th>创建时间</th></tr></thead>
        <tbody>
          <tr *ngFor="let match of matches">
            <td><a [routerLink]="['/matches', match.id]" class="mono">{{ match.id }}</a></td>
            <td>{{ match.mode }}</td>
            <td>{{ match.mapId }}</td>
            <td>{{ match.playerCount }}</td>
            <td>{{ match.durationSeconds }}s</td>
            <td>{{ formatResultSummary(match) }}</td>
            <td>{{ match.createdAt | date:'yyyy-MM-dd HH:mm' }}</td>
          </tr>
        </tbody>
      </table>
    </section>
  `
})
export class MatchesPageComponent {
  private readonly api = inject(AdminApiService);
  matches: MatchListItem[] = [];

  constructor() {
    this.api.matches().subscribe((page) => this.matches = page.items);
  }

  formatResultSummary(match: MatchListItem): string {
    const resultJson = match.resultJson;
    let winnerTeam = match.winnerTeam?.trim() || '-';
    let schema = '';

    try {
      const parsed = JSON.parse(resultJson) as { winnerTeam?: unknown; winner_team?: unknown; schema?: unknown };
      if (winnerTeam === '-') {
        const rawWinnerTeam = typeof parsed.winnerTeam === 'string' && parsed.winnerTeam.trim().length > 0
          ? parsed.winnerTeam
          : parsed.winner_team;
        winnerTeam = typeof rawWinnerTeam === 'string' && rawWinnerTeam.trim().length > 0
          ? rawWinnerTeam.trim()
          : '-';
      }
      schema = typeof parsed.schema === 'string' && parsed.schema.trim().length > 0
        ? parsed.schema.trim()
        : '';
    } catch {
      return winnerTeam;
    }

    return schema ? `${winnerTeam} (${schema})` : winnerTeam;
  }
}

@Component({
  selector: 'dba-match-detail-page',
  standalone: true,
  imports: [CommonModule],
  template: `
    <section class="page" *ngIf="match">
      <div class="section-title"><div><span class="eyebrow">Match</span><h2>{{ match.mode }} / {{ match.mapId }}</h2></div></div>
      <div class="metrics compact">
        <article><span>对局</span><strong class="mono">{{ match.id }}</strong></article>
        <article><span>会话</span><strong class="mono">{{ match.sessionId }}</strong></article>
        <article class="team-outcome-winner"><span>胜方队伍</span><strong>{{ formatTeamOutcome(match) }}</strong></article>
        <article class="team-outcome-distribution"><span>队伍分布</span><strong>{{ formatTeamDistribution(match.players, match.teamDistribution) }}</strong></article>
        <article><span>时长</span><strong>{{ match.durationSeconds }}s</strong></article>
        <article><span>创建时间</span><strong>{{ match.createdAt | date:'yyyy-MM-dd HH:mm' }}</strong></article>
      </div>
      <div class="panel">
        <h3>玩家结果</h3>
        <table>
          <thead><tr><th>玩家</th><th>队伍</th><th>结果</th><th>K/D/A</th><th>分数</th><th>经验</th><th>奖励</th></tr></thead>
          <tbody>
            <tr *ngFor="let player of match.players">
              <td class="mono">{{ player.playerId }}</td>
              <td>{{ player.team || '-' }}</td>
              <td>{{ player.result }}</td>
              <td>{{ player.kills }}/{{ player.deaths }}/{{ player.assists }}</td>
              <td>{{ player.score }}</td>
              <td>{{ player.expDelta }}</td>
              <td>{{ formatRewards(player.rewards) }}</td>
            </tr>
          </tbody>
        </table>
      </div>
      <pre>{{ prettyResult }}</pre>
    </section>
  `
})
export class MatchDetailPageComponent {
  private readonly api = inject(AdminApiService);
  private readonly route = inject(ActivatedRoute);
  match: MatchDetail | null = null;
  prettyResult = '';

  constructor() {
    const matchId = this.route.snapshot.paramMap.get('matchId');
    if (matchId) {
      this.api.match(matchId).subscribe((match) => {
        this.match = match;
        this.prettyResult = prettyJson(match.resultJson);
      });
    }
  }

  formatRewards(rewards: Record<string, unknown> | null | undefined): string {
    const entries = Object.entries(rewards ?? {});
    if (entries.length === 0) {
      return '-';
    }

    return entries
      .sort(([left], [right]) => left.localeCompare(right))
      .map(([key, value]) => `${key}: ${String(value)}`)
      .join(', ');
  }

  formatTeamOutcome(match: MatchDetail): string {
    return match.winnerTeam?.trim() || this.extractWinnerTeam(match.resultJson);
  }

  formatTeamDistribution(players: MatchDetail['players'], teamDistribution?: Record<string, number> | null): string {
    const distributionEntries = Object.entries(teamDistribution ?? {});
    if (distributionEntries.length > 0) {
      return distributionEntries
        .sort(([left], [right]) => left.localeCompare(right))
        .map(([team, count]) => `${team}: ${count}`)
        .join(', ');
    }

    const counts = new Map<string, number>();
    for (const player of players) {
      const team = player.team?.trim() || '-';
      counts.set(team, (counts.get(team) ?? 0) + 1);
    }

    return Array.from(counts.entries())
      .sort(([left], [right]) => left.localeCompare(right))
      .map(([team, count]) => `${team}: ${count}`)
      .join(', ') || '-';
  }

  extractWinnerTeam(resultJson: string | null | undefined): string {
    if (!resultJson) {
      return '-';
    }

    try {
      const parsed = JSON.parse(resultJson) as { winnerTeam?: unknown; winner_team?: unknown };
      const rawWinnerTeam = typeof parsed.winnerTeam === 'string' && parsed.winnerTeam.trim().length > 0
        ? parsed.winnerTeam
        : parsed.winner_team;
      return typeof rawWinnerTeam === 'string' && rawWinnerTeam.trim().length > 0
        ? rawWinnerTeam.trim()
        : '-';
    } catch {
      return '-';
    }
  }
}

@Component({
  selector: 'dba-configs-page',
  standalone: true,
  imports: [CommonModule],
  template: `
    <section class="page">
      <div class="section-title"><div><span class="eyebrow">Configs</span><h2>游戏配置</h2></div></div>
      <table>
        <thead><tr><th>Key</th><th>版本</th><th>状态</th><th>渠道</th><th>区域</th><th>发布时间</th></tr></thead>
        <tbody>
          <tr *ngFor="let config of configs">
            <td>{{ config.configKey }}</td>
            <td>{{ config.version }}</td>
            <td><span class="badge">{{ config.status }}</span></td>
            <td>{{ config.channel }}</td>
            <td>{{ config.region }}</td>
            <td>{{ config.publishedAt | date:'yyyy-MM-dd HH:mm' }}</td>
          </tr>
        </tbody>
      </table>
    </section>
  `
})
export class ConfigsPageComponent {
  private readonly api = inject(AdminApiService);
  configs: GameConfigItem[] = [];

  constructor() {
    this.api.configs().subscribe((configs) => this.configs = configs);
  }
}

@Component({
  selector: 'dba-client-versions-page',
  standalone: true,
  imports: [CommonModule, FormsModule],
  template: `
    <section class="page">
      <div class="section-title">
        <div><span class="eyebrow">Client Versions</span><h2>客户端版本</h2></div>
        <button type="button" (click)="load()">刷新</button>
      </div>
      <form class="inline-form" (ngSubmit)="saveVersion()">
        <input name="version" [(ngModel)]="draft.version" placeholder="版本">
        <input name="channel" [(ngModel)]="draft.channel" placeholder="渠道">
        <input name="platform" [(ngModel)]="draft.platform" placeholder="平台">
        <input name="downloadUrl" [(ngModel)]="draft.downloadUrl" placeholder="下载地址">
        <input name="checksum" [(ngModel)]="draft.checksum" placeholder="Checksum">
        <input name="sizeBytes" [(ngModel)]="draft.sizeBytes" type="number" placeholder="大小">
        <label><input name="mandatory" [(ngModel)]="draft.isMandatory" type="checkbox"> 强制</label>
        <label><input name="active" [(ngModel)]="draft.isActive" type="checkbox"> 激活</label>
        <input name="reason" [(ngModel)]="draft.reason" placeholder="变更原因">
        <button type="submit">保存</button>
      </form>
      <table>
        <thead><tr><th>版本</th><th>渠道</th><th>平台</th><th>大小</th><th>强制</th><th>激活</th><th>创建时间</th></tr></thead>
        <tbody>
          <tr *ngFor="let item of versions">
            <td>{{ item.version }}</td><td>{{ item.channel }}</td><td>{{ item.platform }}</td>
            <td>{{ item.sizeBytes | number }}</td><td>{{ item.isMandatory ? '是' : '否' }}</td>
            <td><span class="badge" [class.ok]="item.isActive">{{ item.isActive ? 'ACTIVE' : 'INACTIVE' }}</span></td>
            <td>{{ item.createdAt | date:'yyyy-MM-dd HH:mm' }}</td>
          </tr>
        </tbody>
      </table>
    </section>
  `
})
export class ClientVersionsPageComponent {
  private readonly api = inject(AdminApiService);
  versions: ClientVersionItem[] = [];
  draft: UpsertClientVersionRequest = {
    version: '',
    channel: 'stable',
    platform: 'windows',
    downloadUrl: '',
    checksum: '',
    sizeBytes: 1,
    isMandatory: false,
    isActive: false,
    minOsVersion: null,
    releaseNotes: null,
    reason: ''
  };

  constructor() {
    this.load();
  }

  load(): void {
    this.api.clientVersions().subscribe((page) => this.versions = page.items);
  }

  saveVersion(): void {
    this.api.upsertClientVersion(this.draft).subscribe(() => this.load());
  }
}

@Component({
  selector: 'dba-inventory-page',
  standalone: true,
  imports: [CommonModule, FormsModule],
  template: `
    <section class="page">
      <div class="section-title">
        <div><span class="eyebrow">Inventory</span><h2>背包管理</h2></div>
        <button type="button" (click)="load()">刷新日志</button>
      </div>
      <form class="inline-form" (ngSubmit)="mutate('grant')">
        <input name="playerId" [(ngModel)]="draft.playerId" placeholder="PlayerId">
        <input name="itemId" [(ngModel)]="draft.itemId" placeholder="ItemId">
        <input name="quantity" [(ngModel)]="draft.quantity" type="number" min="1" placeholder="数量">
        <input name="reason" [(ngModel)]="draft.reason" placeholder="高危操作原因">
        <button type="submit">发放</button>
        <button type="button" class="danger" (click)="mutate('deduct')">扣除</button>
      </form>
      <table>
        <thead><tr><th>玩家</th><th>物品</th><th>变化</th><th>原因</th><th>时间</th></tr></thead>
        <tbody>
          <tr *ngFor="let item of logs">
            <td class="mono">{{ item.playerId }}</td>
            <td>{{ item.itemId }}</td>
            <td>{{ item.quantityDelta }}</td>
            <td>{{ item.reason }}</td>
            <td>{{ item.createdAt | date:'yyyy-MM-dd HH:mm:ss' }}</td>
          </tr>
        </tbody>
      </table>
    </section>
  `
})
export class InventoryPageComponent {
  private readonly api = inject(AdminApiService);
  logs: InventoryLogItem[] = [];
  draft: InventoryMutationRequest = {
    playerId: '',
    itemId: '',
    quantity: 1,
    reason: ''
  };

  constructor() {
    this.load();
  }

  load(): void {
    this.api.inventoryLogs().subscribe((logs) => this.logs = logs);
  }

  mutate(kind: 'grant' | 'deduct'): void {
    if (!this.draft.playerId.trim() || !this.draft.itemId.trim() || !this.draft.reason.trim()) {
      alert('PlayerId、ItemId 和原因必填');
      return;
    }

    const request = { ...this.draft, quantity: Number(this.draft.quantity) };
    const action = kind === 'grant'
      ? this.api.grantInventoryItem(request)
      : this.api.deductInventoryItem(request);

    action.subscribe(() => this.load());
  }
}

@Component({
  selector: 'dba-feedback-page',
  standalone: true,
  imports: [CommonModule],
  template: `
    <section class="page">
      <div class="section-title"><div><span class="eyebrow">Feedback</span><h2>玩家反馈</h2></div></div>
      <table>
        <thead><tr><th>类型</th><th>标题</th><th>玩家</th><th>状态</th><th>创建时间</th></tr></thead>
        <tbody><tr *ngFor="let item of items"><td>{{ item.feedbackType }}</td><td>{{ item.title || '-' }}</td><td>{{ item.nickname || item.email || '-' }}</td><td><span class="badge">{{ item.status }}</span></td><td>{{ item.createdAt | date:'yyyy-MM-dd HH:mm' }}</td></tr></tbody>
      </table>
    </section>
  `
})
export class FeedbackPageComponent {
  private readonly api = inject(AdminApiService);
  items: FeedbackItem[] = [];

  constructor() {
    this.api.feedback().subscribe((page) => this.items = page.items);
  }
}

@Component({
  selector: 'dba-support-tickets-page',
  standalone: true,
  imports: [CommonModule],
  template: `
    <section class="page">
      <div class="section-title"><div><span class="eyebrow">Support</span><h2>客服工单</h2></div></div>
      <table>
        <thead><tr><th>优先级</th><th>类型</th><th>主题</th><th>玩家</th><th>状态</th><th>创建时间</th></tr></thead>
        <tbody><tr *ngFor="let item of items"><td>{{ item.priority }}</td><td>{{ item.ticketType }}</td><td>{{ item.subject }}</td><td>{{ item.nickname || '-' }}</td><td><span class="badge">{{ item.status }}</span></td><td>{{ item.createdAt | date:'yyyy-MM-dd HH:mm' }}</td></tr></tbody>
      </table>
    </section>
  `
})
export class SupportTicketsPageComponent {
  private readonly api = inject(AdminApiService);
  items: SupportTicketItem[] = [];

  constructor() {
    this.api.supportTickets().subscribe((page) => this.items = page.items);
  }
}

@Component({
  selector: 'dba-audit-logs-page',
  standalone: true,
  imports: [CommonModule],
  template: `
    <section class="page">
      <div class="section-title"><div><span class="eyebrow">Audit</span><h2>审计日志</h2></div></div>
      <table>
        <thead><tr><th>操作</th><th>管理员</th><th>目标</th><th>原因</th><th>IP</th><th>时间</th></tr></thead>
        <tbody><tr *ngFor="let item of logs"><td>{{ item.action }}</td><td>{{ item.adminUsername || '-' }}</td><td>{{ item.targetType }} / {{ item.targetId || '-' }}</td><td>{{ item.reason || '-' }}</td><td>{{ item.ipAddress || '-' }}</td><td>{{ item.createdAt | date:'yyyy-MM-dd HH:mm:ss' }}</td></tr></tbody>
      </table>
    </section>
  `
})
export class AuditLogsPageComponent {
  private readonly api = inject(AdminApiService);
  logs: AdminAuditLogItem[] = [];

  constructor() {
    this.api.auditLogs().subscribe((page) => this.logs = page.items);
  }
}

@Component({
  selector: 'dba-platform-page',
  standalone: true,
  imports: [CommonModule],
  template: `
    <section class="page">
      <div class="section-title"><div><span class="eyebrow">Platform</span><h2>应用结构</h2></div></div>
      <div class="app-grid">
        <article class="panel" *ngFor="let app of applications">
          <h3>{{ app.name }}</h3>
          <p>{{ app.goal }}</p>
          <dl>
            <dt>目录</dt><dd>{{ app.directory }}</dd>
            <dt>运行时</dt><dd>{{ app.runtime }}</dd>
            <dt>状态</dt><dd><span class="badge">{{ app.status }}</span></dd>
            <dt>命令</dt><dd class="mono">{{ app.runCommand }}</dd>
          </dl>
        </article>
      </div>
    </section>
  `
})
export class PlatformPageComponent {
  private readonly api = inject(AdminApiService);
  applications: PlatformApplication[] = [];

  constructor() {
    this.api.platformApplications().subscribe((payload) => this.applications = payload.applications);
  }
}
