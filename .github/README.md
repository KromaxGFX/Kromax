<p align="center">
  <img src="https://raw.githubusercontent.com/KromaxGFX/WebSite/main/public/favicon.svg" alt="KromaxGFX" width="120" height="120">
</p>

<h1 align="center">KromaxGFX</h1>

<p align="center">
  <strong>Una API gráfica multiplataforma para todos los dispositivos y sistemas operativos que puedan mostrar gráficos.</strong>
</p>

<p align="center">
  <a href="https://github.com/KromaxGFX/KromaxCore/actions"><img src="https://img.shields.io/github/actions/workflow/status/KromaxGFX/KromaxCore/ci.yml?branch=main&label=Core%20CI&logo=github" alt="Core CI"></a>
  <a href="https://github.com/KromaxGFX/KromaxVulkan/actions"><img src="https://img.shields.io/github/actions/workflow/status/KromaxGFX/KromaxVulkan/ci.yml?branch=main&label=Vulkan%20Backend&logo=github" alt="Vulkan Backend CI"></a>
  <a href="https://github.com/KromaxGFX/KromaxMetal/actions"><img src="https://img.shields.io/github/actions/workflow/status/KromaxGFX/KromaxMetal/ci.yml?branch=main&label=Metal%20Backend&logo=github" alt="Metal Backend CI"></a>
  <a href="https://github.com/KromaxGFX/KromaxDX12/actions"><img src="https://img.shields.io/github/actions/workflow/status/KromaxGFX/KromaxDX12/ci.yml?branch=main&label=DX12%20Backend&logo=github" alt="DX12 Backend CI"></a>
  <a href="https://github.com/KromaxGFX/KromaxWebGPU/actions"><img src="https://img.shields.io/github/actions/workflow/status/KromaxGFX/KromaxWebGPU/ci.yml?branch=main&label=WebGPU%20Backend&logo=github" alt="WebGPU Backend CI"></a>
  <br>
  <a href="https://opensource.org/licenses/Apache-2.0"><img src="https://img.shields.io/badge/License-Apache%202.0-blue.svg" alt="License: Apache 2.0"></a>
  <a href="https://github.com/KromaxGFX/KromaxCore/releases"><img src="https://img.shields.io/github/v/release/KromaxGFX/KromaxCore?label=Latest%20Release&logo=github" alt="Latest Release"></a>
  <a href="https://kromaxgfx.github.io/WebSite/"><img src="https://img.shields.io/badge/Docs-kromaxgfx.github.io-0b0d12?logo=githubpages" alt="Documentation"></a>
  <a href="https://github.com/KromaxGFX/KromaxCore/blob/main/CMakeLists.txt"><img src="https://img.shields.io/badge/C%2B%2B-20-blue?logo=c%2B%2B" alt="C++20"></a>
  <a href="https://github.com/KromaxGFX/KromaxCore/stargazers"><img src="https://img.shields.io/github/stars/KromaxGFX/KromaxCore?style=social" alt="Stars"></a>
</p>

---

## 🎯 Misión

**KromaxGFX** desarrolla **Kromax**: una **API gráfica multiplataforma** que permite a motores de juegos, renderizadores científicos y aplicaciones de alto rendimiento ejecutar el **mismo código de render** en Vulkan, Metal, DirectX 12, WebGPU y OpenGL — sin `#ifdef`, sin capas de abstracción con fugas y sin ataduras a un proveedor.

> **Un contrato. Múltiples backends. Cero dependencias gráficas en el núcleo.**

---

## 🏗️ Arquitectura

