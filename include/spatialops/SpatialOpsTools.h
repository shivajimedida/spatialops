#ifndef SpatialOpsTools_h
#define SpatialOpsTools_h

#include <spatialops/SpatialOpsConfigure.h>

namespace SpatialOps{

  template<typename T1, typename T2>
  struct IsSameType{ enum{ result=0 }; };

  template< typename T1 >
  struct IsSameType<T1,T1>{ enum{ result=1 }; };


  template<typename FieldT>
  inline size_t nghost(){ return FieldT::Ghost::NM + FieldT::Ghost::NP; }

  template<>
  inline size_t nghost<double>(){ return 0; }

}

#endif

