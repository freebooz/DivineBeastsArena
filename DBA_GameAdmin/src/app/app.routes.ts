import { Routes } from '@angular/router';
import { authGuard } from './core/auth.guard';
import { AdminShellComponent } from './layout/admin-shell.component';
import {
  AuditLogsPageComponent,
  ClientVersionsPageComponent,
  ConfigsPageComponent,
  DashboardPageComponent,
  FeedbackPageComponent,
  InventoryPageComponent,
  MatchDetailPageComponent,
  MatchesPageComponent,
  PlatformPageComponent,
  PlayerDetailPageComponent,
  PlayersPageComponent,
  ServersPageComponent,
  SupportTicketsPageComponent
} from './pages/admin-pages';
import { LoginPageComponent } from './pages/login-page.component';
import { PaymentOrdersPageComponent } from './pages/payment-orders-page.component';
import { QuestsPageComponent } from './pages/quests-page.component';
import { WalletPageComponent } from './pages/wallet-page.component';

export const routes: Routes = [
  { path: 'login', component: LoginPageComponent },
  {
    path: '',
    component: AdminShellComponent,
    canActivate: [authGuard],
    canActivateChild: [authGuard],
    children: [
      { path: '', pathMatch: 'full', redirectTo: 'dashboard' },
      { path: 'dashboard', component: DashboardPageComponent },
      { path: 'players', component: PlayersPageComponent },
      { path: 'players/:playerId', component: PlayerDetailPageComponent },
      { path: 'matches', component: MatchesPageComponent },
      { path: 'matches/:matchId', component: MatchDetailPageComponent },
      { path: 'servers', component: ServersPageComponent },
      { path: 'inventory', component: InventoryPageComponent },
      { path: 'configs', component: ConfigsPageComponent },
      { path: 'client-versions', component: ClientVersionsPageComponent },
      { path: 'feedback', component: FeedbackPageComponent },
      { path: 'support', component: SupportTicketsPageComponent },
      { path: 'audit', component: AuditLogsPageComponent },
      { path: 'platform', component: PlatformPageComponent },
      { path: 'payment-orders', component: PaymentOrdersPageComponent },
      { path: 'quests', component: QuestsPageComponent },
      { path: 'wallet', component: WalletPageComponent }
    ]
  },
  { path: '**', redirectTo: 'dashboard' }
];
