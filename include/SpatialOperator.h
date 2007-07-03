#ifndef UT_SpatialOperator_h
#define UT_SpatialOperator_h

#include <boost/static_assert.hpp>

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <sstream>

#include <SpatialOpsDefs.h>
#include <SpatialField.h>

namespace SpatialOps{

  template<typename T1, typename T2>
  struct IsSameType{
    enum{ result=0 };
  };
  template< typename T1 >
  struct IsSameType<T1,T1>{
    enum{ result=1 };
  };



  template<class Ghost>
  std::vector<int> ghost_vec()
  {
    std::vector<int> ng(6,0);
    std::vector<int>::iterator ig = ng.begin();
    *ig++ = Ghost::template get<XDIR,SideMinus>();
    *ig++ = Ghost::template get<XDIR,SidePlus >();
    *ig++ = Ghost::template get<YDIR,SideMinus>();
    *ig++ = Ghost::template get<YDIR,SidePlus >();
    *ig++ = Ghost::template get<ZDIR,SideMinus>();
    *ig++ = Ghost::template get<ZDIR,SidePlus >();
    return ng;
  }


  //====================================================================


  /**
   *  @struct OpAssemblerSelector
   *  @author James C. Sutherland
   *
   *  @brief Defines the type of Assembler required to build a given
   *  SpatialOperator.
   *
   *  This should be specialized for each operator type to provide the
   *  type of assembler required for the operator.
   *
   *  @par Template Parameters
   *    \li \b OpType The type of operator.
   *    \li \b Dir    The direction that this operator acts on.
   *    \li \b Location The location for this operator
   *    \li \b SrcGhost Specifies ghosting information for the source field.
   *    \li \b SrcGhost Specifies ghosting information for the destination field.
   *
   *  Specialized versions of the OpAssemblerSelector struct must
   *  specialize one or more of the template arguments. As this is an
   *  empty struct, it simply defines an interface.  Thus, if a
   *  specialized match is not found, the compiler will fail.
   *
   *  Specialized versions must provide a typedef that defines an \c
   *  Assembler type which is the type of Assembler to be used to
   *  construct the SpatialOperator.  This is required as an input to
   *  the constructor.  As an example, assume that we have an operator
   *  assembler to build a SpatialOperator of type \c MyOp - let's
   *  call it \c MyOpAssembler If MyOp was templated on the direction,
   *  location, and ghosting of the source and destination fields,
   *  e.g.,
   *
   *  \code
   *    template< typename Direction,
   *              typename Location,
   *              typename SrcGhost,
   *              typename DestGhost >
   *    class MyOpAssembler
   *    {
   *      ...
   *    };
   *  \endcode
   *
   *  Then we could define an OpAssemblerSelector for \c MyOp objects as
   *
   *  \code
   *    template< typename Direction,
   *              typename Location,
   *              typename SrcGhost,
   *              typename DestGhost >
   *    OpAssemblerSelector< MyOp, Direction, Location, SrcGhost, DestGhost >
   *    {
   *      typedef MyOpAssembler<Direction,Location,SrcGhost,Direction>  Assembler;
   *    };
   *  \endcode
   *
   *
   *  @par The Assembler
   *
   *  An Assembler must provide the following methods:
   *
   *   \li A method to return a vector containing the number of points
   *   in each direction (excluding ghost cells) that this operator
   *   should be constructed for. This should have the following
   *   signature: \code const std::vector<int>& get_extent() \endcode
   *
   *   \li A method to return the number of rows in the operator.
   *   This should have the following signature: \code int get_nrows()
   *   \endcode
   *
   *   \li A method to return the number of columns in the operator.
   *   This should have the following signature: \code int get_ncols()
   *   \endcode
   *
   *   \li A method to return the nonzero entries in a given row,
   *   together with the indices of the columns corresponding to the
   *   nonzero entries.  This method should have the following
   *   signature: \code void get_row_entries( const int irow,
   *   std::vector<double>& vals, std::vector<int>& ixs ) \endcode
   *
   *   @par More Examples
   *
   *   See FV2ndOrderTypes.h for an example of defining
   *   OpAssemblerSelector structs and see FVStaggeredSpatialOps.h for
   *   examples of defining assemblers for various operators.
   */
  template< typename OpType,
	    typename Dir,
	    typename Location,
	    typename SrcGhost,
	    typename DestGhost >
  struct OpAssemblerSelector{};



