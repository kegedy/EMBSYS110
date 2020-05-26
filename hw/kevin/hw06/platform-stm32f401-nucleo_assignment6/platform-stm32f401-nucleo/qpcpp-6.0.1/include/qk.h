/// @file
/// @brief QK/C++ platform-independent public interface.
/// @ingroup qk
/// @cond
///***************************************************************************
/// Last updated for version 5.9.7
/// Last updated on  2017-08-18
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, LLC. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// https://state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qk_h
#define qk_h

#include "qequeue.h" // QK kernel uses the native QF event queue
#include "qmpool.h"  // QK kernel uses the native QF memory pool
#include "qpset.h"   // QK kernel uses the native QF priority set


//****************************************************************************
// QF configuration for QK

//! This macro defines the type of the event queue used for active objects.
/// @description
///QK uses the native QF event queue QEQueue.
#define QF_EQUEUE_TYPE         QEQueue

//****************************************************************************
//! attributes of the QK kernel (in C for easy access in assembly)
extern "C" {

struct QK_Attr {
    uint_fast8_t actPrio;    //!< prio of the active AO
    uint_fast8_t nextPrio;   //!< prio of the next AO to execute
    uint_fast8_t lockPrio;   //!< lock prio (0 == no-lock)
    uint_fast8_t lockHolder; //!< prio of the lock holder
#ifndef QK_ISR_CONTEXT_
    uint_fast8_t volatile intNest;    //!< ISR nesting level
#endif // QK_ISR_CONTEXT_
    QP::QPSet readySet;          //!< QK ready-set of AOs and "naked" threads
};

//! global attributes of the QK kernel
extern QK_Attr QK_attr_;

//! QK scheduler finds the highest-priority thread ready to run
uint_fast8_t QK_sched_(void);

//! QK activator activates the next active object. The activated AO preempts
// the currently executing AOs.
void QK_activate_(void);

} // extern "C"


//****************************************************************************
namespace QP {

//! The scheduler lock status
typedef uint_fast16_t QSchedStatus;

//****************************************************************************
//! QK services.
/// @description
/// This class groups together QK services. It has only static members and
/// should not be instantiated.
///
/// @note
/// The QK scheduler, QK priority, QK ready set, etc. belong conceptually
/// to the QK class (as static class members). However, to avoid potential
/// C++ name-mangling problems in assembly language, these elements are
/// defined outside of the QK class and use the extern "C" linkage.
class QK {
public:
    // QK scheduler locking...
    //! QK selective scheduler lock
    static QSchedStatus schedLock(uint_fast8_t const ceiling);

    //! QK selective scheduler unlock
    static void schedUnlock(QSchedStatus const stat);

    //! QK idle callback (customized in BSPs for QK)
    /// @description
    /// QP::QK::onIdle() is called continously by the QK idle loop. This
    /// callback gives the application an opportunity to enter a power-saving
    /// CPU mode, or perform some other idle processing.
    ///
    /// @note QP::QK::onIdle() is invoked with interrupts enabled and must
    /// also return with interrupts enabled.
    ///
    /// @sa QP::QF::onIdle()
    static void onIdle(void);

    //! get the current QK version number string of the form X.Y.Z
    static char_t const *getVersion(void) {
        return versionStr;
    }
};

} // namespace QP


//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    #ifndef QK_ISR_CONTEXT_
        //! Internal port-specific macro that reports the execution context
        // (ISR vs. thread).
        /// @returns true if the code executes in the ISR context and false
        /// otherwise
        #define QK_ISR_CONTEXT_() \
            (QK_attr_.intNest != static_cast<uint_fast8_t>(0))
    #endif // QK_ISR_CONTEXT_

    // QK-specific scheduler locking
    //! Internal macro to represent the scheduler lock status
    // that needs to be preserved to allow nesting of locks.
    //
    #define QF_SCHED_STAT_ QSchedStatus lockStat_;

    //! Internal macro for selective scheduler locking.
    #define QF_SCHED_LOCK_(prio_) do { \
        if (QK_ISR_CONTEXT_()) { \
            lockStat_ = static_cast<QSchedStatus>(0xFF); \
        } else { \
            lockStat_ = QK::schedLock((prio_)); \
        } \
    } while (false)

    //! Internal macro for selective scheduler unlocking.
    #define QF_SCHED_UNLOCK_() do { \
        if (lockStat_ != static_cast<QSchedStatus>(0xFF)) { \
            QK::schedUnlock(lockStat_); \
        } \
    } while (false)

    // QK-specific native event queue operations...
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        Q_ASSERT_ID(110, (me_)->m_eQueue.m_frontEvt != static_cast<QEvt *>(0))

    #define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
        QK_attr_.readySet.insert((me_)->m_prio); \
        if (!QK_ISR_CONTEXT_()) { \
            if (QK_sched_() != static_cast<uint_fast8_t>(0)) { \
                QK_activate_(); \
            } \
        } \
    } while (false)

    // QK-specific native QF event pool operations...
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_) \
        static_cast<uint_fast16_t>((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_) ((p_).put(e_))

#endif // QP_IMPL

#endif // qk_h
