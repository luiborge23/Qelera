# Qelera — HashiCorp Vault Configuration v1.0
# Vault 1.15+ with AppRole, JWT, and Transit engines
# Created: May 28, 2026

# ============================================================
# DOCKER COMPOSE — VAULT SERVICE
# ============================================================
# Place in docker-compose.yml:
#
#   vault:
#     image: hashicorp/vault:1.15.4
#     container_name: qelera-vault
#     cap_add:
#       - IPC_LOCK
#     environment:
#       VAULT_ADDR: http://127.0.0.1:8200
#       VAULT_DEV_ROOT_TOKEN_ID: ${VAULT_DEV_ROOT_TOKEN}
#     ports:
#       - "8200:8200"
#     volumes:
#       - vault_data:/vault/data
#       - ./vault/config:/vault/config
#       - ./vault/scripts:/vault/scripts
#     networks:
#       - elk
#       - qelera
#     restart: unless-stopped
#     healthcheck:
#       test: ["CMD", "vault", "status"]
#       interval: 10s
#       timeout: 5s
#       retries: 5

# ============================================================
# VAULT CONFIGURATION (HCL)
# ============================================================
# Place in ./vault/config/vault.hcl:
#
# storage "file" {
#   path = "/vault/data"
# }
#
# listener "tcp" {
#   address     = "0.0.0.0:8200"
#   tls_disable = 1  # Set to 0 in production with TLS
# }
#
# api_addr     = "http://vault:8200"
# cluster_addr = "https://vault:8201"
#
# ui = true
#
# # Performance tuning
# performance {
#   standby_perf_seal_bound = true
#   standby_perf_seal_bound_threshold = 10
# }

