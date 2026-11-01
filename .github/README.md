# KromaxGFX

Organización enfocada en desarrollar **Kromax**: una API gráfica multiplataforma
para todos los dispositivos y sistemas operativos que puedan mostrar gráficos a
un usuario.

## Repositorios

| Repositorio | Descripción | Estado |
|-------------|-------------|--------|
| [KromaxCore](https://github.com/KromaxGFX/KromaxCore) | Núcleo: contrato de render, tipos, loader de backends, tests de conformidad | Activo |
| [KromaxVulkan](https://github.com/KromaxGFX/KromaxVulkan) | Backend Vulkan 1.3+ (Linux, Windows, Android, macOS vía MoltenVK) | Activo |
| [KromaxMetal](https://github.com/KromaxGFX/KromaxMetal) | Backend Metal 3+ (macOS, iOS, iPadOS, tvOS, visionOS) | Activo |
| [KromaxDX12](https://github.com/KromaxGFX/KromaxDX12) | Backend DirectX 12 (Windows 10/11, Xbox Series X\|S) | Activo |
| [KromaxWebGPU](https://github.com/KromaxGFX/KromaxWebGPU) | Backend WebGPU (navegadores WASM, nativo vía wgpu/Dawn) | Activo |
| [KromaxOpenGL](https://github.com/KromaxGFX/KromaxOpenGL) | Backend OpenGL 4.6 / ES 3.2 (compatibilidad amplia) | Mantenimiento |
| [KromaxCLI](https://github.com/KromaxGFX/KromaxCLI) | CLI: init, backend mgmt, shader compile, conformance, bindgen | Activo |
| [KromaxShaderC](https://github.com/KromaxGFX/KromaxShaderC) | Compilador de shaders `.kromax` → SPIR-V, DXIL, Metal IR, WGSL | Activo |
| [KromaxConformance](https://github.com/KromaxGFX/KromaxConformance) | Suite de tests de conformidad obligatoria por backend | Activo |
| [WebSite](https://github.com/KromaxGFX/WebSite) | Sitio de la organización y documentación (este repo clonado por separado) | Activo |

## Qué es Kromax

Kromax no es un motor, ni un wrapper, ni una capa de abstracción tradicional.
Es un **contrato de render versionado** que separa *qué* necesita la aplicación
de *cómo* lo implementa cada API gráfica.

```
┌─────────────────────────────────────────────┐
│           APLICACIÓN / MOTOR                │
│  (solo conoce <kromax/core.hpp> y .kromax)  │
└─────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────┐
│                KROMAXCORE                   │
│  • Contrato (IRenderBackend, tipos, límites)│
│  • Loader de backends (descubrimiento din.) │
│  • Tests de conformidad (interfaz, no impl.)│
│  • Cero dependencias gráficas               │
└─────────────────────────────────────────────┘
                    │
        ┌───────────┼───────────┐
        ▼           ▼           ▼
   ┌────────┐ ┌────────┐ ┌────────┐
   │VULKAN  │ │ METAL  │ │ DX12   │
   └────────┘ └────────┘ └────────┘
        ▲           ▲           ▲
        └───────────┼───────────┘
                    ▼
             ┌────────────┐
             │  WEBGPU    │
             └────────────┘
```

### Principios rectores

1. **Portabilidad real**: el mismo binario del motor compila y funciona en
   Vulkan, Metal, DirectX 12, WebGPU y OpenGL sin `#ifdef` en el código de usuario.

2. **Sin fugas de abstracción**: la API expone solo lo que todos los backends
   pueden implementar de forma eficiente. Nada de "funciona en Vulkan pero en
   Metal hacemos un workaround".

3. **Contrato versionado**: SemVer estricto. Cambios de ruptura = MAJOR bump,
   migración documentada.

4. **Tests de conformidad obligatorios**: cada backend pasa la misma suite.
   Si un backend pasa, es conforme. No hay sorpresas en producción.

5. **Cero dependencias en el núcleo**: `KromaxCore` no enlaza contra Vulkan,
   Metal, DX12 ni WebGPU. Los backends son plugins que se cargan dinámicamente.

## Sitio web y documentación

- **Web**: https://kromaxgfx.github.io/WebSite/
- **Repositorio del sitio**: https://github.com/KromaxGFX/WebSite
- **Documentación**: arquitectura, guías de inicio, backends, portabilidad, estilo de código, flujo de trabajo Git.

## Licencia

Todo el código de la organización (núcleo, backends, herramientas, documentación)
se publica bajo **licencia Apache 2.0**. Esto permite uso comercial,
modificación, distribución y sublicenciamiento, con la única exigencia de
preservar los avisos de copyright y licencia.

## Gobernanza

KromaxGFX se rige por los mismos principios que el código que produce:
transparencia, meritocracia y estabilidad. Las decisiones de diseño se
discuten en issues públicos, los cambios de API pasan por RFC, y los
mantenedores de cada backend tienen voz y voto en la dirección de la
especificación.

La organización no está atada a ninguna empresa: es un proyecto comunitario.
Cualquier persona o entidad puede contribuir, usar el código en proyectos
propietarios o crear backends privados.

## Cómo contribuir

1. Lee la [guía de estilo de código](https://kromaxgfx.github.io/WebSite/docs/guia/estilo-de-codigo/).
2. Familiarízate con el flujo de [ramas y commits](https://kromaxgfx.github.io/WebSite/docs/guia/ramas-y-commits/).
3. Busca una issue etiquetada `good first issue` o propone una nueva.
4. Envía un pull request; la CI validará formato, tests y análisis estático.

## Contacto

La conversación técnica ocurre en GitHub: issues, discussions y pull requests.
Para temas de gobernanza o seguridad, usa el canal correspondiente en este
repositorio (`.github`).

---

**KromaxGFX** — Una API gráfica para todos los dispositivos.