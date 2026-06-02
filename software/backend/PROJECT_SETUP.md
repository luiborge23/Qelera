# Qelera Backend — FastAPI Project Structure v1.0
# Python 3.12+, FastAPI, SQLAlchemy, Alembic
# Created: May 28, 2026

# ============================================================
# PROJECT STRUCTURE
# ============================================================
# backend/
# ├── app/
# │   ├── main.py                 # FastAPI app entry point
# │   ├── config.py               # Settings & configuration
# │   ├── database.py             # Database connection & session management
# │   ├── models/                 # SQLAlchemy ORM models
# │   │   ├── base.py
# │   │   ├── user.py
# │   │   ├── workspace.py
# │   │   ├── device.py
# │   │   ├── session.py
# │   │   ├── qoe_metric.py
# │   │   ├── alert.py
# │   │   └── api_key.py
# │   ├── schemas/                # Pydantic schemas (request/response)
# │   │   ├── auth.py
# │   │   ├── user.py
# │   │   ├── device.py
# │   │   ├── session.py
# │   │   ├── qoe.py
# │   │   ├── alert.py
# │   │   └── common.py
# │   ├── api/                    # Route handlers
# │   │   ├── router.py           # Main router
# │   │   ├── auth.py             # Auth endpoints
# │   │   ├── users.py            # User management
# │   │   ├── devices.py          # Device management
# │   │   ├── sessions.py         # Test session management
# │   │   ├── qoe.py              # QoE metrics endpoints
# │   │   ├── alerts.py           # Alert management
# │   │   └── websocket.py        # WebSocket real-time service
# │   ├── services/               # Business logic
# │   │   ├── auth_service.py
# │   │   ├── user_service.py
# │   │   ├── device_service.py
# │   │   ├── session_service.py
# │   │   ├── qoe_service.py
# │   │   ├── alert_service.py
# │   │   └── notification_service.py
# │   ├── core/                   # Core utilities
# │   │   ├── security.py         # JWT, password hashing
# │   │   ├── middleware.py       # Custom middleware
# │   │   ├── dependencies.py     # FastAPI dependencies
# │   │   └── exceptions.py       # Custom exceptions
# │   └── utils/                  # Utility functions
# │       ├── logger.py
# │       ├── validators.py
# │       └── helpers.py
# ├── tests/
# │   ├── conftest.py
# │   ├── test_auth.py
# │   ├── test_devices.py
# │   ├── test_sessions.py
# │   ├── test_qoe.py
# │   └── test_alerts.py
# ├── alembic/
# │   ├── env.py
# │   └── versions/
# ├── Dockerfile
# ├── requirements.txt
# └── pyproject.toml

# ============================================================
# REQUIREMENTS (requirements.txt)
# ============================================================
fastapi==0.115.0
uvicorn[standard]==0.30.0
sqlalchemy==2.0.35
alembic==1.13.0
psycopg2-binary==2.9.9
redis==5.0.7
kafka-python==2.0.2
pydantic==2.9.0
pydantic-settings==2.5.0
python-jose[cryptography]==3.3.0
passlib[bcrypt]==1.7.4
python-multipart==0.0.9
httpx==0.27.0
websockets==13.0
celery==5.4.0
flower==2.0.1
prometheus-client==0.20.0
sentry-sdk[fastapi]==2.13.0
structlog==24.1.0
aioredis==2.0.1
aiohttp==3.10.0
numpy==1.26.4
pandas==2.2.2
scikit-learn==1.5.1
xgboost==2.1.1

