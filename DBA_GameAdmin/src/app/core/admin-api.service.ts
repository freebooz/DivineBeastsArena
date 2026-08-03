import { HttpClient, HttpParams } from '@angular/common/http';
import { Injectable, inject } from '@angular/core';
import { Observable, map } from 'rxjs';
import { environment } from '../../environments/environment';
import {
  AdminAdjustWalletRequest,
  AdminAuditLogItem,
  AdminCreateQuestRequest,
  AdminLoginRequest,
  AdminLoginResponse,
  AdminPaymentOrderItem,
  AdminPaymentOrderListResponse,
  AdminProfile,
  AdminQuestListResponse,
  AdminRefundRequest,
  AdminWalletBalanceListResponse,
  AdminWalletLedgerListResponse,
  ApiEnvelope,
  ClientVersionItem,
  FeedbackItem,
  GameConfigItem,
  GameServerItem,
  InventoryLogItem,
  InventoryMutationRequest,
  LiveOpsStatus,
  MatchDetail,
  MatchListItem,
  PagedResponse,
  PlatformApplications,
  PlayerDetail,
  PlayerListItem,
  SupportTicketItem,
  UpsertClientVersionRequest
} from './models';

@Injectable({ providedIn: 'root' })
export class AdminApiService {
  private readonly http = inject(HttpClient);
  private readonly baseUrl = environment.apiBaseUrl;

  login(request: AdminLoginRequest): Observable<AdminLoginResponse> {
    return this.post<AdminLoginResponse>('/api/admin/auth/login', request);
  }

  me(): Observable<AdminProfile> {
    return this.get<AdminProfile>('/api/admin/me');
  }

  liveOpsStatus(): Observable<LiveOpsStatus> {
    return this.get<LiveOpsStatus>('/api/live-ops/status');
  }

  platformApplications(): Observable<PlatformApplications> {
    return this.get<PlatformApplications>('/api/platform/applications');
  }

  players(page = 1, pageSize = 50): Observable<PagedResponse<PlayerListItem>> {
    return this.get<PagedResponse<PlayerListItem>>('/api/admin/players', { page, pageSize });
  }

  player(playerId: string): Observable<PlayerDetail> {
    return this.get<PlayerDetail>(`/api/admin/players/${playerId}`);
  }

  servers(page = 1, pageSize = 50, status = ''): Observable<PagedResponse<GameServerItem>> {
    return this.get<PagedResponse<GameServerItem>>('/api/admin/servers', { page, pageSize, status });
  }

  killServer(serverId: string, reason: string): Observable<void> {
    return this.post<void>(`/api/admin/servers/${serverId}/kill`, { reason });
  }

  matches(page = 1, pageSize = 50): Observable<PagedResponse<MatchListItem>> {
    return this.get<PagedResponse<MatchListItem>>('/api/admin/matches', { page, pageSize });
  }

  match(matchId: string): Observable<MatchDetail> {
    return this.get<MatchDetail>(`/api/admin/matches/${matchId}`);
  }

  auditLogs(page = 1, pageSize = 50): Observable<PagedResponse<AdminAuditLogItem>> {
    return this.get<PagedResponse<AdminAuditLogItem>>('/api/admin/audit-logs', { page, pageSize });
  }

  feedback(page = 1, pageSize = 50, status = ''): Observable<PagedResponse<FeedbackItem>> {
    return this.get<PagedResponse<FeedbackItem>>('/api/admin/feedback', { page, pageSize, status });
  }

  supportTickets(page = 1, pageSize = 50, status = ''): Observable<PagedResponse<SupportTicketItem>> {
    return this.get<PagedResponse<SupportTicketItem>>('/api/admin/support/tickets', { page, pageSize, status });
  }

  clientVersions(page = 1, pageSize = 50): Observable<PagedResponse<ClientVersionItem>> {
    return this.get<PagedResponse<ClientVersionItem>>('/api/admin/client-versions', { page, pageSize });
  }

  upsertClientVersion(request: UpsertClientVersionRequest): Observable<ClientVersionItem> {
    return this.post<ClientVersionItem>('/api/admin/client-versions', request);
  }