```
┌─────────────────────────────────────────────────────────────────┐
│                     APLICACIÓN / MOTOR                          │
│  (solo conoce <kromax/core.hpp> y shaders .kromax)              │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                        KROMAXCORE                               │
│  • Contrato de render (IRenderBackend, tipos, límites)          │
│  • Loader de backends (descubrimiento dinámico en runtime)      │
│  • Suite de conformidad (interfaz, no implementación)           │
│  • Cero dependencias gráficas (no enlaza Vulkan/Metal/DX12/WGPU)│
└─────────────────────────────────────────────────────────────────┘
                                │
              ┌─────────────────┼─────────────────┐
              ▼                 ▼                 ▼
       ┌────────────┐    ┌────────────┐    ┌────────────┐
       │  VULKAN    │    │   METAL    │    │   DX12     │
       │  Backend   │    │  Backend   │    │  Backend   │
       └────────────┘    └────────────┘    └────────────┘
              ▲                 ▲                 ▲
              └─────────────────┼─────────────────┘
                                ▼
                       ┌────────────────┐
                       │    WEBGPU      │
                       │    Backend     │
                       └────────────────┘
```

### Diferencia clave

| Enfoque tradicional | Kromax |
|---------------------|--------|
| Capa de abstracción con `#ifdef` por API | **Contrato versionado** (SemVer) |
| Wrapper que impone el mínimo común denominador | **Cada backend implementa nativamente** lo que el contrato define |
| "Funciona en mi máquina" | **Tests de conformidad obligatorios** por backend |
| Dependencias gráficas en el núcleo | **Cero dependencias** — backends son plugins dinámicos |

---

## 📦 Repositorios

