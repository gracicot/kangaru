#ifndef KANGARU5_DETAIL_UNDEF_HPP
#define KANGARU5_DETAIL_UNDEF_HPP

#ifndef KANGARU5_DETAIL_DEFINE_HPP
#error "Must include define.hpp before including undef.hpp"
#endif

#undef KANGARU5_NO_UNIQUE_ADDRESS
#undef KANGARU5_FWD

// We make those header re-includable
#undef KANGARU5_DETAIL_DEFINE_HPP
#undef KANGARU5_DETAIL_UNDEF_HPP

#endif // KANGARU5_DETAIL_UNDEF_HPP
