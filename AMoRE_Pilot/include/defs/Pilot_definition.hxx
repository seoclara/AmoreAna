#ifndef _PILOT_DEFINITIONS__H_
#define _PILOT_DEFINITIONS__H_ 1
#include <cstddef>

namespace MCAnlysDef {
    namespace AMoRE_Pilot {
        /*!
         *  @enum eScintNum
         *  @brief Too trivial...
         */
        enum eScintNum {
            kErrorType = 0,
            kTop1,
            kTop2,
            kLeft1,
            kLeft2,
            kRight1,
            kRight2,
            kFront1,
            kFront2,
            kBack1,
            kBack2
        };
        /*! @enum eEvtTag
         *  @brief The tag for types of an event.
         *  @details kNoIncident: The primary muon does not passed any pannel
         *           k~Only: The incident muon has been passed each side pannel only.
         *           kA2B: The incident muon has been passed from A side pannel to B side pannel
         *           */
        enum eEvtTag {
            kNoIncident = 0,
            kTOnly,
            kLOnly,
            kROnly,
            kFOnly,
            kBOnly,
            kT2S,
            kS2T,
            kS2S,
            kErrorEvt
        };
    } // namespace AMoRE_Pilot
} // namespace MCAnlysDef

static constexpr double ns = 1;
static constexpr double us = 1e3 * ns;
static constexpr double ms = 1e6 * ns;
static constexpr double s  = 1e9 * ns;

constexpr size_t maxcmonum   = 6;
constexpr size_t maxscintnum = 10;

const int fcPanelMapper[11] = {-1, 0, 1, 2, 3, 4, 5, 8, 9, 6, 7};
const int ppc[10]           = {4, 4, 3, 3, 3, 3, 2, 2, 2, 2};
const char PanelName[10][3] = {"T1", "T2", "L1", "L2", "R1", "R2", "B1", "B2", "F1", "F2"};

#endif
