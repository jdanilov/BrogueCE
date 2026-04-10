#!/bin/bash
# Touch files after Claude edits them to trigger Vite rebuild
# This script is called by Claude Code's PostToolUse hook

FILE_PATH=$(echo "$HOOK_INPUT" | jq -r '.tool_input.file_path')

if [ -f "$FILE_PATH" ]; then
  touch "$FILE_PATH"
  echo "Touched: $FILE_PATH" >&2
fi
