/**
 * @file qmg2sc.hpp
 * @author Benjamin Barrois <benjamin.barrois@hiventive.com>
 * @date Jan, 28th 2019
 * @copyright Copyright (C) 2019, Hiventive.
 *
 * @brief QEMU Machine Generator To SystemC Bridge
 */

namespace hv {

template <unsigned int BUSWIDTH>
QMG2SC<BUSWIDTH>::QMG2SC(::hv::module::ModuleName name_)
    : ::hv::module::Module(name_), MMIOSocket("MMIOSocket"), IRQOutSocket("IRQOutSocket"),
      IRQInSocket("IRQInSocket") {
    SC_HAS_PROCESS(QMG2SC<BUSWIDTH>);
    IRQInSocket.registerBTransport(this, &QMG2SC<BUSWIDTH>::mIRQInBTransport);

    QMGInit((void *)this);

    QMGRegisterMMIOBTransport(QMG2SCMMIOBTransport);
    QMGRegisterMMIOGetDirectMemPtr(QMG2SCMMIOGetDirectMemPtr);
    QMGRegisterIRQBTransport(QMG2SCIRQOutBTransport);

    SC_METHOD(incomingIRQOut);
    sensitive << mIncomingIRQOutEvent;
    dont_initialize();

    SC_METHOD(incomingMMIO);
    sensitive << mIncomingMMIOEvent;
    dont_initialize();
}

template <unsigned int BUSWIDTH> QMG2SC<BUSWIDTH>::~QMG2SC() {
    mQMGStartThread.join();
}

template <unsigned int BUSWIDTH> void QMG2SC<BUSWIDTH>::start_of_simulation() {
    mQMGStartThread = ::std::thread(QMGStart);
}

template <unsigned int BUSWIDTH> void QMG2SC<BUSWIDTH>::handleMMIOBTransport(QMGMMIOPayload *p) {
    mIncomingMMIOMutex[0].lock();
    mIncomingMMIOMutex[1].lock();

    mIncomingQMGMMIOPayload = p;
    mIncomingMMIOEvent.notify();

    mIncomingMMIOMutex[1].lock();
    mIncomingMMIOMutex[1].unlock();
    mIncomingMMIOMutex[0].unlock();
}

template <unsigned int BUSWIDTH> void QMG2SC<BUSWIDTH>::incomingMMIO() {
    if (mIncomingQMGMMIOPayload->size > 8) {
        HV_ERR("Error: QEMU does not support read/writes above 8 bytes");
    }
    mmio_payload_type txn;

    ::sc_core::sc_time zeroTime(::sc_core::SC_ZERO_TIME);
    txn.setAddress(mIncomingQMGMMIOPayload->address);
    txn.setDataLength(mIncomingQMGMMIOPayload->size);

    // Note: no mask is necessary on p->value after read because it is zero-initialized
    txn.setDataPtr(reinterpret_cast<::hv::common::hvuint8_t *>(&mIncomingQMGMMIOPayload->value));

    switch (mIncomingQMGMMIOPayload->cmd) {
    case QMG_COMMAND_READ:
        txn.setCommand(::hv::communication::tlm2::protocols::memorymapped::MemoryMappedCommand::
                           MEM_MAP_READ_COMMAND);
        break;
    case QMG_COMMAND_WRITE:
        txn.setCommand(::hv::communication::tlm2::protocols::memorymapped::MemoryMappedCommand::
                           MEM_MAP_WRITE_COMMAND);
        break;
    default:
        txn.setCommand(::hv::communication::tlm2::protocols::memorymapped::MemoryMappedCommand::
                           MEM_MAP_IGNORE_COMMAND);
    }
    MMIOSocket->b_transport(txn, zeroTime);
    mIncomingQMGMMIOPayload->dmiAllowed = txn.isDMIAllowed();
    mIncomingQMGMMIOPayload->respStatus =
        txn.isResponseOk() ? QMG_MMIO_OK_RESPONSE : QMG_MMIO_ERROR_RESPONSE;

    mIncomingMMIOMutex[1].unlock();
}

template <unsigned int BUSWIDTH>
bool QMG2SC<BUSWIDTH>::handleMMIOGetDirectMemPtr(QMGMMIOPayload *p, QMGDMIData *d) {
    mmio_payload_type txn;
    txn.setAddress(p->address);
    ::tlm::tlm_dmi dmiData;

    bool ret = MMIOSocket->get_direct_mem_ptr(txn, dmiData);
    d->ptr = dmiData.get_dmi_ptr();
    d->startAddress = dmiData.get_start_address();
    d->endAddress = dmiData.get_end_address();
    ::sc_core::sc_time oneNanoSecond = ::sc_core::sc_time(1.0, ::sc_core::SC_NS);
    d->readLatency = (uint64_t)(dmiData.get_read_latency() / oneNanoSecond);
    d->writeLatency = (uint64_t)(dmiData.get_write_latency() / oneNanoSecond);
    d->accessRule = dmiData.is_read_write_allowed()
                        ? QMG_DMI_READ_WRITE
                        : dmiData.is_read_allowed()
                              ? QMG_DMI_READ
                              : dmiData.is_write_allowed() ? QMG_DMI_WRITE : QMG_DMI_NONE;
    return ret;
}

template <unsigned int BUSWIDTH> void QMG2SC<BUSWIDTH>::handleIRQOutBTransport(QMGIRQPayload *p) {
    mIncomingIRQOutMutex[0].lock();
    mIncomingIRQOutMutex[1].lock();
    mIncomingQMGIRQPayload = p;
    mIncomingIRQOutEvent.notify();
    mIncomingIRQOutMutex[1].lock();
    mIncomingIRQOutMutex[1].unlock();
    mIncomingIRQOutMutex[0].unlock();
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::mIRQInBTransport(irq_payload_type &txn, ::sc_core::sc_time &t) {
    QMGSetIRQInLevel(txn.getID(), txn.getValue() ? 1 : 0);
    txn.setResponseStatus(
        ::hv::communication::tlm2::protocols::irq::IRQResponseStatus::IRQ_OK_RESPONSE);
}

template <unsigned int BUSWIDTH> void QMG2SC<BUSWIDTH>::incomingIRQOut() {
    irq_payload_type txn;
    ::sc_core::sc_time zeroTime(::sc_core::SC_ZERO_TIME);

    txn.setID(mIncomingQMGIRQPayload->id);
    txn.setValue(mIncomingQMGIRQPayload->level);

    IRQOutSocket->b_transport(txn, zeroTime);

    mIncomingQMGIRQPayload->respStatus =
        txn.isResponseOk() ? QMG_IRQ_OK_RESPONSE : QMG_IRQ_ERROR_RESPONSE;
    mIncomingIRQOutMutex[1].unlock();
}

} // namespace hv