| Repositorio | Descripción | Estado | CI |
|-------------|-------------|--------|----|
| **[KromaxCore](https://github.com/KromaxGFX/KromaxCore)** | Núcleo: contrato `IRenderBackend`, tipos, loader, conformidad | ![Active](https://img.shields.io/badge/Activo-brightgreen) | [![CI](https://github.com/KromaxGFX/KromaxCore/actions/workflows/ci.yml/badge.svg)](https://github.com/KromaxGFX/KromaxCore/actions) |
| **[KromaxVulkan](https://github.com/KromaxGFX/KromaxVulkan)** | Backend Vulkan 1.3+ (Linux, Windows, Android, macOS vía MoltenVK) | ![Active](https://img.shields.io/badge/Activo-brightgreen) | [![CI](https://github.com/KromaxGFX/KromaxVulkan/actions/workflows/ci.yml/badge.svg)](https://github.com/KromaxGFX/KromaxVulkan/actions) |
| **[KromaxMetal](https://github.com/KromaxGFX/KromaxMetal)** | Backend Metal 3+ (macOS, iOS, iPadOS, tvOS, visionOS) | ![Active](https://img.shields.io/badge/Activo-brightgreen) | [![CI](https://github.com/KromaxGFX/KromaxMetal/actions/workflows/ci.yml/badge.svg)](https://github.com/KromaxGFX/KromaxMetal/actions) |
| **[KromaxDX12](https://github.com/KromaxGFX/KromaxDX12)** | Backend DirectX 12 (Windows 10/11, Xbox Series X\|S) | ![Active](https://img.shields.io/badge/Activo-brightgreen) | [![CI](https://github.com/KromaxGFX/KromaxDX12/actions/workflows/ci.yml/badge.svg)](https://github.com/KromaxGFX/KromaxDX12/actions) |
| **[KromaxWebGPU](https://github.com/KromaxGFX/KromaxWebGPU)** | Backend WebGPU (navegadores WASM, nativo vía wgpu/Dawn) | ![Active](https://img.shields.io/badge/Activo-brightgreen) | [![CI](https://github.com/KromaxGFX/KromaxWebGPU/actions/workflows/ci.yml/badge.svg)](https://github.com/KromaxGFX/KromaxWebGPU/actions) |
| **[KromaxOpenGL](https://github.com/KromaxGFX/KromaxOpenGL)** | Backend OpenGL 4.6 / ES 3.2 (compatibilidad amplia) | ![Maintenance](https://img.shields.io/badge/Mantenimiento-yellow) | [![CI](https://github.com/KromaxGFX/KromaxOpenGL/actions/workflows/ci.yml/badge.svg)](https://github.com/KromaxGFX/KromaxOpenGL/actions) |
| **[KromaxCLI](https://github.com/KromaxGFX/KromaxCLI)** | CLI: `init`, `backend`, `shader compile`, `conformance`, `bindgen` | ![Active](https://img.shields.io/badge/Activo-brightgreen) | [![CI](https://github.com/KromaxGFX/KromaxCLI/actions/workflows/ci.yml/badge.svg)](https://github.com/KromaxGFX/KromaxCLI/actions) |
| **[KromaxShaderC](https://github.com/KromaxGFX/KromaxShaderC)** | Compilador `.kromax` → SPIR-V, DXIL, Metal IR, WGSL | ![Active](https://img.shields.io/badge/Activo-brightgreen) | [![CI](https://github.com/KromaxGFX/KromaxShaderC/actions/workflows/ci.yml/badge.svg)](https://github.com/KromaxGFX/KromaxShaderC/actions) |
| **[KromaxConformance](https://github.com/KromaxGFX/KromaxConformance)** | Suite de tests de conformidad obligatoria por backend | ![Active](https://img.shields.io/badge/Activo-brightgreen) | [![CI](https://github.com/KromaxGFX/KromaxConformance/actions/workflows/ci.yml/badge.svg)](https://github.com/KromaxGFX/KromaxConformance/actions) |
| **[WebSite](https://github.com/KromaxGFX/WebSite)** | Sitio web y documentación de la organización (Astro) | ![Active](https://img.shields.io/badge/Activo-brightgreen) | [![Deploy](https://github.com/KromaxGFX/WebSite/actions/workflows/desplegar.yml/badge.svg)](https://github.com/KromaxGFX/WebSite/actions) |

---

## ⚡ Principios rectores

1. **Portabilidad real** — El mismo binario compila y funciona en Vulkan, Metal, DX12, WebGPU y OpenGL sin ramas por plataforma en el código de usuario.

2. **Sin fugas de abstracción** — La API expone solo lo que **todos** los backends pueden implementar eficientemente. Nada de *"en Vulkan esto va rápido, en Metal hacemos un workaround"*.

3. **Contrato versionado (SemVer)** — Cambios de ruptura = MAJOR bump con guía de migración documentada. MINOR = nuevas capacidades opcionales (feature flags). PATCH = fixes.

4. **Tests de conformidad obligatorios** — Cada backend **debe** pasar el 100% de la suite `KromaxConformance` para ser release. Dashboard público de conformidad por backend/versión/hardware.

5. **Cero dependencias en el núcleo** — `KromaxCore` no enlaza contra Vulkan, Metal, DX12 ni WebGPU. Los backends son **bibliotecas compartidas** (`.so`/`.dylib`/`.dll`/`.wasm`) cargadas dinámicamente.

6. **Diagnósticos accionables** — Errores del backend mapeados a códigos de error del contrato con contexto (archivo, línea, recurso involucrado).

---

## 📚 Documentación y Sitio Web

- **Web**: https://kromaxgfx.github.io/WebSite/
- **Repositorio del sitio**: https://github.com/KromaxGFX/WebSite (Astro 7, estático, sin JS runtime)

### Guías principales
| Guía | Descripción |
|------|-------------|
| [Primeros pasos](https://kromaxgfx.github.io/WebSite/docs/guia/inicio/) | Instalación, primer proyecto, siguiente pasos |
| [Backends soportados](https://kromaxgfx.github.io/WebSite/docs/guia/backends/) | Detalles de cada backend, plataformas, feature matrix |
| [Portabilidad](https://kromaxgfx.github.io/WebSite/docs/guia/portabilidad/) | Escribe código que compile en todas partes sin `#ifdef` |
| [Arquitectura](https://kromaxgfx.github.io/WebSite/docs/arquitectura/) | Contrato, loader, versionado, feature flags, conformidad |
| [Estilo de código](https://kromaxgfx.github.io/WebSite/docs/guia/estilo-de-codigo/) | C++20, CMake, shaders, docs, naming, RAII, expected |
| [Ramas y commits](https://kromaxgfx.github.io/WebSite/docs/guia/ramas-y-commits/) | Convencional Commits, flujo de PR, SemVer, changelog |

---

## ⚖️ Licencia

Todo el código de la organización (núcleo, backends, herramientas, documentación) se publica bajo **licencia Apache 2.0**.

```
Copyright 2024 KromaxGFX Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
```

Permite: uso comercial, modificación, distribución, sublicenciamiento, uso privado.
Requiere: preservar avisos de copyright, licencia y cambios.
No otorga: derechos de patente (salvo los explícitos en Apache 2.0).

---

## 🏛️ Gobernanza

KromaxGFX se rige por los mismos principios que el código que produce:

- **Transparencia** — Decisiones de diseño en issues públicos, RFC para cambios de API.
- **Meritocracia** — Mantenedores de backends tienen voz y voto en la especificación.
- **Estabilidad** — SemVer estricto, conformidad obligatoria, CI multi-plataforma.
- **Comunidad abierta** — Sin ataduras corporativas. Cualquier persona/entidad puede contribuir, usar el código en proyectos propietarios o crear backends privados.

### Roles
| Rol | Responsabilidad |
|-----|-----------------|
| **Core Maintainers** | Aprobar RFC, gestionar releases de `KromaxCore`, velar por SemVer |
| **Backend Maintainers** | Implementar contrato, mantener conformidad, CI de su backend |
| **Contributors** | Issues, PRs, tests, docs, bindings de lenguaje |

---

## 🤝 Cómo contribuir

1. **Lee las guías** — [Estilo de código](https://kromaxgfx.github.io/WebSite/docs/guia/estilo-de-codigo/) y [Ramas y commits](https://kromaxgfx.github.io/WebSite/docs/guia/ramas-y-commits/).
2. **Busca trabajo** — Issues etiquetadas `good first issue`, `help wanted` o `backend:*`.
3. **Fork + Branch** — `feature/descripcion-corta` desde `main`.
4. **Desarrolla** — Código + tests + docs en el mismo commit. `npm run verificar:todo` (o `cmake + ctest + clang-tidy`) en verde.
5. **Pull Request** — Título convencional (`feat:`, `fix:`, `docs:`, `refactor:`). Mínimo 1 approval + CI verde (Linux/Windows/macOS).
6. **Merge** — Squash and merge. La rama se borra automáticamente.

### Convención de commits
```text
<tipo>: <descripción corta>

[cuerpo opcional explicando QUÉ y POR QUÉ]

[footer: Closes #123, Breaking-change: ...]
```
Tipos: `feat`, `fix`, `docs`, `refactor`, `perf`, `build`, `ci`, `test`, `chore`, `revert`.

---

## 📞 Contacto y Comunidad

| Canal | Propósito |
|-------|-----------|
| **GitHub Issues** | Bugs, feature requests, RFC, preguntas técnicas |
| **GitHub Discussions** | Diseño, arquitectura, preguntas abiertas, anuncios |
| **Pull Requests** | Código, tests, docs, bindings |
| **Security** | `security@kromaxgfx.github.io` (o advisory privado en GH) |
| **Governance** | Issues en este repo (`.github`) con label `governance` |

---

## 🔗 Enlaces rápidos

- 🌐 **Sitio web**: https://kromaxgfx.github.io/WebSite/
- 📖 **Documentación**: https://kromaxgfx.github.io/WebSite/docs/
- 📦 **Release más reciente**: https://github.com/KromaxGFX/KromaxCore/releases
- 🐛 **Reportar bug**: https://github.com/KromaxGFX/KromaxCore/issues/new/choose
- 💡 **Pedir feature**: https://github.com/KromaxGFX/KromaxCore/issues/new/choose
- ⭐ **Apoya el proyecto**: Dale star a [KromaxCore](https://github.com/KromaxGFX/KromaxCore)

---

<p align="center">
  <strong>KromaxGFX</strong> — Una API gráfica para todos los dispositivos.<br>
  <sub>Hecho con ❤️ por la comunidad KromaxGFX. Licencia Apache 2.0.</sub>
</p>