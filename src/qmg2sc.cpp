/**
 * @file qmg2sc.cpp
 * @author Benjamin Barrois <benjamin.barrois@hiventive.com>
 * @date Jan, 28th 2019
 * @copyright Copyright (C) 2019, Hiventive.
 *
 * @brief QEMU Machine Generator To SystemC Bridge
 */

#include "qmg2sc.h"

namespace hv {

void QMG2SCMMIOBTransport(void *handler, QMGMMIOPayload *p) {
    ((QMG2SCIf *)handler)->handleMMIOBTransport(p);
}

bool QMG2SCMMIOGetDirectMemPtr(void *handler, QMGMMIOPayload *p, QMGDMIData *d) {
    return ((QMG2SCIf *)handler)->handleMMIOGetDirectMemPtr(p, d);
}

void QMG2SCIRQOutBTransport(void *handler, QMGIRQPayload *p) {
    ((QMG2SCIf *)handler)->handleIRQOutBTransport(p);
}

} // namespace hv
