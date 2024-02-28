#ifndef _H_AMoRE200_DEFINITIONS__
#define _H_AMoRE200_DEFINITIONS__

static constexpr double ns = 1;
static constexpr double us = 1e3 * ns;
static constexpr double ms = 1e6 * ns;
static constexpr double s = 1e9 * ns;

constexpr int number_of_scintillators = 132;
constexpr size_t max_nprimary = 100;






constexpr size_t max_ncrystals = 423; // for amore-phase2 180kg LMOs,  630 (for 1st designed version. 70 tower), 90 (for amore-phase1)
//constexpr size_t max_ncrystals = 432; // for amore-phase2 178kg LMOs,  630 (for 1st designed version. 70 tower), 90 (for amore-phase1)

#endif
