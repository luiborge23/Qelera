# ============================================================
# Dashboard Relay — Local WebSocket & HTTP API
# ============================================================
# Python 3.12 | WebSocket
# Real-time dashboard relay for local monitoring
# ============================================================

import asyncio
import json
import logging
import websockets
from datetime import datetime
from typing import Dict, List, Optional

logger = logging.getLogger('DashboardRelay')


class DashboardRelay:
    """WebSocket relay for local dashboard monitoring."""
    
    def __init__(self, config: dict):
        """Initialize the dashboard relay."""
        self.config = config
        self.host = config.get('dashboard', {}).get('host', '0.0.0.0')
        self.port = config.get('dashboard', {}).get('port', 8080)
        self.ws_port = config.get('dashboard', {}).get('ws_port', 8081)
        
        self.websocket_server = None
        self.connected_clients = set()
        self.status_cache = {}
        
        logger.info("DashboardRelay initialized")
    
    async def start(self):
        """Start the dashboard relay."""
        logger.info(f"Starting dashboard relay on {self.host}:{self.ws_port}")
        
        # Start WebSocket server
        self.websocket_server = await websockets.serve(
            self._handle_client,
            self.host,
            self.ws_port
        )
        
        logger.info("Dashboard relay started")
    
    async def _handle_client(self, websocket, path):
        """Handle WebSocket client connection."""
        logger.info(f"Client connected: {websocket.remote_address}")
        self.connected_clients.add(websocket)
        
        try:
            # Send initial status
            await self._send_status(websocket)
            
            # Keep connection alive and broadcast updates
            async for message in websocket:
                try:
                    data = json.loads(message)
                    await self._process_command(websocket, data)
                except json.JSONDecodeError:
                    await websocket.send(json.dumps({
                        'type': 'error',
                        'message': 'Invalid JSON'
                    }))
        
        except websockets.exceptions.ConnectionClosed:
            logger.info(f"Client disconnected: {websocket.remote_address}")
        finally:
            self.connected_clients.discard(websocket)
    
    async def _send_status(self, websocket):
        """Send current gateway status to client."""
        status = {
            'type': 'status',
            'timestamp': datetime.utcnow().isoformat(),
            'data': self.status_cache
        }
        
        try:
            await websocket.send(json.dumps(status))
        except Exception as e:
            logger.error(f"Failed to send status: {e}")
    
    async def _process_command(self, websocket, data: dict):
        """Process incoming commands from dashboard."""
        command = data.get('type')
        
        if command == 'request_status':
            await self._send_status(websocket)
        
        elif command == 'request_frames':
            await self._send_recent_frames(websocket)
        
        elif command == 'request_alerts':
            await self._send_recent_alerts(websocket)
        
        else:
            await websocket.send(json.dumps({
                'type': 'error',
                'message': f'Unknown command: {command}'
            }))
    
    async def _send_recent_frames(self, websocket):
        """Send recent frame data to client."""
        # This would query the edge cache
        frames = []
        
        response = {
            'type': 'frames',
            'timestamp': datetime.utcnow().isoformat(),
            'data': frames
        }
        
        await websocket.send(json.dumps(response))
    
    async def _send_recent_alerts(self, websocket):
        """Send recent alerts to client."""
        # This would query the edge cache
        alerts = []
        
        response = {
            'type': 'alerts',
            'timestamp': datetime.utcnow().isoformat(),
            'data': alerts
        }
        
        await websocket.send(json.dumps(response))
    
    async def broadcast_alert(self, alert: dict):
        """Broadcast alert to all connected clients."""
        message = {
            'type': 'alert',
            'timestamp': datetime.utcnow().isoformat(),
            'data': alert
        }
        
        if self.connected_clients:
            await asyncio.gather(
                *[client.send(json.dumps(message)) for client in self.connected_clients],
                return_exceptions=True
            )
    
    async def broadcast_status(self, status: dict):
        """Broadcast status update to all connected clients."""
        message = {
            'type': 'status',
            'timestamp': datetime.utcnow().isoformat(),
            'data': status
        }
        
        if self.connected_clients:
            await asyncio.gather(
                *[client.send(json.dumps(message)) for client in self.connected_clients],
                return_exceptions=True
            )
    
    async def stop(self):
        """Stop the dashboard relay."""
        if self.websocket_server:
            self.websocket_server.close()
            await self.websocket_server.wait_closed()
        
        self.connected_clients.clear()
        logger.info("Dashboard relay stopped")
    
    def get_status(self) -> dict:
        """Get relay status."""
        return {
            'connected_clients': len(self.connected_clients),
            'host': self.host,
            'port': self.ws_port
        }
