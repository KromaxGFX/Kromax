# Kromax ADR: Opaque Handle Model

## Status: Accepted

## Context
Design decision for the object model in Kromax, following Vulkan conventions for handle safety and extensibility.

## Decision
- Use `KX_DEFINE_HANDLE(type)` macro that expands to `typedef struct type##_T* type`
- All public objects (Instance, PhysicalDevice, Device, Queue, Memory, Buffer, Image, etc.) are opaque handles
- Users should not attempt to interpret or cast the handle value
- Handle validity is managed internally; users pass handles to Kromax functions
- All `kxDestroy*` functions take handles and free the associated resources

## Consequences
- Type safety: handles have distinct types (KxInstance != KxDevice)
- Implementation freedom: internal object layout can change without affecting users
- Safety: users cannot accidentally free or misuse handles
- ABI stability: handle size and type are part of the public API contract
- Extra indirection: handle → internal object adds one level of pointer dereferencing, negligible performance impact