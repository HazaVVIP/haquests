# Developer Documentation Index

Selamat datang di Developer documentation untuk HAQuests project. Dokumentasi ini berisi hasil research, planning, dan technical decisions yang dibuat selama phase 0 (Foundation & Research).

## 📚 Table of Contents

### 1. [Project Overview](00-PROJECT_OVERVIEW.md)
**Ringkasan**: Gambaran umum proyek, tujuan, benefit, dan filosofi desain.

**Isi:**
- Tujuan utama proyek
- Target use cases
- Filosofi desain (modular, performance-aware, safety-first)
- Non-goals dan risks
- Success criteria
- Timeline estimasi

**Baca ini jika**: Anda baru pertama kali melihat project ini dan ingin memahami "why" dan "what".

---

### 2. [Raw Socket Research](01-RAW_SOCKET_RESEARCH.md)
**Ringkasan**: Deep dive ke raw socket programming di Linux, termasuk structures, privileges, dan best practices.

**Isi:**
- Socket types (SOCK_RAW, AF_PACKET)
- CAP_NET_RAW requirements
- IP dan TCP header structures
- Checksum calculation
- Sending dan receiving packets
- Common pitfalls dan solutions
- Testing strategy

**Baca ini jika**: Anda perlu memahami low-level details tentang bagaimana raw socket bekerja.

**Key Takeaways:**
- Use SOCK_RAW dengan IPPROTO_TCP (recommended)
- Checksums harus 100% correct
- libpcap untuk receiving lebih praktis
- Butuh CAP_NET_RAW atau root

---

### 3. [TCP State Machine](02-TCP_STATE_MACHINE.md)
**Ringkasan**: Implementation guide untuk TCP state machine, termasuk states, transitions, dan reliability mechanisms.

**Isi:**
- TCP states dan state diagram
- 3-way handshake implementation
- Sequence number management
- Window management
- Data transfer
- Retransmission timer
- Connection termination
- Congestion control (optional)

**Baca ini jika**: Anda akan implement TCP layer atau perlu memahami TCP protocol details.

**Key Takeaways:**
- 11 TCP states (MVP fokus pada subset)
- Sequence number tracking critical
- Start dengan happy path, iterate
- Retransmission untuk reliability

---

### 4. [TLS/SSL Integration](03-TLS_SSL_INTEGRATION.md)
**Ringkasan**: Strategy untuk mengintegrasikan OpenSSL dengan raw socket menggunakan custom BIO.

**Isi:**
- OpenSSL BIO abstraction
- Custom BIO implementation
- TLS connection setup
- Certificate verification
- ALPN support
- Session resumption
- Error handling
- Buffering strategy

**Baca ini jika**: Anda perlu implement HTTPS support atau troubleshoot TLS issues.

**Key Takeaways:**
- Custom BIO adalah bridge antara OpenSSL dan raw socket
- BIO callbacks: read, write, control
- Test dengan loopback dulu
- Certificate verification critical

---

### 5. [HTTP Parser & Smuggling](04-HTTP_PARSER_SMUGGLING.md)
**Ringkasan**: HTTP request/response parsing dan implementation HTTP Request Smuggling techniques.

**Isi:**
- HTTP Request Smuggling basics (CL.TE, TE.CL, TE.TE)
- HTTP request builder
- HTTP response parser
- Chunked transfer encoding
- WAF bypass techniques
- Malformed requests
- Detection techniques

**Baca ini jika**: Anda perlu implement HTTP layer atau smuggling features.

**Key Takeaways:**
- Smuggling requires precise control over request formatting
- CL.TE, TE.CL, TE.TE adalah main techniques
- Timing-based detection
- **IMPORTANT**: Only untuk authorized testing

---

### 6. [Directory Structure](05-DIRECTORY_STRUCTURE.md)
**Ringkasan**: Proposed project directory structure, build system, dan organization.

**Isi:**
- Complete directory tree
- CMake build system structure
- Docker configuration
- Module organization
- Development workflow
- Installation procedures

**Baca ini jika**: Anda perlu setup project structure atau understand how files organized.

**Key Takeaways:**
- Modular structure: core/, tcp/, tls/, http/
- CMake untuk build system
- Docker untuk development
- Clear separation: include/, src/, tests/, examples/

---

### 7. [Implementation Roadmap](06-IMPLEMENTATION_ROADMAP.md)
**Ringkasan**: Detailed implementation plan dengan phases, tasks, deliverables, dan timeline.

**Isi:**
- Phase 0: Foundation & Setup ✅
- Phase 1: Core Layer - Raw Packets
- Phase 2: TCP Layer - State Machine
- Phase 3: HTTP Layer
- Phase 4: TLS Layer
- Phase 5: HTTP Smuggling
- Phase 6: Advanced Features
- Phase 7: Testing & Hardening
- Milestones dan success criteria
- Risk mitigation

**Baca ini jika**: Anda perlu understand development plan atau track progress.

