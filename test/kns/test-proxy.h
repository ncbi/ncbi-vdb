extern "C" {
    struct KEndPointArgsIterator * KNSManagerMakeKEndPointArgsIterator (
        const KNSManager * self, const String * hostname, uint32_t port,
        size_t * cnt );
    bool KEndPointArgsIterator_Next ( KEndPointArgsIterator * self,
        const String ** hostname, uint16_t * port, bool * proxy_default_port,
        bool * proxy_ep, size_t * crnt_proxy_idx, bool * last_proxy );
}
