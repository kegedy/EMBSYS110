/// @file
/// @ingroup qf
/// @brief Internal (package scope) QF/C++ interface.
/// @cond
///***************************************************************************
/// Last updated for version 5.9.7
/// Last updated on  2017-08-25
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

#ifndef qf_pkg_h
#define qf_pkg_h

//! helper macro to cast const away from an event pointer @p e_
#define QF_EVT_CONST_CAST_(e_) const_cast<QEvt *>(e_)

// QF-specific critical section...
#ifndef QF_CRIT_STAT_TYPE
    //! This is an internal macro for defining the critical section
    //! status type.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// provides the definition of the critical section status variable.
    /// Otherwise this macro is empty.
    /// @sa #QF_CRIT_STAT_TYPE
    #define QF_CRIT_STAT_

    //! This is an internal macro for entering a critical section.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes #QF_CRIT_ENTRY passing the key variable as the parameter.
    /// Otherwise #QF_CRIT_ENTRY is invoked with a dummy parameter.
    /// @sa #QF_CRIT_ENTRY
    #define QF_CRIT_ENTRY_()    QF_CRIT_ENTRY(dummy)

    //! This is an internal macro for exiting a critical section.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes #QF_CRIT_EXIT passing the key variable as the parameter.
    /// Otherwise #QF_CRIT_EXIT is invoked with a dummy parameter.
    /// @sa #QF_CRIT_EXIT
    ///
    #define QF_CRIT_EXIT_()     QF_CRIT_EXIT(dummy)

#else
    #define QF_CRIT_STAT_       QF_CRIT_STAT_TYPE critStat_;
    #define QF_CRIT_ENTRY_()    QF_CRIT_ENTRY(critStat_)
    #define QF_CRIT_EXIT_()     QF_CRIT_EXIT(critStat_)
#endif  // QF_CRIT_STAT_TYPE


namespace QP {

// package-scope objects -----------------------------------------------------
extern QF_EPOOL_TYPE_ QF_pool_[QF_MAX_EPOOL]; //!< allocate event pools
extern uint_fast8_t QF_maxPool_;     //!< # of initialized event pools
extern QSubscrList *QF_subscrList_;  //!< the subscriber list array
extern enum_t QF_maxPubSignal_;      //!< the maximum published signal

//............................................................................
//! Structure representing a free block in the Native QF Memory Pool
/// @sa QP::QMPool
struct QFreeBlock {
    QFreeBlock * volatile m_next;    //!< link to the next free block
};

//****************************************************************************
// internal helper inline functions

//! increment the refCtr_ of an event @p e
inline void QF_EVT_REF_CTR_INC_(QEvt const * const e) {
    ++(QF_EVT_CONST_CAST_(e))->refCtr_;
}

//! decrement the refCtr_ of an event @p e
inline void QF_EVT_REF_CTR_DEC_(QEvt const * const e) {
    --(QF_EVT_CONST_CAST_(e))->refCtr_;
}

//! macro to test that a pointer @p x_ is in range between @p min_ and @p max_
/// @description
/// This macro is specifically and exclusively used for checking the range
/// of a block pointer returned to the pool. Such a check must rely on the
/// pointer arithmetic not compliant with the MISRA-C++:2008 rules ??? and
/// ???. Defining a specific macro for this purpose allows to selectively
/// disable the warnings for this particular case.
#define QF_PTR_RANGE_(x_, min_, max_)  (((min_) <= (x_)) && ((x_) <= (max_)))

} // namespace QP


//! access element at index @p i_ from the base pointer @p base_
#define QF_PTR_AT_(base_, i_) (base_[i_])

//****************************************************************************
#ifdef Q_SPY  // QS software tracing enabled?

    #if (QF_EQUEUE_CTR_SIZE == 1)

        //! Internal QS macro to output an unformatted event queue
        //! counter data element
        /// @note the counter size depends on the macro #QF_EQUEUE_CTR_SIZE.
        #define QS_EQC_(ctr_)       QS::u8_(static_cast<uint8_t>(ctr_))
    #elif (QF_EQUEUE_CTR_SIZE == 2)
        #define QS_EQC_(ctr_)       QS::u16_(static_cast<uint16_t>(ctr_))
    #elif (QF_EQUEUE_CTR_SIZE == 4)
        #define QS_EQC_(ctr_)       QS::u32_(static_cast<uint32_t>(ctr_))
    #else
        #error "QF_EQUEUE_CTR_SIZE not defined"
    #endif


    #if (QF_EVENT_SIZ_SIZE == 1)

        //! Internal QS macro to output an unformatted event size
        //! data element
        /// @note the event size depends on the macro #QF_EVENT_SIZ_SIZE.
        #define QS_EVS_(size_)      QS::u8_(static_cast<uint8_t>(size_))
    #elif (QF_EVENT_SIZ_SIZE == 2)
        #define QS_EVS_(size_)      QS::u16_(static_cast<uint16_t>(size_))
    #elif (QF_EVENT_SIZ_SIZE == 4)
        #define QS_EVS_(size_)      QS::u32_(static_cast<uint32_t>(size_))
    #endif


    #if (QF_MPOOL_SIZ_SIZE == 1)

        //! Internal QS macro to output an unformatted memory pool
        /// block-size data element
        /// @note the block-size depends on the macro #QF_MPOOL_SIZ_SIZE.
        #define QS_MPS_(size_)      QS::u8_(static_cast<uint8_t>(size_))
    #elif (QF_MPOOL_SIZ_SIZE == 2)
        #define QS_MPS_(size_)      QS::u16_(static_cast<uint16_t>(size_))
    #elif (QF_MPOOL_SIZ_SIZE == 4)
        #define QS_MPS_(size_)      QS::u32_(static_cast<uint32_t>(size_))
    #endif

    #if (QF_MPOOL_CTR_SIZE == 1)

        //! Internal QS macro to output an unformatted memory pool
        //! block-counter data element
        /// @note the counter size depends on the macro #QF_MPOOL_CTR_SIZE.
        #define QS_MPC_(ctr_)       QS::u8_(static_cast<uint8_t>(ctr_))
    #elif (QF_MPOOL_CTR_SIZE == 2)
        #define QS_MPC_(ctr_)       QS::u16_(static_cast<uint16_t>(ctr_))
    #elif (QF_MPOOL_CTR_SIZE == 4)
        #define QS_MPC_(ctr_)       QS::u32_(static_cast<uint32_t>(ctr_))
    #endif


    #if (QF_TIMEEVT_CTR_SIZE == 1)

        //! Internal QS macro to output an unformatted time event
        //! tick-counter data element
        /// @note the counter size depends on the macro #QF_TIMEEVT_CTR_SIZE.
        #define QS_TEC_(ctr_)       QS::u8_(static_cast<uint8_t>(ctr_))
    #elif (QF_TIMEEVT_CTR_SIZE == 2)
        #define QS_TEC_(ctr_)       QS::u16_(static_cast<uint16_t>(ctr_))
    #elif (QF_TIMEEVT_CTR_SIZE == 4)
        #define QS_TEC_(ctr_)       QS::u32_(static_cast<uint32_t>(ctr_))
    #endif

#endif  // Q_SPY

#endif  // qf_pkg_h
