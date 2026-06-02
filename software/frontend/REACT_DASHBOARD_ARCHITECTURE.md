# Qelera React Dashboard — Monitoring & Analytics v1.0
# React 18+, TypeScript, Vite, Recharts, WebSocket, TailwindCSS
# Created: May 28, 2026

# ============================================================
# PROJECT STRUCTURE
# ============================================================
# react-dashboard/
# ├── index.html
# ├── package.json
# ├── tsconfig.json
# ├── vite.config.ts
# ├── tailwind.config.js
# ├── postcss.config.js
# ├── public/
# │   └── favicon.svg
# ├── src/
# │   ├── main.tsx                    # Entry point
# │   ├── App.tsx                     # Root component
# │   ├── index.css                   # Tailwind imports
# │   ├── types/
# │   │   ├── index.ts                # Shared TypeScript types
# │   │   ├── qoe.ts                  # QoE metric types
# │   │   ├── alerts.ts               # Alert types
# │   │   ├── devices.ts              # Device types
# │   │   └── sessions.ts             # Session types
# │   ├── services/
# │   │   ├── api.ts                  # REST API client (Axios)
# │   │   ├── websocket.ts            # WebSocket connection manager
# │   │   ├── mqtt.ts                 # MQTT over WebSocket
# │   │   └── vault.ts                # Vault token management
# │   ├── hooks/
# │   │   ├── useQoEMetrics.ts        # Real-time QoE data hook
# │   │   ├── useAlerts.ts            # Alert management hook
# │   │   ├── useDevices.ts           # Device list/management hook
# │   │   ├── useSessions.ts          # Session lifecycle hook
# │   │   ├── useWebSocket.ts         # WebSocket connection hook
# │   │   └── useTheme.ts             # Dark/light theme hook
# │   ├── components/
# │   │   ├── layout/
# │   │   │   ├── Sidebar.tsx         # Navigation sidebar
# │   │   │   ├── Header.tsx          # Top header bar
# │   │   │   ├── DashboardLayout.tsx # Main layout wrapper
# │   │   │   └── NotificationCenter.tsx  # Alert bell + dropdown
# │   │   ├── charts/
# │   │   │   ├── QoETrendChart.tsx   # Time-series QoE chart
# │   │   │   ├── DevicePerformance.tsx  # Device comparison chart
# │   │   │   ├── AlertDistribution.tsx  # Pie/donut chart
# │   │   │   ├── SessionHeatmap.tsx   # Heatmap of sessions
# │   │   │   └── MetricGauge.tsx     # Gauge component
# │   │   ├── tables/
# │   │   │   ├── DeviceTable.tsx     # Device listing table
# │   │   │   ├── AlertTable.tsx      # Alert history table
# │   │   │   ├── SessionTable.tsx    # Session records table
# │   │   │   └── MetricTable.tsx     # Detailed metrics table
# │   │   ├── cards/
# │   │   │   ├── QoEScoreCard.tsx    # Overall QoE score display
# │   │   │   ├── DeviceStatusCard.tsx  # Device status overview
# │   │   │   ├── AlertSummaryCard.tsx  # Alert count summary
# │   │   │   └── SessionStatsCard.tsx  # Session statistics
# │   │   ├── views/
# │   │   │   ├── DashboardView.tsx   # Main dashboard overview
# │   │   │   ├── DeviceView.tsx      # Device management
# │   │   │   ├── SessionView.tsx     # Session monitoring
# │   │   │   ├── AlertView.tsx       # Alert management
# │   │   │   ├── AnalyticsView.tsx   # Analytics & reports
# │   │   │   └── SettingsView.tsx    # Configuration
# │   │   └── ui/
# │   │       ├── Button.tsx          # Reusable button
# │   │       ├── Modal.tsx           # Modal dialog
# │   │       ├── Badge.tsx           # Status badge
# │   │       ├── Spinner.tsx         # Loading spinner
# │   │       ├── Toast.tsx           # Toast notifications
# │   │       └── DataTable.tsx       # Generic table component
# │   ├── store/
# │   │   ├── index.ts                # Redux/Zustand setup
# │   │   ├── qoeSlice.ts             # QoE state slice
# │   │   ├── alertSlice.ts           # Alert state slice
# │   │   ├── deviceSlice.ts          # Device state slice
# │   │   └── uiSlice.ts              # UI state (modals, etc.)
# │   ├── utils/
# │   │   ├── formatters.ts           # Number/date formatters
# │   │   ├── validators.ts           # Input validators
# │   │   ├── constants.ts            # App constants
# │   │   └── helpers.ts              # Utility functions
# │   └── routes/
# │       └── app.tsx                 # React Router configuration
# ├── tests/
# │   ├── __mocks__/
# │   ├── components/
# │   ├── hooks/
# │   ├── services/
# │   └── utils/
# └── README.md