  //====================================================================


  /**
   *  @class  SpatialOperator
   *  @author James C. Sutherland
   *  @date   December, 2006
   *
   *  Provides support for discrete spatial operators.
   *
   *  @par Template Parameters
   *  <ul>
   *
   *    <li> \b LinAlg The Linear Algebra type for use with this
   *    SpatialOperator.  This must define:
   *    <ul>
   *      <li> \b MatType This defines the type for the underlying Matrix operator.
   *    </ul>
   *
   *    <li> \b OpType This defines the type of the operator.  This is
   *    really only used to distinguish various types of operators.
   *    In many cases, an empty struct will suffice for this type.
   *
   *    <li> \b Direction Specifies the direction that this operator
   *    applies in.  See @link DirectionDefinitions here @endlink for
   *    the predefined direction types.  These are also "empty" types
   *    in the sense that they do not provide functionality, only type
   *    information.
   *
   *    <li> \b SrcFieldTraits Specifies information about the field
   *    type that this operator acts on.  It must define several
   *    things:
   *
   *    <ul>
   *
   *      <li>\b StorageLocation Defines where this field is located.
   *    This is a user-defined type, and is an "empty" type - it
   *    provides only type information and no functionality.  It is
   *    never instantiated.
   *
   *      <li> \b GhostTraits Defines information about ghosting.
   *      This should provide a templated method,
   *
   *       \code
   *         template<typename Dir, typename SideType> static int get();
   *       \endcode
   *
   *      which returns the number of ghost cells in a given direction
   *      and given side of the patch.  This must be a static method,
   *      and may be specialized to deal with different ghosting on
   *      different faces of a patch.
   *
   *    </ul>
   *
   *    <li> \b DestFieldTraits Specifies information about the field
   *    type that this operator produces.  It must define the same
   *    things as the SrcFieldTraits type does.
   *  </ul>
   *
   *
   *  @par The OpAssemblerSelector
   * 
   *  All operators have an associated assembler object that must be
   *  instantiated and passed to the SpatialOperator constructor.  The
   *  full type of the OpAssemblerSelector is determined from the
   *  template parameters to the SpatialOperator.  However, the user
   *  must provide specializations of the OpAssemblerSelector concept
   *  that have the required functionality.  See documentation on
   *  OpAssemblerSelector for more details.
   */
  template< typename LinAlg,           // linear algebra support for this operator
	    typename OpType,           // type of operator
	    typename Direction,        // direction that the operator acts in
	    typename SrcFieldTraits,   // information on the source field (field operator acts on)
	    typename DestFieldTraits > // information on the dest field (field operator produces)
  class SpatialOperator
  {
  public:

    typedef LinAlg                                     LinAlgType;
    typedef typename LinAlg::MatType                   MatType;

    typedef typename SrcFieldTraits::StorageLocation   OpLocation;
    typedef Direction                                  DirType;

    typedef typename SrcFieldTraits::GhostTraits       SrcGhost;
    typedef typename SrcFieldTraits::StorageLocation   SrcLocation;

    typedef typename DestFieldTraits::GhostTraits      DestGhost;
    typedef typename DestFieldTraits::StorageLocation  DestLocation;

    typedef typename OpAssemblerSelector
                       < OpType,
			 Direction,
			 OpLocation,
			 SrcGhost,
			 DestGhost >::Assembler        Assembler;

  public:

    /**
     *  Construct a SpatialOperator.
     *
     *  @param opAssembler The assembler is a strongly typed object
     *  that provides information required to construct this
     *  operator. The assembler type is defined by the
     *  OpAssemblerSelector, which must be specialized by the client
     *  who is building the SpatialOperator.  The assembler provides
     *  methods required to construct the operator.  See documentation
     *  on OpAssemblerSelector for more information.  Since the
     *  assembler is only required during construction, it may be a
     *  temporary object that can be destroyed after a call to this
     *  constructor.
     */
    SpatialOperator( Assembler & opAssembler );

    virtual ~SpatialOperator();

    /**
     *  @name 
     *  Return the underlying linear algebra matrix representation 
     */
    //@{
    inline       MatType& get_linalg_mat()      { return mat_; }
    inline const MatType& get_linalg_mat() const{ return mat_; }
    //@}