# ============================================================
# INITIALIZATION SCRIPT
# ============================================================
# Place in ./vault/scripts/init-vault.sh:
#
#!/bin/bash
# set -euo pipefail
#
# echo "=== Initializing Vault ==="
#
# # Check if already initialized
# if vault status -format=json | jq -r '.initialized' | grep -q true; then
#   echo "Vault is already initialized. Skipping init."
# else
#   # Initialize with 5 key shares, 3 key threshold
#   vault operator init -key-shares=5 -key-threshold=3 \
#     -format=json > /vault/data/vault-keys.json
#   echo "Vault initialized. Keys saved to /vault/data/vault-keys.json"
#   echo "IMPORTANT: Store these keys securely!"
#   cat /vault/data/vault-keys.json
# fi
#
# # Unseal (3 of 5 keys)
# echo "=== Unsealing Vault ==="
# UNSEAL_KEY_1=$(jq -r '.unseal_keys_b64[0]' /vault/data/vault-keys.json)
# UNSEAL_KEY_2=$(jq -r '.unseal_keys_b64[1]' /vault/data/vault-keys.json)
# UNSEAL_KEY_3=$(jq -r '.unseal_keys_b64[2]' /vault/data/vault-keys.json)
#
# vault operator unseal $UNSEAL_KEY_1
# vault operator unseal $UNSEAL_KEY_2
# vault operator unseal $UNSEAL_KEY_3
#
# echo "=== Enabling Secrets Engines ==="
#
# # KV v2 for application secrets
# vault secrets enable -path=secret kv-v2
# vault secrets tune -max-version=2 secret
#
# # Database secrets engine (PostgreSQL)
# vault secrets enable -path=db secret
#
# # Transit engine for encryption-as-a-service
# vault transit enable
#
# # AppRole for machine-to-machine auth
# vault auth enable approle
#
# # JWT auth for frontend/mobile clients
# vault write auth/jwt/config \
#   bound_issuer="https://auth.qelera.io" \
#   jwt_validation_pubkeys=@jwks.pub \
#   default_role="app"
#
# # Kubernetes auth (for K8s deployments)
# vault auth enable kubernetes
# vault write auth/kubernetes/config \
#   token_reviewer_jwt="${K8S_JWT}" \
#   kubernetes_host="${K8S_HOST}" \
#   kubernetes_ca_cert=@ca.crt
#
# echo "=== Creating Policies ==="
#
# # Backend service policy
# vault policy write backend <<EOF
# path "secret/data/backend/*" {
#   capabilities = ["read", "list"]
# }
# path "db/creds/qelera-*" {
#   capabilities = ["read"]
# }
# path "transit/encrypt/backend-*" {
#   capabilities = ["update"]
# }
# path "transit/decrypt/backend-*" {
#   capabilities = ["update"]
# }
# path "auth/approle/*" {
#   capabilities = ["read"]
# }
# EOF
#
# # Admin policy
# vault policy write admin <<EOF
# path "secret/*" {
#   capabilities = ["create", "read", "update", "delete", "list", "sudo"]
# }
# path "db/*" {
#   capabilities = ["create", "read", "update", "delete", "list", "sudo"]
# }
# path "transit/*" {
#   capabilities = ["create", "read", "update", "delete", "list", "sudo"]
# }
# path "auth/*" {
#   capabilities = ["create", "read", "update", "delete", "list", "sudo"]
# }
# path "sys/mounts" {
#   capabilities = ["read", "list"]
# }
# path "sys/mounts/*" {
#   capabilities = ["create", "read", "update", "delete", "list", "sudo"]
# }
# EOF
#
# # Developer policy (limited access)
# vault policy write developer <<EOF
# path "secret/data/developer/*" {
#   capabilities = ["read", "list"]
# }
# path "db/creds/qelera-dev-*" {
#   capabilities = ["read"]
# }
# EOF
#
# echo "=== Creating AppRoles ==="
#
# # Backend AppRole
# vault write auth/approle/role/backend \
#   secret_id_ttl="24h" \
#   token_ttl="1h" \
#   token_max_ttl="4h" \
#   token_policies="backend" \
#   token_num_uses="100" \
#   secret_id_num_uses="10" \
#   token_bound_cidrs="10.0.0.0/8,172.16.0.0/12" \
#   policies="backend"
#
# # Developer AppRole
# vault write auth/approle/role/developer \
#   secret_id_ttl="24h" \
#   token_ttl="30m" \
#   token_max_ttl="2h" \
#   token_policies="developer" \
#   token_num_uses="50" \
#   secret_id_num_uses="5" \
#   policies="developer"
#
# # Admin AppRole
# vault write auth/approle/role/admin \
#   secret_id_ttl="24h" \
#   token_ttl="1h" \
#   token_max_ttl="4h" \
#   token_policies="admin" \
#   token_num_uses="100" \
#   secret_id_num_uses="20" \
#   token_bound_cidrs="10.0.0.0/8,172.16.0.0/12" \
#   policies="admin"
#
# echo "=== Setting Secrets ==="
#
# # Database credentials (dynamic)
# vault write db/config/qelera-prod \
#   plugin_name=postgresql-database-plugin \
#   allowed_roles="qelera-prod-*" \
#   connection_url="postgresql://{{username}}:{{password}}@db:5432/qelera" \
#   username="vault_dba" \
#   password="${DB_ADMIN_PASSWORD}" \
#   max_open_connections=10 \
#   max_idle_connections=5
#
# vault write db/roles/qelera-prod-admin \
#   db_name=qelera-prod \
#   creation_statements="CREATE ROLE \"{{name}}\" WITH LOGIN PASSWORD '{{password}}' VALID UNTIL '{{expiration}}'; GRANT admin TO \"{{name}}\";" \
#   revocation_statements="REASSIGN OWNED BY \"{{name}}\" TO postgres; DROP ROLE IF EXISTS \"{{name}}\"" \
#   default_ttl="10m" \
#   max_ttl="30m"
#
# vault write db/roles/qelera-prod-reader \
#   db_name=qelera-prod \
#   creation_statements="CREATE ROLE \"{{name}}\" WITH LOGIN PASSWORD '{{password}}' VALID UNTIL '{{expiration}}'; GRANT CONNECT ON DATABASE qelera TO \"{{name}}\"; GRANT USAGE ON SCHEMA public TO \"{{name}}\"; GRANT SELECT ON ALL TABLES IN SCHEMA public TO \"{{name}}\";" \
#   revocation_statements="REASSIGN OWNED BY \"{{name}}\" TO postgres; DROP ROLE IF EXISTS \"{{name}}\"" \
#   default_ttl="10m" \
#   max_ttl="30m"
#
# vault write db/roles/qelera-prod-writer \
#   db_name=qelera-prod \
#   creation_statements="CREATE ROLE \"{{name}}\" WITH LOGIN PASSWORD '{{password}}' VALID UNTIL '{{expiration}}'; GRANT CONNECT ON DATABASE qelera TO \"{{name}}\"; GRANT USAGE ON SCHEMA public TO \"{{name}}\"; GRANT SELECT, INSERT, UPDATE ON ALL TABLES IN SCHEMA public TO \"{{name}}\";" \
#   revocation_statements="REASSIGN OWNED BY \"{{name}}\" TO postgres; DROP ROLE IF EXISTS \"{{name}}\"" \
#   default_ttl="10m" \
#   max_ttl="30m"
#
# # Static secrets
# vault write secret/data/backend/jwt secret="${JWT_SECRET}" algorithm="HS256"
# vault write secret/data/backend/redis password="${REDIS_PASSWORD}"
# vault write secret/data/backend/sentry dsn="${SENTRY_DSN}"
# vault write secret/data/backend/cors origins="https://app.qelera.io,https://admin.qelera.io"
# vault write secret/data/developer/jwt secret="${JWT_DEV_SECRET}" algorithm="HS256"
#
# # Transit keys for data encryption
# vault write transit/keys/qelera-aes256 \
#   type=aes256-gcm96 \
#   convergent_encryption=true \
#   derived=true \
#   key_size=32
#
# vault write transit/keys/session-encryption \
#   type=aes256-gcm96 \
#   convergent_encryption=true \
#   derived=true \
#   key_size=32
#
# echo "=== Vault Configuration Complete ==="
# echo "Vault is ready!"

