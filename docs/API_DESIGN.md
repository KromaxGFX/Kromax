# Kromax API Design Documentation

## Naming Conventions

- **Functions**: `kxCamelCase` (e.g., `kxCreateInstance`, `kxQueueSubmit`)
- **Types**: `KxCamelCase` (e.g., `KxInstance`, `KxDevice`, `KxResult`)
- **Constants/Enums**: `KX_SNAKE_CASE` (e.g., `KX_SUCCESS`, `KX_ERROR_OUT_OF_MEMORY`)
- **Macros**: `KX_*` (e.g., `KX_STRUCTURE_TYPE_APPLICATION_INFO`)

## sType / pNext Pattern

Every input struct starts with:
```c
KxStructureType sType;
const void* pNext;
```

This allows extension without breaking the ABI. The `sType` must be the first member.

## Enumeration Pattern (Two-Call)

All `kxEnumerate*` functions follow a two-call pattern:

1. First call: pass `NULL` for the output array, get the count
   ```c
   KxResult result = kxEnumeratePhysicalDevices(instance, &count, NULL);
   // result = KX_ERROR_INCOMPLETE if count is returned
   ```
2. Second call: allocate array and call again
   ```c
   KxPhysicalDevice* devices = malloc(count * sizeof(KxPhysicalDevice));
   result = kxEnumeratePhysicalDevices(instance, &count, devices);
   ```

## Versioning

- `KX_MAKE_VERSION(major, minor, patch)`: Packs semver into uint32_t
- `KX_API_VERSION_0_1`: Current version (0.1.0)
- Strict semver: breaking changes increment major, new features increment minor, bug fixes increment patch

## Extension Policy

- Extensions suffixed `EXT` (types, functions, flags)
- Query with `kxEnumerateInstanceExtensionProperties` / `kxEnumerateDeviceExtensionProperties`
- Core is minimal; display, debug, and validation are extensions/layers
- Policy of backward compatibility: core types/sizes fixed with `static_assert`

## Adding an Extension

1. Define new types/structs with `KX_STRUCTURE_TYPE_*` base type
2. Add extension suffix `EXT` to type names
3. Implement functions with `kxGetInstanceProcAddr` / `kxGetDeviceProcAddr`
4. Document in `docs/BACKENDS.md` and `docs/API_DESIGN.md`
5. Update `docs/ROADMAP.md` if phase change required
EOF