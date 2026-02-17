# HAQuests - HTTP Library dengan Raw Socket C++

## Ringkasan Proyek

Library HTTP berbasis raw socket C++ yang dirancang untuk memberikan kontrol penuh atas layer L4 (TCP) dan L7 (HTTP) untuk keperluan penetration testing dan security research.

## Tujuan Utama

### 1. Kontrol Penuh Layer Network
- **L4 (TCP)**: Manipulasi langsung TCP packets, sequence numbers, flags, dan segmentation
- **L7 (HTTP)**: Kemampuan membuat HTTP requests yang cacat/ambigu untuk testing

### 2. Menghindari Auto-Correction
- Bypass normalisasi dari library standar (libcurl, dll)
- Bypass stack TCP/IP OS untuk kontrol penuh
- Kemampuan mengirim paket yang sengaja dibuat cacat

### 3. Security Research Applications
- **HTTP Request Smuggling**: CL.TE, TE.CL, dan variasi lainnya
- **WAF Bypass**: TCP segmentation, manipulasi flags
- **IDS Evasion**: Out-of-order packets, fragmentation tricks

## Benefit Utama

### Untuk Security Testing
1. **Precision Control**: Kontrol byte-level atas setiap paket
2. **Research Capability**: Platform untuk meneliti behavior HTTP parsers
3. **WAF Testing**: Kemampuan menguji detection systems dengan payload yang sophisticated

### Untuk Development
1. **Learning Tool**: Memahami TCP/IP stack secara mendalam
2. **Custom Protocol**: Basis untuk implementasi protocol kustom
3. **Performance**: Potential untuk optimasi khusus use case tertentu

## Target Use Cases

1. **Penetration Testing**: Testing aplikasi web dengan teknik advanced
2. **Security Research**: Menemukan parser differentials dan ambiguities
3. **WAF/IDS Testing**: Validasi detection rules dan bypass techniques
4. **Educational**: Pembelajaran mendalam tentang network programming

## Filosofi Desain

### Keep It Modular
- Setiap komponen (TCP, HTTP, TLS) sebagai modul terpisah
- Interface yang clean antar komponen
- Testable dan maintainable

### Performance Aware
- Zero-copy operations where possible
- Efficient memory management
- Minimal overhead

### Safety First
- Clear documentation tentang privilege requirements
- Error handling yang robust
- Validasi input yang ketat

## Non-Goals (Explicitly Out of Scope)

1. **Production HTTP Client**: Ini bukan pengganti libcurl untuk aplikasi production
2. **RFC Compliance**: Sengaja tidak patuh RFC untuk testing purposes
3. **Cross-Platform**: Focus utama Linux (dengan CAP_NET_RAW)
4. **High-Level API**: Prioritas pada control, bukan convenience

## Risks dan Mitigations

### Technical Risks
1. **Complexity**: TCP state machine sangat kompleks
   - Mitigation: Iterative development, extensive testing
   
2. **TLS Integration**: OpenSSL integration dengan raw socket tricky
   - Mitigation: Research BIO interfaces, prototype early

3. **Reliability**: Raw socket prone to connection issues
   - Mitigation: Robust error handling, retry mechanisms

### Operational Risks
1. **Privilege Requirements**: Requires root/CAP_NET_RAW
   - Mitigation: Clear documentation, container support
   
2. **Legal/Ethical**: Tool bisa disalahgunakan
   - Mitigation: Clear disclaimer, responsible disclosure guidelines

## Success Criteria

### Phase 1: Foundation (MVP)
- [ ] Raw socket creation dan basic packet sending
- [ ] Simple TCP 3-way handshake
- [ ] Basic HTTP GET request (non-TLS)

### Phase 2: Core Features
- [ ] Full TCP state machine
- [ ] HTTP parser dengan custom manipulation
- [ ] TLS/SSL support

### Phase 3: Advanced Features
- [ ] HTTP/2 support
- [ ] Advanced evasion techniques
- [ ] Performance optimizations

## Timeline Estimasi

- **Research & Design**: 1-2 minggu
- **Phase 1 MVP**: 2-3 minggu
- **Phase 2 Core**: 4-6 minggu
- **Phase 3 Advanced**: 4-8 minggu

**Total**: 3-5 bulan untuk full-featured library

## Stakeholders

- **Primary**: Security researchers, penetration testers
- **Secondary**: Network programmers, students
- **Tertiary**: Tool developers yang butuh custom HTTP behavior
