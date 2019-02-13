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

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::addCPU(const ::std::string &cpuName, const bool &startPoweredOff,
                              const ::hv::common::hvint32_t &mpAffinity,
                              const ::hv::common::hvuint32_t &resetCBAR) {
    QMGCPUInfo tmp;
    tmp.cpuName = (char *)cpuName.c_str();
#ifdef QMG_ARM_ARCH
    tmp.mpaffinity = mpAffinity;
    tmp.startPoweredOff = startPoweredOff;
    tmp.resetCBAR = resetCBAR;
#endif
    cpus.push_back(tmp);
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::addCPUs(const ::std::string &cpuName, const ::hv::common::hvuint32_t &nCPUs,
                               const bool &startPoweredOff,
                               const ::hv::common::hvint32_t &mpAffinity,
                               const ::hv::common::hvuint32_t &resetCBAR) {
    for (::hv::common::hvuint32_t i = 0; i < nCPUs; ++i) {
        this->addCPU(cpuName, startPoweredOff, mpAffinity, resetCBAR);
    }
}

template <unsigned int BUSWIDTH> void QMG2SC<BUSWIDTH>::setRAMSize(const ::std::size_t &ramSize) {
    mQMGMachineInfo.ramSize = ramSize;
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setVCRAMSize(const ::std::size_t &vcramSize) {
    mQMGMachineInfo.vcramSize = vcramSize;
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setIgnoreMemoryTransactionFailures(const bool &val) {
    mQMGMachineInfo.ignoreMemoryTransactionFailures = val;
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setBlockInterfaceType(const QMG2SCBlockInterfaceType &t) {
    mQMGMachineInfo.blockInterfaceType = (::hv::common::hvuint32_t)t;
}

template <unsigned int BUSWIDTH> void QMG2SC<BUSWIDTH>::setNoParallel(const bool &val) {
    mQMGMachineInfo.noParallel = val;
}

template <unsigned int BUSWIDTH> void QMG2SC<BUSWIDTH>::setNoFloppy(const bool &val) {
    mQMGMachineInfo.noFloppy = val;
}

template <unsigned int BUSWIDTH> void QMG2SC<BUSWIDTH>::setNoCDROM(const bool &val) {
    mQMGMachineInfo.noCDROM = val;
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::addBlob(::hv::common::hvuint8_t *blob, const ::std::size_t size,
                               const ::std::string &name, const ::hv::common::hvaddr_t &addr) {
    QMGBlobInfo tmp;
    tmp.blob = blob;
    tmp.blobSize = size;
    tmp.blobName = (char *)name.c_str();
    tmp.blobAddr = addr;
    blobs.push_back(tmp);
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setBoardId(const ::hv::common::hvint32_t &boardId) {
#ifdef QMG_ARM_ARCH
    mQMGBootInfo.boardId = boardId;
#endif
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setCPUBootId(const ::hv::common::hvint32_t &cpuId) {
#ifdef QMG_ARM_ARCH
    mQMGBootInfo.bootCPUId = cpuId;
#endif
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setSMPBootAddr(const ::hv::common::hvaddr_t &SMPBootAddr) {
#ifdef QMG_ARM_ARCH
    mQMGBootInfo.SMPBootAddr = SMPBootAddr;
#endif
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setSecondaryResetSetPCToSMPBootAddr(const bool &activate) {
#ifdef QMG_ARM_ARCH
    mQMGBootInfo.resetSecondarySetPCToSMPBootAddr = activate;
#endif
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setKernelPath(const ::std::string &kernelPath) {
#ifdef QMG_ARM_ARCH
    mQMGBootInfo.kernelPath = (char *)kernelPath.c_str();
#endif
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setKernelCommand(const ::std::string &kernelCommand) {
#ifdef QMG_ARM_ARCH
    mQMGBootInfo.kernelCommand = (char *)kernelCommand.c_str();
#endif
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setInitRDPath(const ::std::string &initRDPath) {
#ifdef QMG_ARM_ARCH
    mQMGBootInfo.initrdPath = (char *)initRDPath.c_str();
#endif
}

template <unsigned int BUSWIDTH> void QMG2SC<BUSWIDTH>::setDTBPath(const ::std::string &dtbPath) {
#ifdef QMG_ARM_ARCH
    mQMGBootInfo.dtbPath = (char *)dtbPath.c_str();
#endif
}
template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setGDBServerActivation(const bool &activate) {
    mQMGDebugInfo.gdbEnableServer = activate;
}

template <unsigned int BUSWIDTH>
void QMG2SC<BUSWIDTH>::setGDBServerParameters(const ::hv::common::hvuint16_t &port,
                                              const bool &waitForConnection) {
    if (!mQMGDebugInfo.gdbEnableServer) {
        HV_WARN("GDB server is not activated. Activate it before setting its parameters to remove "
                "this warning")
    }
    mQMGDebugInfo.gdbPort = port;
    mQMGDebugInfo.gdbWaitForConnection = waitForConnection;
}

template <unsigned int BUSWIDTH> void QMG2SC<BUSWIDTH>::before_end_of_elaboration() {
    mQMGCPUInfo.cpuCount = cpus.size();
    mQMGCPUInfo.cpuInfoArray = cpus.data();
    mQMGCPUInfo.MMIOBTransportCb = QMG2SCMMIOBTransport;
    mQMGCPUInfo.MMIOGetDirectMemPtrCb = QMG2SCMMIOGetDirectMemPtr;
    mQMGCPUInfo.IRQBtransportCb = QMG2SCIRQOutBTransport;
    QMGSetCPUsInfo(&mQMGCPUInfo);

    QMGSetMachineInfo(&mQMGMachineInfo);

    mQMGBootInfo.nBlobs = blobs.size();
    mQMGBootInfo.blobs = blobs.data();
    QMGSetBootInfo(&mQMGBootInfo);

    QMGSetDebugInfo(&mQMGDebugInfo);
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