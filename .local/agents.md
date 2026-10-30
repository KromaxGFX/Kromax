# Agents Configuration

## Plan Storage Location
All implementation plans MUST be stored in `.local/.agentes/` directory.

### Naming Convention
- Plans: `plan-<feature-name>-<date>.md`
- Example: `plan-software-rasterizer-2026-10-29.md`

### Required Plan Structure
Each plan must include:
1. **Objective** - Clear goal statement
2. **Domain Analysis** - SME domains needed
3. **Phase Breakdown** - Phases with tasks
4. **Dependencies** - Task dependencies
5. **Acceptance Criteria** - Per task and overall
6. **Risk Assessment** - Technical risks and mitigations

## Workflow Reminder
1. ANALYZE → Identify domains, create initial spec
2. SME_CONSULTATION → Delegate to @sme (one domain per call, max 3 calls)
3. COLLATE → Synthesize SME outputs into unified spec
4. CODE → Delegate to @coder
5. QA_REVIEW → Delegate to @reviewer (specify CHECK dimensions)
6. TRIAGE → Review feedback: APPROVED | REVISION_NEEDED | BLOCKED
7. TEST → If approved, delegate to @test_engineer

## Delegation Rules
- SME: ONE domain per call (serial), max 3 per phase
- Reviewer: Specify CHECK dimensions relevant to the change
- Always wait for response before next delegation

## Compliance Check
- Reviewer delegation is MANDATORY for every coder task
- pre_check_batch is NOT a substitute for reviewer
- Stage A (automated tools) + Stage B/Council (agent review) = BOTH required