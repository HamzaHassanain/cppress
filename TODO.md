## **Must Be Implemented by Framework Developer**

_These require changes to core framework code, protocol handling, or low-level architecture_

### Critical Protocol/Core Features

#### **Tier 1: Blocking Issues (Framework is incomplete without these)**

1.  **HTTP Keep-Alive support** - Requires connection reuse in socket layer
2.  **Chunked transfer encoding** - Core HTTP protocol parsing/generation
3.  **Multipart/form-data parsing** - Complex body parsing in HTTP layer
4.  **Request body streaming** - Architecture change (can't load all in memory)
5.  **Cookie parsing** - HTTP header parsing (you only have setting)
6.  **Range requests (206 Partial Content)** - HTTP protocol feature

#### **Tier 2: Performance/Scalability (Framework is slow without these)**

11. **Static file caching** - Need framework-level cache
12. **Compiled route tree** - Router internal optimization
13. **Zero-copy optimizations** - Buffer management in socket/HTTP layers
14. **Memory pooling** - Object reuse for requests/responses
15. **Request queue limits** - Thread pool modification
16. **Dynamic thread pool sizing** - Thread pool enhancement
17. **Header parsing optimization** - HTTP parser caching/optimization

#### **Tier 3: API Usability (Framework is clunky without these)**

19. **Response helper methods** (`res->ok()`, `res->not_found()`) - Response class extension
20. **JSON response shortcut** (`res->json(object)` instead of `send_json(stringify())`) - Response class
21. **File download helper** (`res->download(path)`) - Response class + streaming
22. **Route grouping** (`router->group("/api", ...)`) - Router architecture
23. **Nested routers** - Router mounting logic
24. **Middleware error handling** - Exception propagation in middleware chain
25. **Route parameter constraints** (regex validation) - Route matching logic
26. **Route inspection/listing** - Router metadata storage

#### **Tier 4: Security Hardening (Framework is unsafe without these)**

27. **Request size limits per route** - Router/route-level configuration
28. **Header injection protection** - HTTP parser validation
29. **Path traversal protection** - Static file serving hardening (basic exists, needs improvement)
30. **Secure cookie flags by default** - Response API defaults
31. **HSTS/security header helpers** - Response class utilities

#### **Tier 5: Developer Experience (Framework is hard to use without these)**

33. **Automatic type conversion** (query params to int/bool) - Request class helpers
34. **Error context in exceptions** - Exception class enhancement
35. **Request/response type validation** - Better template error messages (C++20 concepts)
36. **Middleware execution trace** - Router internal tracking
37. **Performance timing APIs** - Framework instrumentation hooks

#### **Tier 6: Build/Deploy (Framework is hard to integrate without these)**

38. **pkg-config support** - CMake build system
39. **Pre-built binaries** - Build/release process
40. **Docker examples** - Deployment documentation
41. **Release optimization flags** - CMake configuration
42. **CI/CD examples** - Build automation

#### **Tier 7: Advanced Features (Nice to have)**

43. **HTTP/2 support** - Complete protocol rewrite
44. **Compression (gzip/brotli)** - Stream encoding/decoding
45. **HTTP pipelining** - Connection management
46. **Transfer-Encoding beyond chunked** - HTTP protocol
47. **Plugin system** - Framework architecture for extensibility
48. **Dependency injection** - Framework-level service container

---

## **Visual Summary**

```

┌─────────────────────────────────────────────────────────────┐
│                Core Framework                               │
├─────────────────────────────────────────────────────────────┤
│  TIER 1 (Blocking): Keep-Alive, Chunked, Multipart,         │
│         Streaming, Cookie Parsing, Range Requests           │
│                                                             │
│  TIER 2 (Performance): Static Cache, Route Tree,            │
│         Zero-Copy, Memory Pooling, Async I/O                │
│                                                             │
│  TIER 3 (API): Helper Methods, Route Grouping,              │
│         Nested Routers, Better Middleware                   │
│                                                             │
│  TIER 4 (Security): Size Limits, Injection Protection,      │
│         Secure Defaults, Security Headers                   │
│                                                             │
│  TIER 5 (DX): Type Conversion, Better Errors, Tracing       │
│                                                             │
│  TIER 6 (Deploy): Build System, Packaging, CI/CD            │
│                                                             │
│  TIER 7 (Advanced): HTTP/2, Compression, Plugins            │
└─────────────────────────────────────────────────────────────┘
```