# ============================================================
# PACKAGE.JSON (dependencies)
# ============================================================
# {
#   "name": "qelera-dashboard",
#   "version": "1.0.0",
#   "private": true,
#   "type": "module",
#   "scripts": {
#     "dev": "vite",
#     "build": "tsc && vite build",
#     "preview": "vite preview",
#     "test": "vitest",
#     "test:coverage": "vitest --coverage",
#     "lint": "eslint src --ext .ts,.tsx",
#     "lint:fix": "eslint src --ext .ts,.tsx --fix",
#     "format": "prettier --write 'src/**/*.{ts,tsx}'",
#     "typecheck": "tsc --noEmit"
#   },
#   "dependencies": {
#     "react": "^18.3.1",
#     "react-dom": "^18.3.1",
#     "react-router-dom": "^6.23.1",
#     "recharts": "^2.12.7",
#     "axios": "^1.7.2",
#     "@tensorflow/tfjs": "^4.19.0",
#     "mqtt": "^5.5.0",
#     "socket.io-client": "^4.7.5",
#     "zustand": "^4.5.2",
#     "date-fns": "^3.6.0",
#     "clsx": "^2.1.1",
#     "tailwind-merge": "^2.3.0",
#     "lucide-react": "^0.395.0"
#   },
#   "devDependencies": {
#     "@types/react": "^18.3.3",
#     "@types/react-dom": "^18.3.0",
#     "@vitejs/plugin-react": "^4.3.1",
#     "autoprefixer": "^10.4.19",
#     "postcss": "^8.4.38",
#     "tailwindcss": "^3.4.3",
#     "typescript": "^5.4.5",
#     "vite": "^5.2.11",
#     "vitest": "^1.6.0",
#     "@testing-library/react": "^16.0.0",
#     "@testing-library/jest-dom": "^6.4.6",
#     "@testing-library/user-event": "^14.5.2",
#     "eslint": "^8.57.0",
#     "prettier": "^3.2.5"
#   }
# }

# ============================================================
# MAIN APP (src/App.tsx)
# ============================================================
# import React, { Suspense, lazy } from 'react';
# import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom';
# import { DashboardLayout } from './components/layout/DashboardLayout';
# import { Spinner } from './components/ui/Spinner';
#
# // Lazy-loaded views for code splitting
# const DashboardView = lazy(() => import('./components/views/DashboardView'));
# const DeviceView = lazy(() => import('./components/views/DeviceView'));
# const SessionView = lazy(() => import('./components/views/SessionView'));
# const AlertView = lazy(() => import('./components/views/AlertView'));
# const AnalyticsView = lazy(() => import('./components/views/AnalyticsView'));
# const SettingsView = lazy(() => import('./components/views/SettingsView'));
#
# function App() {
#   return (
#     <BrowserRouter>
#       <Routes>
#         <Route path="/" element={<DashboardLayout />}>
#           <Route index element={<Navigate to="/dashboard" replace />} />
#           <Route
#             path="dashboard"
#             element={
#               <Suspense fallback={<Spinner />}>
#                 <DashboardView />
#               </Suspense>
#             }
#           />
#           <Route
#             path="devices"
#             element={
#               <Suspense fallback={<Spinner />}>
#                 <DeviceView />
#               </Suspense>
#             }
#           />
#           <Route
#             path="sessions"
#             element={
#               <Suspense fallback={<Spinner />}>
#                 <SessionView />
#               </Suspense>
#             }
#           />
#           <Route
#             path="alerts"
#             element={
#               <Suspense fallback={<Spinner />}>
#                 <AlertView />
#               </Suspense>
#             }
#           />
#           <Route
#             path="analytics"
#             element={
#               <Suspense fallback={<Spinner />}>
#                 <AnalyticsView />
#               </Suspense>
#             }
#           />
#           <Route
#             path="settings"
#             element={
#               <Suspense fallback={<Spinner />}>
#                 <SettingsView />
#               </Suspense>
#             }
#           />
#         </Route>
#       </Routes>
#     </BrowserRouter>
#   );
# }
#
# export default App;

