export interface ApiEnvelope<T> {
  success?: boolean;
  data?: T;
  error?: string;
  message?: string;
}

export interface AdminLoginRequest {
  username: string;
  password: string;
}

export interface AdminLoginResponse {
  accessToken: string;
  adminId: string;
  username: string;
  role: string;
}

export interface AdminProfile {
  adminId: string;
  username: string;
  role: string;
  lastLoginAt?: string | null;
}

export interface PagedResponse<T> {
  items: T[];
  totalCount: number;
  page: number;
  pageSize: number;
}

export interface PlayerListItem {
  playerId: string;
  nickname: string;
  accountId?: string | null;
  accountType: string;
  email?: string | null;
  accountStatus: string;
  level: number;
  exp: number;
  createdAt: string;
  lastLoginAt?: string | null;
  characterCount: number;
  selectedCharacterName?: string | null;
}

export interface PlayerDetail extends PlayerListItem {
  status: string;
  statistics: PlayerStatistics;
  inventory: InventoryItem[];
  unlocks: PlayerUnlock[];
}

export interface PlayerStatistics {
  totalMatches: number;
  wins: number;
  losses: number;
  draws: number;
  kills: number;
  deaths: number;
  assists: number;
  score: number;
  playTimeSeconds: number;
}

export interface InventoryItem {
  id: string;
  itemId: string;
  quantity: number;
  expiresAt?: string | null;
}

export interface InventoryLogItem {
  id: string;
  playerId: string;
  itemId: string;
  quantityDelta: number;
  reason: string;
  createdAt: string;
}

export interface InventoryMutationRequest {
  playerId: string;
  itemId: string;
  quantity: number;
  reason: string;
}

export interface PlayerUnlock {
  unlockType: string;
  unlockId: string;
  source: string;
  createdAt: string;
}

export interface GameServerItem {
  id: string;
  sessionId?: string | null;
  mode?: string | null;
  mapId?: string | null;
  region?: string | null;
  buildVersion?: string | null;
  ip: string;
  port: number;
  status: string;
  startedAt: string;
  lastHeartbeatAt?: string | null;
  endedAt?: string | null;
}

export interface MatchListItem {
  id: string;
  sessionId: string;
  mode: string;
  mapId: string;
  durationSeconds: number;
  playerCount: number;
  resultJson: string;
  winnerTeam?: string | null;
  createdAt: string;
}

export interface MatchDetail extends MatchListItem {
  winnerTeam?: string | null;
  teamDistribution?: Record<string, number>;
  players: MatchPlayerItem[];
}

export interface MatchPlayerItem {
  playerId: string;
  team?: string | null;
  result: string;
  kills: number;
  deaths: number;
  assists: number;
  score: number;
  expDelta: number;
  rewards: Record<string, unknown>;
}

export interface AdminAuditLogItem {
  id: string;
  adminUserId?: string | null;
  adminUsername?: string | null;
  action: string;
  targetType: string;
  targetId?: string | null;
  reason?: string | null;
  ipAddress?: string | null;
  createdAt: string;
}

export interface FeedbackItem {
  id: string;
  playerId?: string | null;
  nickname?: string | null;
  email?: string | null;
  feedbackType: string;
  title?: string | null;
  status: string;
  createdAt: string;
  updatedAt?: string | null;
}

export interface SupportTicketItem {
  id: string;
  playerId?: string | null;
  nickname?: string | null;
  ticketType: string;
  subject: string;
  status: string;
  priority: string;
  createdAt: string;
  updatedAt?: string | null;
}

export interface ClientVersionItem {
  id: string;
  version: string;
  channel: string;
  platform: string;
  downloadUrl: string;
  checksum: string;
  sizeBytes: number;
  isMandatory: boolean;
  isActive: boolean;
  minOsVersion?: string | null;
  releaseNotes?: string | null;
  createdAt: string;
}

export interface UpsertClientVersionRequest {
  version: string;
  channel: string;
  platform: string;
  downloadUrl: string;
  checksum: string;
  sizeBytes: number;
  isMandatory: boolean;
  isActive: boolean;
  minOsVersion?: string | null;
  releaseNotes?: string | null;
  reason: string;
}

export interface GameConfigItem {
  id: string;
  configKey: string;
  version: string;
  contentJson: string;
  status: string;
  checksum: string;
  channel: string;
  region: string;
  minClientVersion?: string | null;
  maxClientVersion?: string | null;
  createdAt: string;
  publishedAt?: string | null;
}

export interface LiveOpsStatus {
  generatedAt: string;
  totalAccounts: number;
  totalPlayers: number;
  totalCharacters: number;
  activeGameServers: number;
  staleGameServers: number;
  openSupportTickets: number;
  openReports: number;
  activeAnnouncements: number;
  activeEvents: number;
  latestClientVersion: string;
  healthItems: LiveOpsHealthItem[];
}

export interface LiveOpsHealthItem {
  name: string;
  status: string;
  detail: string;
}

export interface PlatformApplications {
  generatedAt: string;
  applications: PlatformApplication[];
}

export interface PlatformApplication {
  id: string;
  name: string;
  category: string;
  directory: string;
  runtime: string;
  goal: string;
  status: string;
  runCommand: string;
  healthCheck: string;
  responsibilities: string[];
  integrationPoints: string[];
  nextSteps: string[];
}
