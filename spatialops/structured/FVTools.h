#ifndef FVToolsTemplates_h
#define FVToolsTemplates_h

#include <spatialops/SpatialOpsConfigure.h>
#include <spatialops/SpatialOpsDefs.h>

#include <spatialops/structured/MemoryWindow.h>

#include <vector>
#include <set>

namespace SpatialOps{
namespace structured{

  /** @file FVToolsTemplates.h
   *  @brief Provides function templates useful for field and operator
   *  creation for structured meshes.  Specialize these templates to
   *  your needs.
   */

  //------------------------------------------------------------------

  template<typename FieldT>
  int get_nx_with_ghost( const int nxNoGhost, const bool hasPlusFaceX )
  {
    int nx = nxNoGhost;
    if( nxNoGhost>1 ) nx += 2*FieldT::Ghost::NGHOST;
    if( hasPlusFaceX && (0==FieldT::Location::FaceDir::value) ) nx+=1;
    return nx;
  }

  template<typename FieldT>
  int get_ny_with_ghost( const int nyNoGhost, const bool hasPlusFaceY )
  {
    int ny = nyNoGhost;
    if( nyNoGhost>1 ) ny += 2*FieldT::Ghost::NGHOST;
    if( hasPlusFaceY && (1==FieldT::Location::FaceDir::value) ) ny+=1;
    return ny;
  }

  template<typename FieldT>
  int get_nz_with_ghost( const int nzNoGhost, const bool hasPlusFaceZ )
  {
    int nz = nzNoGhost;
    if( nzNoGhost>1 ) nz += 2*FieldT::Ghost::NGHOST;
    if( hasPlusFaceZ && (2==FieldT::Location::FaceDir::value) ) nz+=1;
    return nz;
  }

  template<typename FieldT>
  IntVec get_dim_with_ghost( const IntVec& dimNoGhost,
                             const bool hasPlusFaceX, const bool hasPlusFaceY, const bool hasPlusFaceZ )
  {
    return IntVec( get_nx_with_ghost<FieldT>(dimNoGhost[0],hasPlusFaceX),
                   get_ny_with_ghost<FieldT>(dimNoGhost[1],hasPlusFaceY),
                   get_nz_with_ghost<FieldT>(dimNoGhost[2],hasPlusFaceZ) );
  }

  template<typename FieldT>
  int get_ntot_with_ghost( const IntVec& dimNoGhost,
                           const bool hasPlusFaceX, const bool hasPlusFaceY, const bool hasPlusFaceZ )
  {
    const IntVec dim( get_dim_with_ghost<FieldT>( dimNoGhost, hasPlusFaceX, hasPlusFaceY, hasPlusFaceZ ) );
    return dim[0]*dim[1]*dim[2];
  }


  /**
   *  \brief Obtain the memory window for a field on a patch that is a single, contiguous memory block
   *  \param localDim  the dimensionality of the field in consideration (without ghost cells)
   */
  template<typename FieldT>
  MemoryWindow
  get_window_with_ghost( const IntVec& localDim, const bool hasPlusFaceX, const bool hasPlusFaceY, const bool hasPlusFaceZ )
  {
    const IntVec dim( get_nx_with_ghost<FieldT>(localDim[0],hasPlusFaceX),
                      get_ny_with_ghost<FieldT>(localDim[1],hasPlusFaceY),
                      get_nz_with_ghost<FieldT>(localDim[2],hasPlusFaceZ) );
    return MemoryWindow( dim );
  }

  //------------------------------------------------------------------

  /**
   *  \brief Obtain the memory window for a field on a patch that is a subset of a larger memory block
   *
   *  \param globalDim the global dimensionality of the memory block (without ghost cells)
   *  \param localDim  the dimensionality of the field in consideration (without ghost cells)
   *  \param offset    the offset (start ijk index) of the local field in the global address space.
   */
  template<typename FieldT>
  MemoryWindow
  get_window_with_ghost( const IntVec& globalDim, const IntVec& localDim, const IntVec& offset,
                         const bool hasPlusFaceX, const bool hasPlusFaceY, const bool hasPlusFaceZ )
  {
    const IntVec dimLoc( get_nx_with_ghost<FieldT>(localDim[0],hasPlusFaceX),
                         get_ny_with_ghost<FieldT>(localDim[1],hasPlusFaceY),
                         get_nz_with_ghost<FieldT>(localDim[2],hasPlusFaceZ) );
    const IntVec dimGlob( get_nx_with_ghost<FieldT>(globalDim[0],hasPlusFaceX),
                          get_ny_with_ghost<FieldT>(globalDim[1],hasPlusFaceY),
                          get_nz_with_ghost<FieldT>(globalDim[2],hasPlusFaceZ) );
    return MemoryWindow( dimGlob, offset, dimLoc );
  }