# ============================================================
# MAIN DASHBOARD VIEW (src/components/views/DashboardView.tsx)
# ============================================================
# import React, { useEffect } from 'react';
# import { useQoEMetrics } from '../../hooks/useQoEMetrics';
# import { useAlerts } from '../../hooks/useAlerts';
# import { useDevices } from '../../hooks/useDevices';
# import { QoEScoreCard } from '../cards/QoEScoreCard';
# import { DeviceStatusCard } from '../cards/DeviceStatusCard';
# import { AlertSummaryCard } from '../cards/AlertSummaryCard';
# import { SessionStatsCard } from '../cards/SessionStatsCard';
# import { QoETrendChart } from '../charts/QoETrendChart';
# import { DevicePerformance } from '../charts/DevicePerformance';
# import { AlertDistribution } from '../charts/AlertDistribution';
# import { AlertTable } from '../tables/AlertTable';
#
# const DashboardView: React.FC = () => {
#   const { metrics, loading: metricsLoading, error: metricsError } = useQoEMetrics();
#   const { alerts, loading: alertsLoading, error: alertsError } = useAlerts();
#   const { devices, loading: devicesLoading, error: devicesError } = useDevices();
#
#   if (metricsLoading || alertsLoading || devicesLoading) {
#     return <div className="flex items-center justify-center h-full">Loading...</div>;
#   }
#
#   if (metricsError || alertsError || devicesError) {
#     return <div className="p-4 bg-red-100 text-red-700 rounded">Error loading data</div>;
#   }
#
#   return (
#     <div className="space-y-6">
#       {/* KPI Cards Row */}
#       <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
#         <QoEScoreCard score={metrics.overallQoE} trend={metrics.qoeTrend} />
#         <DeviceStatusCard total={devices.length} online={devices.filter(d => d.online).length} />
#         <AlertSummaryCard total={alerts.length} critical={alerts.filter(a => a.severity === 'critical').length} />
#         <SessionStatsCard active={metrics.activeSessions} completed={metrics.totalSessions} />
#       </div>
#
#       {/* Charts Row */}
#       <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
#         <QoETrendChart data={metrics.qoeHistory} />
#         <DevicePerformance data={devices.map(d => ({ name: d.name, score: d.avgQoE }))} />
#       </div>
#
#       {/* Alert Distribution */}
#       <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
#         <div className="lg:col-span-1">
#           <AlertDistribution data={alerts} />
#         </div>
#         <div className="lg:col-span-2">
#           <AlertTable alerts={alerts.slice(0, 10)} />
#         </div>
#       </div>
#     </div>
#   );
# };
#
# export default DashboardView;

# ============================================================
# QOE TREND CHART (src/components/charts/QoETrendChart.tsx)
# ============================================================
# import React from 'react';
# import {
#   LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip,
#   Legend, ResponsiveContainer, Area, AreaChart
# } from 'recharts';
#
# interface QoETrendChartProps {
#   data: Array<{
#     timestamp: string;
#     qoe: number;
#     minQoe: number;
#     maxQoe: number;
#   }>;
# }
#
# const QoETrendChart: React.FC<QoETrendChartProps> = ({ data }) => {
#   return (
#     <div className="bg-white dark:bg-gray-800 p-4 rounded-lg shadow">
#       <h3 className="text-lg font-semibold mb-4 text-gray-900 dark:text-white">
#         QoE Trend (24h)
#       </h3>
#       <ResponsiveContainer width="100%" height={300}>
#         <AreaChart data={data}>
#           <defs>
#             <linearGradient id="colorQoe" x1="0" y1="0" x2="0" y2="1">
#               <stop offset="5%" stopColor="#10b981" stopOpacity={0.3}/>
#               <stop offset="95%" stopColor="#10b981" stopOpacity={0}/>
#             </linearGradient>
#           </defs>
#           <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
#           <XAxis
#             dataKey="timestamp"
#             stroke="#6b7280"
#             tick={{ fontSize: 12 }}
#           />
#           <YAxis
#             domain={[0, 100]}
#             stroke="#6b7280"
#             tick={{ fontSize: 12 }}
#             label={{ value: 'QoE Score', angle: -90, position: 'insideLeft' }}
#           />
#           <Tooltip
#             contentStyle={{
#               backgroundColor: '#1f2937',
#               border: 'none',
#               borderRadius: '8px',
#               color: '#fff'
#             }}
#           />
#           <Legend />
#           <Area
#             type="monotone"
#             dataKey="qoe"
#             stroke="#10b981"
#             fillOpacity={1}
#             fill="url(#colorQoe)"
#             name="Average QoE"
#           />
#           <Line
#             type="monotone"
#             dataKey="minQoe"
#             stroke="#ef4444"
#             strokeDasharray="5 5"
#             name="Min QoE"
#             dot={false}
#           />
#           <Line
#             type="monotone"
#             dataKey="maxQoe"
#             stroke="#3b82f6"
#             strokeDasharray="5 5"
#             name="Max QoE"
#             dot={false}
#           />
#         </AreaChart>
#       </ResponsiveContainer>
#     </div>
#   );
# };
#
# export default QoETrendChart;

