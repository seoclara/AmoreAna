#ifndef _I_DEFINITIONS__H_
#define _I_DEFINITIONS__H_ 1
#include <cstddef>

namespace MCAnlysDef {
	namespace AMoRE_I {
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
	} // namespace AMoRE_I
} // namespace MCAnlysDef

static constexpr double ns = 1;
static constexpr double us = 1e3 * ns;
static constexpr double ms = 1e6 * ns;
static constexpr double s  = 1e9 * ns;

constexpr size_t max_ncrystals    = 18;
constexpr size_t maxscintnum  = 10;
constexpr size_t max_nprimary = 100;

constexpr int fcPanelMapper[maxscintnum + 1] = {-1, 0, 1, 2, 3, 4, 5, 8, 9, 6, 7};
constexpr int ppc[maxscintnum]               = {4, 4, 3, 3, 3, 3, 2, 2, 2, 2};
constexpr char PanelName[maxscintnum][3]     = {"T1", "T2", "L1", "L2", "R1",
	"R2", "B1", "B2", "F1", "F2"};

/*
   constexpr char crystalNameList[max_ncrystals][10] = {"SE09", "SE07", "SE03", "LMO5", "SE06", "SE05",
   "SE04", "SB28", "LMO1", "SS68", "S35", "LMO3",
   "LMO4", "LMO2", "SE08", "SE02", "SE01", "SB29"};
   constexpr double crystalMassList[max_ncrystals] = {0.35423, 0.357631, 0.426066, 0.3775, 0.35553, 0.373239,
   0.47329, 0.195949, 0.300102, 0.352028, 0.25598, 0.311508,
   0.308106, 0.312008, 0.358232, 0.340022, 0.353029, 0.390048};
   */
// crystal name and mass in order of real channel number
constexpr char crystalNameList[max_ncrystals][10] = {"SB29","SE01","SE02","SE08",
	"LMO2","LMO4","LMO3","S35","SS68",
	"LMO5","SE03","SE07","SE09",
	"LMO1","SB28","SE04","SE05","SE06"};

constexpr double crystalMassList[max_ncrystals] = {0.390048, 0.353029, 0.340022, 0.358232,
	0.312008, 0.308106, 0.311508, 0.25598, 0.352028,
	0.377541, 0.426066, 0.357631, 0.35423,
	0.300102, 0.195949, 0.47329, 0.373239, 0.35553};

constexpr int realChannelList[max_ncrystals] = {12, 11, 10, 9, 
	17, 16, 15, 14, 13, 
	8, 7, 6, 5, 
	4, 3, 2, 1, 0}; // simulation channel to real channel

constexpr int channelConvert[max_ncrystals] = {17, 16, 15, 14, 
	13, 12,	11, 10, 9, 
	3, 2,	1, 0, 
	8, 7, 6, 5, 4};  // realchannel to simulation channel
/*
   constexpr double crystalMassList[max_ncrystals] = {0.330501, 0.341586, 0.402638, 0.332437, 0.329192, 0.354255,
   0.435669, 0.190542, 0.25681, 0.297723, 0.263964, 0.263913,
   0.261824, 0.262033, 0.340776, 0.322879, 0.333255, 0.408524};
   */

#endif