  //====================================================================

  // intended for local use only.
  void _ghost_set_( const int ng,
                    const int nxt, const int nyt, const int nzt,
                    const IntVec& dim,
                    const bool hasPlusXSideFaces,
                    const bool hasPlusYSideFaces,
                    const bool hasPlusZSideFaces,
                    size_t& ix,
                    std::set<size_t>& ghostSet );

  //==================================================================

  /**
   *  @brief Obtain the set of indices corresponding to ghost cells
   *  for this field.
   *
   * @param hasPlusXSideFaces A boolean flag to indicate if this patch
   * is on a +x side physical boundary.  If so, then it is assumed
   * that there is an extra face on that side of the domain, and face
   * variable dimensions will be modified accordingly.
   *
   * @param hasPlusYSideFaces A boolean flag to indicate if this patch
   * is on a +y side physical boundary.  If so, then it is assumed
   * that there is an extra face on that side of the domain, and face
   * variable dimensions will be modified accordingly.

   * @param hasPlusZSideFaces A boolean flag to indicate if this patch
   * is on a +z side physical boundary.  If so, then it is assumed
   * that there is an extra face on that side of the domain, and face
   * variable dimensions will be modified accordingly.
   *
   * @todo Remove default values.  This is very dangerous for parallel
   * computations to have a default value for the + side information.
   *
   *  \todo JCS: need to rework this.  It only works for the "old"
   *  memory view.  Need to integrate this with the MemoryWindow
   *  stuff.
   */
  template<typename FieldT> 
  const std::set<size_t> get_ghost_set( const IntVec& dim,
                                        const bool hasPlusXSideFaces=true,
                                        const bool hasPlusYSideFaces=true,
                                        const bool hasPlusZSideFaces=true )
  {
    std::set<size_t> ghostSet;
    ghostSet.clear();
    size_t ix=0;
    _ghost_set_( FieldT::Ghost::NGHOST,
                 get_nx_with_ghost<FieldT>(dim[0],hasPlusXSideFaces),
                 get_ny_with_ghost<FieldT>(dim[1],hasPlusYSideFaces),
                 get_nz_with_ghost<FieldT>(dim[2],hasPlusZSideFaces),
                 dim,
                 hasPlusXSideFaces, hasPlusYSideFaces, hasPlusZSideFaces,
                 ix,
                 ghostSet );
    return ghostSet;
  }

  //==================================================================

  /**
   *  @brief Use this to transform a flat index to i,j,k indices.
   */
  template<typename FieldT>
  struct flat2ijk
  {
    static IntVec value( const IntVec& dim, const int ix,
                         const bool hasPlusXSideFaces=true,
                         const bool hasPlusYSideFaces=true,
                         const bool hasPlusZSideFaces=true );
  };

  //==================================================================

  /**
   *  @brief Use this to transform i,j,k indices to a flat index.
   */
  template<typename FieldT>
  struct ijk2flat
  {
    static int value( const IntVec& dim, const IntVec& ixt,
                      const bool hasPlusXSideFaces=true,
                      const bool hasPlusYSideFaces=true,
                      const bool hasPlusZSideFaces=true );
  };

  //====================================================================

  template<typename FieldT>
  inline IntVec
  flat2ijk<FieldT>::value( const IntVec& dim, const int ix,
                           const bool hasPlusXSideFaces, const bool hasPlusYSideFaces, const bool hasPlusZSideFaces )
  {
    IntVec triplet(0,0,0);

    const int nxt = get_nx_with_ghost<FieldT>(dim[0],hasPlusXSideFaces);
    const int nyt = get_ny_with_ghost<FieldT>(dim[1],hasPlusXSideFaces);
    triplet[0] = ix%nxt;
    triplet[1] = ix/nxt % nyt;
    triplet[2] = ix/(nxt*nyt);

    return triplet;
  }

  //==================================================================

  template<typename FieldT>
  inline int
  ijk2flat<FieldT>::value( const IntVec& dim, const IntVec& triplet,
                           const bool hasPlusXSideFaces, const bool hasPlusYSideFaces, const bool hasPlusZSideFaces )
  {
    const int nxt = get_nx_with_ghost<FieldT>(dim[0],hasPlusXSideFaces);
    const int nyt = get_ny_with_ghost<FieldT>(dim[1],hasPlusYSideFaces);
      
    return
      triplet[0] +
      triplet[1] * nxt +
      triplet[2] * nxt*nyt;
  }

  //==================================================================

}// namespace structured
}// namespace SpatialOps

#endif