    /**
     *  @brief Apply this operator to the supplied source field to
     *  produce the supplied destination field.
     *
     *  Calculates the matrix-vector product (dest)=[Op](src).
     *
     *  @param src  The field to apply the operator to.
     *  @param dest The resulting field.
     */
    inline void apply_to_field( const SpatialField<LinAlg,SrcLocation,SrcGhost> & src,
				SpatialField<LinAlg,DestLocation,DestGhost> & dest ) const;


    /**
     *  @brief Apply this operator to another operator to produce a third.
     *
     *  Calculates the matrix product [D] = [Op][S], where [D]
     *  represents the destination operator, [S] represents the
     *  "source" operator, and [Op] represents this operator.
     *
     *  @param src  The operator that we act on to produce the result.
     *  @param dest The resulting operator.
     */
    template< class SrcOp, class DestOp >
    inline void apply_to_op( const SrcOp& src, DestOp& dest ) const;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /**
     *  @name Matrix Operators
     *
     *  Operators to add/subtract/assign another spatial operator with
     *  the same direction, and source/destination traits.
     */
    //@{
    template< typename OT >
    inline SpatialOperator& operator=( const SpatialOperator<LinAlg,OT,Direction,SrcFieldTraits,DestFieldTraits>& );

    template< typename OT >
    inline SpatialOperator& operator+=( const SpatialOperator<LinAlg,OT,Direction,SrcFieldTraits,DestFieldTraits>& );

    template< typename OT >
    inline SpatialOperator& operator-=( const SpatialOperator<LinAlg,OT,Direction,SrcFieldTraits,DestFieldTraits>& );
    //@}

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /**
     *  @name Field Operators
     *  Operators to add/subtract a SpatialField from the diagonal of this operator.
     */
    //@{
    /** @brief sum the given field into the diagonal */
    inline SpatialOperator& operator+=( const SpatialField<LinAlg,DestLocation,DestGhost>& f ){ linAlg_+=f; return *this; }

    /** @brief subtract the given field from the diagonal */
    inline SpatialOperator& operator-=( const SpatialField<LinAlg,DestLocation,DestGhost>& f ){ linAlg_-=f; return *this; }
    //@}

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /**
     *  Scale the matrix such that A(i,j) = x(i)*A(i,j) where i denotes
     *  the row number of A and j denotes the column number of A.
     */
    inline void left_scale( const SpatialField<LinAlg,DestLocation,DestGhost>& s )
    {
      linAlg_.left_scale( s.get_linalg_vec() );
    }

    /**
     *  Scale the matrix such that A(i,j) = x(j)*A(i,j) where i denotes
     *  the global row number of A and j denotes the global column
     *  number of A.
     */
    inline void right_scale( const SpatialField<LinAlg,SrcLocation,SrcGhost> & a )
    {
      linAlg_.right_scale( a.get_linalg_vec() );
    }


    /** @brief reset the coefficients in the matrix */
    inline void reset_entries( const double val = 0 )
    {
      linAlg_.reset_entries( val );
    }


    /** @brief Obtain the number of rows in this operator */
    inline int nrows() const{ return nrows_; }

    /** @brief Obtain the number of columns in this operator */
    inline int ncols() const{ return ncols_; }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /**
     * @name Ghost Information - All Faces
     */
    //@{

    /** @brief Obtain the number of ghost cells for the source field.
     *  Returns an array with 6 entries corresponding to the number of
     *  ghost cells on each side of the patch, ordered as
     *  <code>(-x,+x,-y,+y,-z,+z)</code>
     */
    inline const std::vector<int>& nghost_src()  const{return nghostSrc_ ;}

    /** @brief Obtain the number of ghost cells for the destination field.
     *  Returns an array with 6 entries corresponding to the number of
     *  ghost cells on each side of the patch, ordered as
     *  <code>(-x,+x,-y,+y,-z,+z)</code>
     */
    inline const std::vector<int>& nghost_dest() const{return nghostDest_;}