**Key Takeaways:**
- 7 phases, ~20 weeks total
- Incremental development
- Clear deliverables per phase
- Milestones untuk tracking

---

### 8. [Testing Strategy](07-TESTING_STRATEGY.md)
**Ringkasan**: Comprehensive testing approach termasuk unit, integration, E2E, performance, dan security testing.

**Isi:**
- Testing pyramid (unit → integration → E2E)
- Unit tests untuk each layer
- Integration tests
- System/E2E tests
- Performance benchmarks
- Fuzz testing
- Security testing (static analysis, dynamic analysis)
- Test coverage goals
- CI/CD pipeline

**Baca ini jika**: Anda perlu write tests atau setup testing infrastructure.

**Key Takeaways:**
- GoogleTest framework
- Coverage goal: >80%
- Docker untuk integration tests
- Multiple testing levels
- CI/CD dengan GitHub Actions

---

### 9. [Research Summary](README.md)
**Ringkasan**: Executive summary dari semua research documents, key decisions, dan next steps.

**Isi:**
- Summary dari each document
- Key technical decisions dan rationale
- Critical risks & mitigation
- Resource requirements
- Success criteria
- Recommended next steps
- Open questions

**Baca ini jika**: Anda ingin quick overview tanpa membaca semua documents.

---

## 🗺️ Reading Paths

### Path 1: Quick Start (Untuk Developer Baru)
1. Start: [Research Summary](README.md)
2. Then: [Project Overview](00-PROJECT_OVERVIEW.md)
3. Then: [Implementation Roadmap](06-IMPLEMENTATION_ROADMAP.md)
4. Optional: Deep dive ke specific topics sesuai kebutuhan

### Path 2: Technical Deep Dive (Untuk Implementation)
1. [Raw Socket Research](01-RAW_SOCKET_RESEARCH.md)
2. [TCP State Machine](02-TCP_STATE_MACHINE.md)
3. [TLS/SSL Integration](03-TLS_SSL_INTEGRATION.md)
4. [HTTP Parser & Smuggling](04-HTTP_PARSER_SMUGGLING.md)

### Path 3: Project Planning (Untuk Project Manager)
1. [Project Overview](00-PROJECT_OVERVIEW.md)
2. [Implementation Roadmap](06-IMPLEMENTATION_ROADMAP.md)
3. [Testing Strategy](07-TESTING_STRATEGY.md)
4. [Research Summary](README.md)

### Path 4: Architecture Review (Untuk Architect)
1. [Directory Structure](05-DIRECTORY_STRUCTURE.md)
2. [Project Overview](00-PROJECT_OVERVIEW.md)
3. [Implementation Roadmap](06-IMPLEMENTATION_ROADMAP.md)

## 📊 Document Statistics

| Document | Lines | Focus Area | Complexity |
|----------|-------|------------|------------|
| 00-PROJECT_OVERVIEW.md | ~200 | Planning | Low |
| 01-RAW_SOCKET_RESEARCH.md | ~400 | Technical | High |
| 02-TCP_STATE_MACHINE.md | ~600 | Technical | Very High |
| 03-TLS_SSL_INTEGRATION.md | ~700 | Technical | High |
| 04-HTTP_PARSER_SMUGGLING.md | ~800 | Technical | Medium |
| 05-DIRECTORY_STRUCTURE.md | ~600 | Architecture | Low |
| 06-IMPLEMENTATION_ROADMAP.md | ~700 | Planning | Medium |
| 07-TESTING_STRATEGY.md | ~800 | Testing | Medium |
| README.md | ~500 | Summary | Low |

**Total**: ~5,300 lines of research documentation

## 🎯 Quick Reference

### Key Technical Decisions

1. **Socket Type**: SOCK_RAW dengan IPPROTO_TCP
2. **Receive Method**: libpcap dengan BPF filtering
3. **TLS Library**: OpenSSL dengan custom BIO
4. **Build System**: CMake
5. **Testing**: GoogleTest
6. **Container**: Docker untuk development

### Critical Dependencies

- OpenSSL 1.1.1+
- libpcap
- CMake 3.15+
- C++17 compiler
- Linux kernel dengan CAP_NET_RAW support

### Important Links

- Main README: [../README.md](../README.md)
- GitHub: https://github.com/HazaVVIP/haquests

## 🔄 Document Updates

Jika ada perubahan significant dalam implementation yang berbeda dari research:

1. Update relevant research document
2. Add note di `CHANGELOG.md` (to be created)
3. Update this INDEX if structure changes

## ❓ Questions?

Jika ada pertanyaan tentang research atau technical decisions:

1. Check relevant research document
2. Check [Research Summary](README.md) untuk context
3. Open GitHub issue untuk discussion

---

**Last Updated**: 2026-02-17
**Research Phase Status**: ✅ Complete
**Next Phase**: Implementation (Phase 1 - Core Layer)
