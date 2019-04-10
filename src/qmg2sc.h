/**
 * @file qmg2sc.h
 * @author Benjamin Barrois <benjamin.barrois@hiventive.com>
 * @date Jan, 28th 2019
 * @copyright Copyright (C) 2019, Hiventive.
 *
 * @brief QEMU Machine Generator To SystemC Bridge
 */

#ifndef QMG2SC_H
#define QMG2SC_H

#include <thread>
#include <mutex>
#include <HVModule>
#include <HVCommunication>
extern "C" {
#include <qmg/qmg.h>
}

namespace hv {

class QMG2SCHVSCThreadSafeEvent : public sc_core::sc_prim_channel {
  public:
    QMG2SCHVSCThreadSafeEvent() : mEvent(), mEventDelay(::sc_core::SC_ZERO_TIME) {
    }
    explicit QMG2SCHVSCThreadSafeEvent(const char *name_)
        : mEvent(name_), mEventDelay(::sc_core::SC_ZERO_TIME) {
    }

    void notify(::sc_core::sc_time delay = ::sc_core::SC_ZERO_TIME) {
        mEventDelay = delay;
        async_request_update();
    }

    /**
     * @brief Implicit conversion to sc_event
     *
     * Very useful for SC_METHOD sensitivity list
     *
     * @return const ::sc_core::sc_event &
     */
    operator const ::sc_core::sc_event &() const {
        return mEvent;
    }

  protected:
    virtual void update(void) {
        mEvent.notify(mEventDelay);
    }

    ::sc_core::sc_event mEvent;
    ::sc_core::sc_time mEventDelay;
};

void QMG2SCMMIOBTransport(void *handler, QMGMMIOPayload *p);
bool QMG2SCMMIOGetDirectMemPtr(void *handler, QMGMMIOPayload *p, QMGDMIData *d);
void QMG2SCIRQOutBTransport(void *handler, QMGIRQPayload *p);

enum QMG2SCBlockInterfaceType {
    QMG2SC_IF_DEFAULT = -1,
    QMG2SC_IF_NONE = 0,
    QMG2SC_IF_IDE = 1,
    QMG2SC_IF_SCSI = 2,
    QMG2SC_IF_FLOPPY = 3,
    QMG2SC_IF_PFLASH = 4,
    QMG2SC_IF_MTD = 5,
    QMG2SC_IF_SD = 6,
    QMG2SC_IF_VIRTIO = 7,
    QMG2SC_IF_XEN = 8,
    QMG2SC_IF_COUNT = 9
};

class QMG2SCIf {
  public:
    virtual void handleMMIOBTransport(QMGMMIOPayload *p) = 0;
    virtual bool handleMMIOGetDirectMemPtr(QMGMMIOPayload *p, QMGDMIData *d) = 0;
    virtual void handleIRQOutBTransport(QMGIRQPayload *p) = 0;
};

template <unsigned int BUSWIDTH = 32> class QMG2SC : public QMG2SCIf, public ::hv::module::Module {
  public:
    typedef ::hv::communication::tlm2::protocols::memorymapped::MemoryMappedPayload<
        ::hv::common::hvaddr_t>
        mmio_payload_type;
    typedef ::hv::communication::tlm2::protocols::irq::IRQProtocolTypes irq_protocol_types;
    typedef typename irq_protocol_types::tlm_payload_type irq_payload_type;

    QMG2SC(::hv::module::ModuleName name_ = ::sc_core::sc_gen_unique_name("QMG2SCModule"));
    ~QMG2SC();

    void start_of_simulation() override;

    ::hv::communication::tlm2::protocols::memorymapped::MemoryMappedSimpleInitiatorSocket<BUSWIDTH>
        MMIOSocket;
    ::hv::communication::tlm2::protocols::irq::IRQSimpleInitiatorSocket<
        irq_protocol_types, 1, ::sc_core::SC_ZERO_OR_MORE_BOUND>
        IRQOutSocket;
    ::hv::communication::tlm2::protocols::irq::IRQSimpleTargetSocket<
        irq_protocol_types, 0, ::sc_core::SC_ZERO_OR_MORE_BOUND>
        IRQInSocket;

  protected:
    void handleMMIOBTransport(QMGMMIOPayload *p) override;
    bool handleMMIOGetDirectMemPtr(QMGMMIOPayload *p, QMGDMIData *d) override;
    void handleIRQOutBTransport(QMGIRQPayload *p) override;
    void incomingMMIO();
    void incomingIRQOut();

    void mIRQInBTransport(irq_payload_type &txn, ::sc_core::sc_time &t);

    ::std::thread mQMGStartThread;

    QMGMMIOPayload *mIncomingQMGMMIOPayload;
    ::std::mutex mIncomingMMIOMutex[2];
    QMG2SCHVSCThreadSafeEvent mIncomingMMIOEvent;

    QMGIRQPayload *mIncomingQMGIRQPayload;
    ::std::mutex mIncomingIRQOutMutex[2];
    QMG2SCHVSCThreadSafeEvent mIncomingIRQOutEvent;
};

} // namespace hv

#include "qmg2sc.hpp"

#endif
