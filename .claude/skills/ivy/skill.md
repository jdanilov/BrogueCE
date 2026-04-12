---
name: ivy
description: ⭕ Launch Ivy orchestrator to execute a plan with Developer and Critic agents
---

# Ivy

Ivy is a Developer ↔ Critic orchestration loop that executes plans from `docs/plans/`.
It spawns its own `claude` subprocesses, so it must run in the user's terminal — not inside Claude Code.

**Tell the user to run this in their terminal:**

```bash
bun ivy [plan-name] [--auto]
```

Flags:
- `--auto` / `--auto-approve` — Run all steps (Developer, Critic, Fixer, Committer) without pausing. By default, Ivy pauses before the Commit step to let the user review changes.

If no plan name is given, show available plans:
```bash
ls docs/plans/*.md 2>/dev/null
```

If no plans exist, suggest creating one with `/brainstorm`.
