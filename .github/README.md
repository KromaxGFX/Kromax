# KromaxGFX — Organización

Repositorio de configuración a nivel de organización para **KromaxGFX**.

Este repositorio (`.github`) contiene archivos de salud comunitaria, plantillas por defecto y configuraciones compartidas que aplican a todos los repositorios de la organización salvo que se sobrescriban en cada repo.

---

## 📁 Estructura

```
.github/
├── CONTRIBUTING.md          # Guía de contribución a nivel organización
├── CODE_OF_CONDUCT.md       # Código de conducta (Contributor Covenant 2.1)
├── SECURITY.md              # Política de seguridad y reporte de vulnerabilidades
├── SUPPORT.md               # Canales de soporte y ayuda
├── GOVERNANCE.md            # Modelo de gobernanza y toma de decisiones
├── LICENSE                  # Licencia Apache 2.0 (referencia)
├── FUNDING.yml              # Configuración de GitHub Sponsors / funding
├── issue_template/
│   ├── config.yml           # Configuración del selector de issues
│   ├── bug_report.yml       # Plantilla: reporte de bug
│   ├── feature_request.yml  # Plantilla: solicitud de feature
│   ├── rfc.yml              # Plantilla: Request for Comments (cambios de API)
│   └── backend_implementation.yml  # Plantilla: implementar nuevo backend
├── pull_request_template/
│   ├── default.md           # PR template por defecto
│   ├── backend.md           # PR template para backends
│   └── docs.md              # PR template para documentación
├── workflows/
│   ├── ci-default.yml       # CI por defecto (heredable via `workflow_call`)
│   ├── conformance.yml      # Workflow compartido: tests de conformidad
│   ├── release.yml          # Workflow compartido: release y changelog
│   └── dependabot.yml       # Configuración global de Dependabot
├── settings/
│   ├── branch-protection.json   # Reglas de protección de rama estándar
│   ├── repository-defaults.json # Configuración por defecto de repos
│   └── labels.yml               # Etiquetas estándar de la organización
└── README.md                # Este archivo
```

---

## 🎯 Propósito

Centralizar la configuración compartida para que todos los repos de la organización (`KromaxCore`, `KromaxVulkan`, `KromaxMetal`, `KromaxDX12`, `KromaxWebGPU`, `KromaxOpenGL`, `KromaxCLI`, `KromaxShaderC`, `KromaxConformance`, `WebSite`) mantengan consistencia en:

- **Estándares de contribución** (Commits convencionales, PRs, reviews)
- **Calidad de código** (CI base, clang-tidy, clang-format, tests)
- **Gestión de issues/PRs** (Plantillas, etiquetas, flujos)
- **Seguridad** (Política de divulgación responsable, advisories)
- **Gobernanza** (Roles, RFCs, proceso de releases)
- **Licencia** (Apache 2.0 en todo el código)

---

## 🔧 Uso

### Heredar workflows compartidos

En cada repo, usa `workflow_call` para reutilizar la CI base:

```yaml
# .github/workflows/ci.yml
name: CI
on: [push, pull_request]
jobs:
  base-ci:
    uses: KromaxGFX/.github/.github/workflows/ci-default.yml@main
  # Jobs específicos del repo aquí...
```

### Sobrescribir plantillas

Crea archivos en `.github/` del repo para sobrescribir los de la organización:

```
repo/
├── .github/
│   ├── CONTRIBUTING.md          # Sobrescribe el de la org
│   ├── issue_template/
│   │   └── bug_report.yml       # Sobrescribe plantilla de bug
│   └── workflows/
│       └── ci.yml               # Workflow completo propio (ignora el heredado)
```

### Etiquetas estándar

Las labels definidas en `settings/labels.yml` se sincronizan vía GitHub Actions (`actions/labeler` o `github-actions/label-sync`).

---

## 📋 Archivos clave

| Archivo | Descripción |
|---------|-------------|
| `CONTRIBUTING.md` | Flujo: `feature/*` → PR → Squash merge → `main`; Conventional Commits; CI obligatorio |
| `CODE_OF_CONDUCT.md` | Contributor Covenant 2.1 — Aplicado en todos los repos |
| `SECURITY.md` | Reporte privado → `security@kromaxgfx.github.io` o GitHub Security Advisories |
| `GOVERNANCE.md` | Roles: Core Maintainers, Backend Maintainers, Contributors; RFC process; SemVer |
| `FUNDING.yml` | GitHub Sponsors, OpenCollective, donaciones puntuales |
| `settings/labels.yml` | `type:bug`, `type:feature`, `type:docs`, `type:refactor`, `backend:vulkan`, `backend:metal`, `backend:dx12`, `backend:webgpu`, `backend:opengl`, `priority:high`, `priority:low`, `good first issue`, `help wanted`, `rfc`, `breaking-change` |

---

## 📄 Licencia

Todo el contenido de este repositorio está bajo **licencia Apache 2.0**, salvo que se indique lo contrario.

```
Copyright 2024 KromaxGFX Contributors
Licensed under the Apache License, Version 2.0
```

---

## 📞 Contacto

- **Organización**: https://github.com/KromaxGFX
- **Web**: https://kromaxgfx.github.io/WebSite/
- **Seguridad**: `security@kromaxgfx.github.io` (o GitHub Security Advisories)
- **Gobernanza**: Issues en este repo con label `governance`