    //@}

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /**
     *  @name Ghost Information - Specific Face
     *
     *  Obtain the number of ghost cells for a particular face of the
     *  patch for the source/destination field corresponding to this
     *  operator.
     *
     *  The two relevant methods are
     *  \code nghost_src<Dir,Side>() \endcode
     *   and
     *  \code nghost_dest<Dir,Side>() \endcode
     *  which provide ghosting
     *  information for the source and destination fields valid for
     *  this operator.
     *
     *  Template Parameters:
     *    \li \b Dir Specifies the direction of interest.  See
     *    @link DirectionDefinitions here @endlink for options.
     *    \li \b SideType Specifies the side of interest.  See
     *    @link SideDefinitions here @endlink for options.
     */
    //@{

    /** @brief Ghost info for source fields on a specific face */
    template< typename Dir, typename SideType >
    inline int nghost_src() const
    {
      return DestGhost::template get<Dir,SideType>();
    }

    /** @brief Ghost info for destination fields on a specific face */
    template< typename Dir, typename SideType >
    inline int nghost_dest() const
    {
      return DestGhost::template get<Dir,SideType>();
    }

    //@}

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /** @brief Obtain the patch extent, i.e. the number of points in
	each direction for the patch.  This excludes ghost cells.  It
	returns a vector with elements <code>(nx,ny,nz)</code> */
    inline const std::vector<int> & get_extent() const{ return extent_; }


    struct IndexTriplet
    {
      int index[3];
      inline int& operator[](const int i){return index[i];}
    };

    /** if provided, the IndexTriplet is populated with the interior ijk index for non-ghost entries.*/
    bool is_col_ghost( const int colnum, IndexTriplet* const ix=NULL ) const;
    bool is_row_ghost( const int rownum, IndexTriplet* const ix=NULL ) const;


    /** @brief Print the operator's entries to the specified output stream. */
    void Print( std::ostream & ) const;

  protected:


    /**
     *  Check compatibility of this operator applied on the one provided
     *  as an argument.  The base class implementation simply performs
     *  dimensional compatibility checks.  Derived classes may
     *  specialize this method further.
     */
    template< typename T >
    inline bool compatibility_check( const T& op,
				     const bool isResultOp ) const;

    /**
     * Check compatability of a field with this operator.  The base
     * class method simply checks for dimensional compatibility.
     * Derived classes may/should specialize this method to ensure
     * proper ghost availability.
     */
    template< typename FieldT >
    inline bool compatibility_check( const FieldT & field ) const;

    /** Insert a row into the matrix */
    inline void insert_row_entry( const int rownum,
				  std::vector<double> & rowValues,
				  std::vector<int> & rowIndices );

    /** Sum a row into the matrix */
    inline void sum_into_row( const int rownum,
			      std::vector<double> & rowValues,
			      std::vector<int> & rowIndices );

    const std::vector<int> extent_;

  private:

    const int nrows_, ncols_;
    const std::vector<int> nghostSrc_, nghostDest_;
    LinAlg linAlg_;
    MatType & mat_;

  };


  //====================================================================



  /**
   *  @class  SpatialOpDatabase
   *  @author James C. Sutherland
   *  @date   December, 2006
   *
   *  Factory for SpatialOperator objects.  These objects are registered
   *  here (created externally, ownership is transferred) and can be
   *  recalled for later use.
   *
   *  Note that one can have multiple operators registered for a given
   *  type, and activate them as needed.  This allows the potential for
   *  dynamic operator switching.
   *
   *  @todo Need to allow the ability to put multiple operators in a
   *  database and retrieve them, potentially with locking capability to prevent multiple access.
   */
  template< class SpatialOpType >
  class SpatialOpDatabase
  {
  public:

    typedef typename SpatialOpType::DirType     Direction;
    typedef typename SpatialOpType::OpLocation  OpLocation;
    typedef typename SpatialOpType::SrcGhost    SrcGhost;
    typedef typename SpatialOpType::DestGhost   DestGhost;


    static SpatialOpDatabase& self();

    /**
     *  Registers a new operator.
     *
     *  @param op : The operator itself.  Ownership is transfered.  This
     *  must be a heap-allocated object (build via "new")
     */
    void register_new_operator( SpatialOpType * const op );

    /**
     *  Obtain the spatial operator with the given shape.
     *  Throws an exception if no match is found.
     *
     *  This pointer reference can change if the default operator for
     *  this type is changed via a call to
     *  <code>set_default_operator()</code> or
     *  <code>register_new_operator()</code>.
     */
    SpatialOpType*& retrieve_operator( const std::vector<int> & nxyz );

