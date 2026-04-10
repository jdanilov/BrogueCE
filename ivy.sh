#!/usr/bin/env bash
# Ivy — Developer ↔ Critic orchestration loop
# Usage: ./ivy.sh [plan-name]

exec bun "$(dirname "$0")/.claude/ivy/cli.ts" "$@"
