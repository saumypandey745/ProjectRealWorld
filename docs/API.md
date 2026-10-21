# Backend & REST API Specification (`/v1`)

Backend service: Node.js (TypeScript) + Fastify/Express + PostgreSQL + Redis.

## Endpoints

### 1. Authentication & Account
- `POST /v1/auth/register` — Register player profile.
- `POST /v1/auth/login` — Login, returns short-lived JWT + rotating refresh token.
- `POST /v1/auth/refresh` — Rotate refresh token, issue new JWT.
- `POST /v1/auth/logout` — Invalidate current refresh token.
- `DELETE /v1/account` — GDPR / DPDP right to be forgotten account wipe.
- `GET /v1/account/export` — Data export archive.

### 2. Cloud Saves
- `GET /v1/saves` — List player's save slots and versions.
- `GET /v1/saves/:slot` — Fetch encrypted/compressed save blob.
- `PUT /v1/saves/:slot` — Save upload with `If-Match` versioning header for conflict detection.

### 3. Remote Config & Live-Ops
- `GET /v1/config` — Remote feature flags, kill switches, current event configs.
- `POST /v1/events` — Batched anonymous game telemetry.
- `GET /v1/leaderboards/:id` — Global/friend leaderboard scores.
- `POST /v1/leaderboards/:id/scores` — Submit score (server-side sanity check).