    /**
     *  determine if an operator of this size exists in the databse
     */
    bool query_operator( const std::vector<int>& nxyz ) const;

  private:

    SpatialOpDatabase();
    ~SpatialOpDatabase();

    SpatialOpDatabase(const SpatialOpDatabase&);
    SpatialOpDatabase&  operator=(const SpatialOpDatabase&);

    struct Shape
    {
      Shape( const std::vector<int> & extent );
      bool operator < ( const Shape& s ) const;
      const std::vector<int> nxyz;
    };
    
    typedef std::map< Shape, SpatialOpType* > ShapeOpMap;
    ShapeOpMap   shapeOpMap_;
  };


  //====================================================================









  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //
  //  Implementation
  //
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%









  //==================================================================


  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  SpatialOperator( Assembler & opAssembler )
    : extent_( opAssembler.get_extent() ),
      nrows_ ( opAssembler.get_nrows()  ),
      ncols_ ( opAssembler.get_ncols()  ),
      nghostSrc_ ( ghost_vec< SrcGhost>() ),
      nghostDest_( ghost_vec<DestGhost>() ),
      mat_( linAlg_.setup_matrix( nrows_, ncols_, Assembler::num_nonzeros() ) )
  {
    if( IsSameType<Direction,XDIR>::result )  assert(  extent_[0]>1 );
    if( IsSameType<Direction,YDIR>::result )  assert( (extent_[0]>1)  && (extent_[1]>1) );
    if( IsSameType<Direction,ZDIR>::result )  assert( (extent_[0]>1)  && (extent_[1]>1) && (extent_[2]>1) );

    // build the operator
    std::vector<double> vals( Assembler::num_nonzeros(), 0.0 );
    std::vector<int>    ixs ( Assembler::num_nonzeros(), 0   );
    for( int i=0; i<nrows_; ++i ){
      vals.clear();
      ixs.clear();
      opAssembler.get_row_entries( i, vals, ixs );
      insert_row_entry( i, vals, ixs );
    }
    linAlg_.finalize();
  }
  //------------------------------------------------------------------
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  ~SpatialOperator()
  {
    linAlg_.destroy_matrix();
  }
  //------------------------------------------------------------------
  /**
   *  Apply this SpatialOperator to a field.  The source and
   *  destination fields must have compatible dimension and ghosting
   *  for use with this operator.
   */
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  void
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  apply_to_field( const SpatialField< LinAlg, typename SrcFieldTraits::StorageLocation, typename SrcFieldTraits::GhostTraits > & src,
		  SpatialField< LinAlg, typename DestFieldTraits::StorageLocation, typename DestFieldTraits::GhostTraits > & dest ) const
  {
    assert( src.get_extent()[0] == dest.get_extent()[0] );
    assert( src.get_extent()[1] == dest.get_extent()[1] );
    assert( src.get_extent()[2] == dest.get_extent()[2] );
    assert( src.get_extent()[0] ==      get_extent()[0] );
    assert( src.get_extent()[1] ==      get_extent()[1] );
    assert( src.get_extent()[2] ==      get_extent()[2] );

    linAlg_.multiply( src.get_linalg_vec(), dest.get_linalg_vec() );
  }

