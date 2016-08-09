extern "C" {
    struct KEndPointArgsIterator * KNSManagerMakeKEndPointArgsIterator
        ( const KNSManager * self, const String * hostname, uint32_t port );
    bool KEndPointArgsIterator_Next ( KEndPointArgsIterator * self,
        const String ** hostname, uint16_t * port,
        bool * proxy_default_port, bool * proxy_ep );
}