# ============================================================
# WEBSOCKET SERVICE (src/services/websocket.ts)
# ============================================================
# import { QoEMetric, AlertEvent, DeviceStatus } from '../types';
#
# export class WebSocketService {
#   private ws: WebSocket | null = null;
#   private url: string;
#   private reconnectInterval: number = 5000;
#   private maxReconnectAttempts: number = 10;
#   private reconnectAttempts: number = 0;
#   private listeners: Map<string, Set<Function>> = new Map();
#
#   constructor(url: string) {
#     this.url = url;
#   }
#
#   connect() {
#     if (this.ws?.readyState === WebSocket.OPEN) return;
#
#     this.ws = new WebSocket(this.url);
#
#     this.ws.onopen = () => {
#       console.log('WebSocket connected');
#       this.reconnectAttempts = 0;
#       this.emit('connected', {});
#     };
#
#     this.ws.onmessage = (event) => {
#       try {
#         const data = JSON.parse(event.data);
#         this.handleMessage(data);
#       } catch (error) {
#         console.error('Failed to parse WebSocket message:', error);
#       }
#     };
#
#     this.ws.onclose = () => {
#       console.log('WebSocket disconnected');
#       this.emit('disconnected', {});
#       this.reconnect();
#     };
#
#     this.ws.onerror = (error) => {
#       console.error('WebSocket error:', error);
#     };
#   }
#
#   private handleMessage(data: any) {
#     const { type, payload } = data;
#     this.emit(type, payload);
#   }
#
#   private reconnect() {
#     if (this.reconnectAttempts >= this.maxReconnectAttempts) {
#       console.error('Max reconnection attempts reached');
#       return;
#     }
#
#     this.reconnectAttempts++;
#     console.log(`Reconnecting in ${this.reconnectInterval}ms (attempt ${this.reconnectAttempts})`);
#
#     setTimeout(() => {
#       this.connect();
#     }, this.reconnectInterval);
#   }
#
#   on(event: string, callback: Function) {
#     if (!this.listeners.has(event)) {
#       this.listeners.set(event, new Set());
#     }
#     this.listeners.get(event)!.add(callback);
#   }
#
#   off(event: string, callback: Function) {
#     this.listeners.get(event)?.delete(callback);
#   }
#
#   private emit(event: string, data: any) {
#     this.listeners.get(event)?.forEach(callback => callback(data));
#   }
#
#   send(type: string, data: any) {
#     if (this.ws?.readyState === WebSocket.OPEN) {
#       this.ws.send(JSON.stringify({ type, data }));
#     }
#   }
#
#   disconnect() {
#     this.ws?.close();
#     this.ws = null;
#   }
# }

# ============================================================
# TYPES (src/types/qoe.ts)
# ============================================================
# export interface QoEMetric {
#   timestamp: string;
#   device_id: string;
#   session_id: string;
#   qoe_score: number;
#   psnr: number;
#   ssim: number;
#   vmaf: number;
#   bitrate: number;
#   resolution: string;
#   frame_rate: number;
#   latency_ms: number;
#   jitter_ms: number;
#   packet_loss_pct: number;
#   buffer_ratio: number;
#   anomaly_detected: boolean;
#   anomaly_type?: string;
# }
#
# export interface QoESummary {
#   overallQoE: number;
#   qoeTrend: number;
#   activeSessions: number;
#   totalSessions: number;
#   qoeHistory: Array<{
#     timestamp: string;
#     qoe: number;
#     minQoe: number;
#     maxQoe: number;
#   }>;
# }