  //------------------------------------------------------------------
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  template< class SrcOp, class DestOp >
  void
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  apply_to_op( const SrcOp& src, DestOp& dest ) const
  {
    BOOST_STATIC_ASSERT( bool( IsSameType< SrcGhost,  typename SrcOp::DestGhost>::result ));
    BOOST_STATIC_ASSERT( bool( IsSameType< DestGhost, typename SrcOp::SrcGhost >::result ));
    assert( compatibility_check(src, false) );
    assert( compatibility_check(dest,true ) );
    linAlg_.multiply( src.get_linalg_mat(), dest.get_linalg_mat() );
  }
  //------------------------------------------------------------------
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  template< typename OT >
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>&
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  operator=( const SpatialOperator<LinAlg,OT,Direction,SrcFieldTraits,DestFieldTraits>& op )
  {
    linAlg_ = op;
    return *this;
  }
  //------------------------------------------------------------------
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  template< typename OT >
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>&
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  operator+=( const SpatialOperator<LinAlg,OT,Direction,SrcFieldTraits,DestFieldTraits>& op )
  {
    linAlg_ += op;
    return *this;
  }
  //------------------------------------------------------------------
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  template< typename OT >
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>&
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  operator-=( const SpatialOperator<LinAlg,OT,Direction,SrcFieldTraits,DestFieldTraits>& op )
  {
    linAlg_ -= op;
    return *this;
  }
  //------------------------------------------------------------------
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  bool
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  is_row_ghost( const int irow,
		IndexTriplet* const ix ) const
  {
    const bool getIxs = (ix != NULL);
    if( getIxs ){
      IndexTriplet & i= *ix;
      i[0] = -1;
      i[1] = -1;
      i[2] = -1;
    }

    bool isGhost = false;

    // is this row a ghost entry?  Row corresponds to entry in dest vec.
    const int nxd = extent_[0] + nghostDest_[0] + nghostDest_[1];
    const int idest = irow % nxd - nghostDest_[0];
    if( idest < 0  ||  idest >= extent_[0] ){
      isGhost = true;
      if( !getIxs ) return true;
    }
    if( getIxs ) (*ix)[0] = idest;

    const int nyd = extent_[1] + nghostDest_[2] + nghostDest_[3];
    if( extent_[1] > 1 ){
      const int jdest = irow/nxd % nyd - nghostDest_[2];
      if( jdest < 0  ||  jdest >= extent_[1] ){
	isGhost = true;
	if( !getIxs ) return true;
      }
      if( getIxs ) (*ix)[1] = jdest;
    }

    if( extent_[2] > 1 ){
      const int kdest = irow/(nxd*nyd) - nghostDest_[4];
      if( kdest < 0  ||  kdest >= extent_[2] ){
	isGhost = true;
      }
      if( getIxs ) (*ix)[2] = kdest;
    }
    return isGhost;
  }
  //------------------------------------------------------------------
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  bool
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  is_col_ghost( const int icol,
		IndexTriplet* const ix ) const
  {
    const bool getIxs = (ix!=NULL);
    if( getIxs ){
      IndexTriplet & i= *ix;
      i[0] = -1;
      i[1] = -1;
      i[2] = -1;
    }

    bool isGhost = false;

    // is this column a ghost entry?  Column corresponds to entry in src vec.
    const int nxs = extent_[0] + nghostSrc_[0] + nghostSrc_[1];
    const int isrc = icol%nxs - nghostSrc_[0];
    if( isrc < 0  ||  isrc >= extent_[0] ){
      isGhost = true;
      if( !getIxs ) return true;
    }
    if( getIxs ) (*ix)[0] = isrc;

    const int nys = extent_[1] + nghostSrc_[2] + nghostSrc_[3];
    if( extent_[1] > 1 ){
      const int jsrc = (icol/nxs) % nys - nghostSrc_[2];
      if( jsrc < 0  ||  jsrc >= extent_[1] ){
	isGhost = true;
	if( !getIxs ) return true;
      }
      if( getIxs ) (*ix)[1] = jsrc;
    }
    if( extent_[2] > 1 ){
      const int ksrc = icol/(nxs*nys) - nghostSrc_[4];
      if( ksrc < 0  ||  ksrc >= extent_[2] )   isGhost = true;
      if( getIxs ) (*ix)[2] = ksrc;
    }
    return isGhost;
  }
  //------------------------------------------------------------------
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  template< typename T >
  bool
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  compatibility_check( const T& op, const bool isResultOp ) const
  {
    if( isResultOp ){
      if( nrows_ != op.nrows() ){
	std::cout << "Destination matrix must have same number of rows as operator." << std::endl;
	return false;
      }
    }
    else{
      if( ncols_ != op.nrows() ){
	std::cout << "Matrix dimensions are incompatible for multiplication." << std::endl;
	return false;
      }
    }
    return true;
  }
  //------------------------------------------------------------------
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  void
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  insert_row_entry( const int rownum,
		    std::vector<double> & rowValues,
		    std::vector<int> & rowIndices )
  {
    linAlg_.insert_row_values( rownum,
			       rowValues,
			       rowIndices );
  }
  //------------------------------------------------------------------
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  void
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  sum_into_row( const int rownum,
		std::vector<double> & rowValues,
		std::vector<int> & rowIndices )
  {
    linAlg_.sum_in_row_values( rownum,
			       rowValues.size(),
			       &rowValues[0],
			       &rowIndices[0] );
  }
  //------------------------------------------------------------------
  template< typename LinAlg, typename OpType, typename Direction, typename SrcFieldTraits, typename DestFieldTraits >
  void
  SpatialOperator<LinAlg,OpType,Direction,SrcFieldTraits,DestFieldTraits>::
  Print( std::ostream & s ) const
  {
    mat_.Print( s );
  }
  //------------------------------------------------------------------