  configs(): Observable<GameConfigItem[]> {
    return this.get<GameConfigItem[]>('/api/admin/configs');
  }

  inventoryLogs(page = 1, pageSize = 50): Observable<InventoryLogItem[]> {
    return this.get<InventoryLogItem[]>('/api/admin/inventory/logs', { page, pageSize });
  }

  grantInventoryItem(request: InventoryMutationRequest): Observable<void> {
    return this.post<void>('/api/admin/inventory/grant', request);
  }

  deductInventoryItem(request: InventoryMutationRequest): Observable<void> {
    return this.post<void>('/api/admin/inventory/deduct', request);
  }

  // 支付订单管理
  paymentOrders(page = 1, pageSize = 50, status = '', playerId = ''): Observable<AdminPaymentOrderListResponse> {
    return this.get<AdminPaymentOrderListResponse>('/api/admin/payments/orders', { page, pageSize, status, playerId });
  }

  paymentOrderDetail(orderId: string): Observable<AdminPaymentOrderItem> {
    return this.get<AdminPaymentOrderItem>(`/api/admin/payments/orders/${orderId}`);
  }

  refundPayment(orderId: string, req: AdminRefundRequest): Observable<void> {
    return this.post<void>(`/api/admin/payments/orders/${orderId}/refund`, req);
  }

  // 任务管理
  quests(): Observable<AdminQuestListResponse> {
    return this.get<AdminQuestListResponse>('/api/admin/quests');
  }

  createQuest(req: AdminCreateQuestRequest): Observable<{ questId: string }> {
    return this.post<{ questId: string }>('/api/admin/quests', req);
  }

  updateQuest(questId: string, req: AdminCreateQuestRequest): Observable<void> {
    return this.put<void>(`/api/admin/quests/${questId}`, req);
  }

  deactivateQuest(questId: string, reason: string): Observable<void> {
    return this.post<void>(`/api/admin/quests/${questId}/deactivate`, { reason });
  }

  // 钱包管理
  walletBalances(page = 1, pageSize = 50, playerId = ''): Observable<AdminWalletBalanceListResponse> {
    return this.get<AdminWalletBalanceListResponse>('/api/admin/wallet/balances', { page, pageSize, playerId });
  }

  walletLedgers(page = 1, pageSize = 50, playerId = '', bizType = ''): Observable<AdminWalletLedgerListResponse> {
    return this.get<AdminWalletLedgerListResponse>('/api/admin/wallet/ledgers', { page, pageSize, playerId, bizType });
  }

  adjustWallet(req: AdminAdjustWalletRequest): Observable<void> {
    return this.post<void>('/api/admin/wallet/adjust', req);
  }

  private get<T>(path: string, query?: Record<string, string | number | boolean>): Observable<T> {
    return this.http.get<ApiEnvelope<T> | T>(this.buildUrl(path), { params: this.buildParams(query) })
      .pipe(map((payload) => this.unwrap(payload)));
  }

  private post<T>(path: string, body: unknown): Observable<T> {
    return this.http.post<ApiEnvelope<T> | T>(this.buildUrl(path), body).pipe(map((payload) => this.unwrap(payload)));
  }

  private put<T>(path: string, body: unknown): Observable<T> {
    return this.http.put<ApiEnvelope<T> | T>(this.buildUrl(path), body).pipe(map((payload) => this.unwrap(payload)));
  }

  private buildUrl(path: string): string {
    return `${this.baseUrl}${path}`;
  }

  private buildParams(query?: Record<string, string | number | boolean>): HttpParams {
    let params = new HttpParams();
    for (const [key, value] of Object.entries(query ?? {})) {
      if (value !== '' && value !== null && value !== undefined) {
        params = params.set(key, String(value));
      }
    }
    return params;
  }

  private unwrap<T>(payload: ApiEnvelope<T> | T): T {
    const maybeEnvelope = payload as ApiEnvelope<T>;
    if (typeof maybeEnvelope === 'object' && maybeEnvelope !== null && 'success' in maybeEnvelope) {
      if (maybeEnvelope.success === false) {
        throw new Error(maybeEnvelope.message || maybeEnvelope.error || 'API request failed');
      }
      return maybeEnvelope.data as T;
    }

    return payload as T;
  }
}