# ============================================================
# VAULT SECRETS STRUCTURE
# ============================================================
#
# secret/
# ├── backend/
# │   ├── jwt              # JWT signing keys
# │   ├── redis            # Redis password
# │   ├── sentry           # Sentry DSN
# │   ├── cors             # CORS configuration
# │   └── oauth            # OAuth client secrets
# ├── developer/
# │   ├── jwt              # Dev JWT keys
# │   └── api_keys         # Dev API keys
# ├── production/
# │   ├── tls              # TLS certificates
# │   ├── smtp             # Email service credentials
# │   └── webhook          # Webhook secrets
# └── infrastructure/
#     ├── db_admin         # DB admin credentials
#     ├── kafka            # Kafka SASL credentials
#     └── elastic          # Elasticsearch credentials
#
# db/
# ├── config/
# │   ├── qelera-prod  # PostgreSQL connection config
# │   └── qelera-dev   # Dev PostgreSQL config
# └── roles/
#     ├── qelera-prod-admin  # Admin role
#     ├── qelera-prod-writer # Writer role
#     ├── qelera-prod-reader # Reader role
#     └── qelera-dev-*       # Dev roles

# ============================================================
# VAULT TRANSIT ENGINE USAGE
# ============================================================
#
# Encryption API:
# POST /v1/transit/encrypt/qelera-aes256
# {
#   "plaintext": "base64-encoded-plaintext"
# }
#
# Decryption API:
# POST /v1/transit/decrypt/qelera-aes256
# {
#   "ciphertext": "vault-transit-ciphertext"
# }
#
# Hashing API:
# POST /v1/transit/hash/qelera-sha256
# {
#   "input": "base64-encoded-input",
#   "algorithm": "sha256-256"
# }
#
# Random Generation:
# POST /v1/transit/random/32
# Returns 32 bytes of cryptographically secure random data

# ============================================================
# SECURITY NOTES
# ============================================================
#
# 1. NEVER commit vault-keys.json to version control
# 2. Use separate Vault instances for dev/staging/prod
# 3. Enable audit logging: vault audit enable file file_path=/vault/logs/audit.log
# 4. Rotate secrets regularly (every 90 days minimum)
# 5. Use TLS in production (set tls_disable = 0)
# 6. Restrict AppRole token_bound_cidrs to your infrastructure IPs
# 7. Enable versioning on KV secrets: vault secrets tune -max-version=10 secret
# 8. Use dynamic secrets for databases (auto-rotation)
# 9. Monitor Vault with Prometheus: vault write sys/tools/debug/pprof
# 10. Backup Vault data regularly and test restores