  //==================================================================


  //------------------------------------------------------------------
  template< class SpatialOpType >
  SpatialOpDatabase<SpatialOpType>&
  SpatialOpDatabase<SpatialOpType>::self()
  {
    static SpatialOpDatabase<SpatialOpType> s;
    return s;
  }
//--------------------------------------------------------------------
  template< class SpatialOpType >
  void
  SpatialOpDatabase<SpatialOpType>::
  register_new_operator( SpatialOpType * const op )
  {
    Shape s( op->get_extent() );
  
    std::pair< typename ShapeOpMap::const_iterator, bool > result
      = shapeOpMap_.insert( make_pair(s,op) );

    if( !result.second ){
      std::ostringstream msg;
      msg << "ERROR!  Cannot insert duplicate operators into database." << std::endl
	  << "        nx=" << s.nxyz[0] << ", ny=" << s.nxyz[1] << ", nz=" << s.nxyz[2] << std::endl;
      throw std::runtime_error( msg.str() );
    }
  }
  //------------------------------------------------------------------
  template< class SpatialOpType >
  SpatialOpType*&
  SpatialOpDatabase<SpatialOpType>::
  retrieve_operator( const std::vector<int> & nxyz )
  {
    typename ShapeOpMap::iterator iop = shapeOpMap_.find( Shape(nxyz) );
    if( iop == shapeOpMap_.end() ){
      std::ostringstream msg;
      msg << "ERROR!  Attempted to retrieve an operator that does not exist." << std::endl
          << "        (nx,ny,nz) = (" << nxyz[0] << "," << nxyz[1] << "," << nxyz[2] << ")" << std::endl
	  << "        Check the operator shape (nx,ny,nz and nghost) and ensure that an" << std::endl
	  << "        operator with this shape has been registered in the database" << std::endl
	  << "        Operator type name: " << typeid(SpatialOpType).name() << std::endl
          << "   Registered shapes:" << std::endl;
      for( iop=shapeOpMap_.begin(); iop!=shapeOpMap_.end(); ++iop ){
	msg << "     (" << iop->first.nxyz[0] << "," << iop->first.nxyz[1] << "," << iop->first.nxyz[2] << ")" <<  std::endl;
      }
      throw std::runtime_error( msg.str() );
    }
    return iop->second;
  }
  //------------------------------------------------------------------
  template< class SpatialOpType >
  bool
  SpatialOpDatabase<SpatialOpType>::
  query_operator( const std::vector<int> & nxyz ) const
  {
    return( shapeOpMap_.find(Shape(nxyz)) != shapeOpMap_.end() );
  }
  //------------------------------------------------------------------
  template< class SpatialOpType >
  SpatialOpDatabase<SpatialOpType>::
  SpatialOpDatabase()
  {
  }
  //------------------------------------------------------------------
  template< class SpatialOpType >
  SpatialOpDatabase<SpatialOpType>::
  ~SpatialOpDatabase()
  {
    for( typename ShapeOpMap::iterator jj=shapeOpMap_.begin(); jj!=shapeOpMap_.end(); ++jj ){
      delete jj->second;
    }
  }
  //------------------------------------------------------------------

  //==================================================================

  //------------------------------------------------------------------
  template< class SpatialOpType >
  SpatialOpDatabase<SpatialOpType>::Shape::
  Shape( const std::vector<int> & extent )
    : nxyz( extent )
  {
    assert( extent.size() == 3 );
  }
  //------------------------------------------------------------------
  template< class SpatialOpType >
  bool
  SpatialOpDatabase<SpatialOpType>::Shape::
  operator<( const Shape& s ) const
  {
    if( nxyz[0] < s.nxyz[0] ) return true;
    if( nxyz[1] < s.nxyz[1] ) return true;
    if( nxyz[2] < s.nxyz[2] ) return true;
    return false;
  }
  //------------------------------------------------------------------


  //==================================================================


} // namespace SpatialOps


#endif