# ============================================================
# PYPROJECT.TOML
# ============================================================
# [build-system]
# requires = ["setuptools>=68.0", "wheel"]
# build-backend = "setuptools.backends._legacy:_Backend"
#
# [project]
# name = "qelera-backend"
# version = "1.0.0"
# description = "Qelera Backend API"
# requires-python = ">=3.12"
# dependencies = [
#     "fastapi>=0.115.0",
#     "uvicorn[standard]>=0.30.0",
#     "sqlalchemy>=2.0.35",
#     "alembic>=1.13.0",
#     "psycopg2-binary>=2.9.9",
#     "redis>=5.0.7",
#     "pydantic>=2.9.0",
#     "python-jose[cryptography]>=3.3.0",
#     "passlib[bcrypt]>=1.7.4",
#     "python-multipart>=0.0.9",
#     "httpx>=0.27.0",
#     "websockets>=13.0",
# ]
#
# [project.optional-dependencies]
# dev = [
#     "pytest>=8.0.0",
#     "pytest-asyncio>=0.23.0",
#     "pytest-cov>=5.0.0",
#     "black>=24.0.0",
#     "ruff>=0.5.0",
#     "mypy>=1.10.0",
# ]
#
# [tool.pytest.ini_options]
# testpaths = ["tests"]
# python_files = ["test_*.py"]
# addopts = "-v --cov=app --cov-report=term-missing"
#
# [tool.ruff]
# line-length = 100
# target-version = "py312"
#
# [tool.black]
# line-length = 100
# target-version = ["py312"]

# ============================================================
# DOCKERFILE
# ============================================================
# FROM python:3.12-slim
#
# WORKDIR /app
#
# # Install system dependencies
# RUN apt-get update && apt-get install -y --no-install-recommends \
#     gcc \
#     libpq-dev \
#     && rm -rf /var/lib/apt/lists/*
#
# # Copy requirements and install
# COPY requirements.txt .
# RUN pip install --no-cache-dir -r requirements.txt
#
# # Copy application
# COPY ./app /app/app
# COPY ./alembic /app/alembic
# COPY alembic.ini /app/
#
# # Create non-root user
# RUN useradd -m -u 1000 appuser
# USER appuser
#
# # Expose port
# EXPOSE 8000
#
# # Health check
# HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
#     CMD curl -f http://localhost:8000/healthz || exit 1
#
# # Run
# CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]

# ============================================================
# ALEMBIC CONFIG (alembic.ini)
# ============================================================
# [alembic]
# script_location = alembic
# sqlalchemy.url = postgresql://vt_user:vt_password@db:5432/qelera
#
# [loggers]
# keys = root,sqlalchemy,alembic
#
# [handlers]
# keys = console
#
# [formatters]
# keys = generic
#
# [logger_root]
# level = WARN
# handlers = console
#
# [logger_sqlalchemy]
# level = WARN
# handlers =
# qualname = sqlalchemy.engine
#
# [logger_alembic]
# level = INFO
# handlers =
# qualname = alembic
#
# [handler_console]
# class = StreamHandler
# args = (sys.stderr,)
# level = NOTSET
# formatter = generic
#
# [formatter_generic]
# format = %(levelname)-5.5s [%(name)s] %(message)s
# datefmt = %H:%M:%S

# ============================================================
# ENVIRONMENT VARIABLES (.env.example)
# ============================================================
# # Database
# DATABASE_URL=postgresql://vt_user:vt_password@db:5432/qelera
#
# # Redis
# REDIS_URL=redis://redis:6379/0
#
# # JWT
# JWT_SECRET_KEY=your-super-secret-jwt-key-change-in-production
# JWT_ALGORITHM=HS256
# JWT_ACCESS_TOKEN_EXPIRE_MINUTES=30
# JWT_REFRESH_TOKEN_EXPIRE_DAYS=7
#
# # Security
# BCRYPT_ROUNDS=12
#
# # Kafka
# KAFKA_BOOTSTRAP_SERVERS=kafka:9092
#
# # Sentry
# SENTRY_DSN=
#
# # CORS
# CORS_ORIGINS=https://app.qelera.io,https://admin.qelera.io
#
# # Logging
# LOG_LEVEL=INFO
# LOG_FORMAT=json
