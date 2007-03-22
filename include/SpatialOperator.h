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


  /**
   *  This should be specialized for each operator type to provide the
   *  type of assembler required for the operator.
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
   *  This is intended to be used as a base class only, and provides
   *  most of the functionality required.  Derived classes mainly are
   *  responsible for populating each row of the matrix.
   *
   *  Several rules apply:
   *
   *   - Application of a SpatialOperator must not involve parallel
   *   communication.  This is to ensure efficiency.
   *
   *   - 
   *   
   *
   */
  template< typename LinAlg,
	    typename OpType,
	    typename Direction,
	    typename SrcFieldTraits,
	    typename DestFieldTraits >
  class SpatialOperator
  {
  public:

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
     *  @param nrows: The number of rows in this matrix
     *
     *  @param ncols : The number of columns in this matrix
     *
     *  @param entriesPerRow : The number of nonzero entries on each
     *  row of this matrix operator.
     *
     *  @param extent : The number of points in each direction
     *  (excluding ghost cells) for the domain extent.
     */
    SpatialOperator( Assembler & opAssembler );

    virtual ~SpatialOperator();

    MatType& get_linalg_mat(){ return mat_; }
    const MatType& get_linalg_mat() const{ return mat_; }


    void apply_to_field( const SpatialField<LinAlg,SrcLocation,SrcGhost> & src,
			 SpatialField<LinAlg,DestLocation,DestGhost> & dest ) const;

    // the source field for an operator defines the number of columns,
    // the dest field defines the number of rows.
    //
    // The src operator must have the same number of rows as this
    // operator has columns.
    //
    // The dest operator must have the same number of rows as this
    // operator has, and the same number of columns as the src op.
    // This means that the dest field for dest operator is the same as
    // the dest field for this operator, and the src field for this
    // operator is the same as the src field for the src op.
    /*
    template< class FieldType1,
	      class OpAssemble1,
	      class OpAssemble2 >
    inline void apply_to_op2( const SpatialOperator< LinAlg, FieldType1,      SrcFieldTraits, OpAssemble1 > & srcOp,
			      SpatialOperator< LinAlg, DestFieldTraits, FieldType1,     OpAssemble2 > & destOp ) const;
    */

    template< class SrcOp, class DestOp >
    inline void apply_to_op( const SrcOp& src, DestOp& dest ) const;



    template< typename OT >
    inline SpatialOperator& operator=( const SpatialOperator<LinAlg,OT,Direction,SrcFieldTraits,DestFieldTraits>& );

    template< typename OT >
    inline SpatialOperator& operator+=( const SpatialOperator<LinAlg,OT,Direction,SrcFieldTraits,DestFieldTraits>& );

    template< typename OT >
    inline SpatialOperator& operator-=( const SpatialOperator<LinAlg,OT,Direction,SrcFieldTraits,DestFieldTraits>& );



    // sum the given field into the diagonal
    inline SpatialOperator& operator+=( const SpatialField<LinAlg,DestLocation,DestGhost>& f ){ linAlg_+=f; return *this; }

    // subtract the given field from the diagonal
    inline SpatialOperator& operator-=( const SpatialField<LinAlg,DestLocation,DestGhost>& f ){ linAlg_-=f; return *this; }


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


    /** reset the coefficients in the matrix */
    inline void reset_entries( const double val = 0 )
    {
      linAlg_.reset_entries( val );
    }


    /** Obtain the number of rows in this operator */
    inline int nrows() const{ return nrows_; }

    /** Obtain the number of columns in this operator */
    inline int ncols() const{ return ncols_; }


    inline const std::vector<int>& nghost_src()  const{return nghostSrc_ ;}
    inline const std::vector<int>& nghost_dest() const{return nghostDest_;}

    template< typename Dir, typename SideType >
    inline int nghost_src() const
    {
      return DestGhost::template get<Dir,SideType>();
    }

    template< typename Dir, typename SideType >
    inline int nghost_dest() const
    {
      return DestGhost::template get<Dir,SideType>();
    }

    inline const std::vector<int> & get_extent() const{ return extent_; }


    struct IndexTriplet
    {
      int index[3];
      inline int& operator[](const int i){return index[i];}
    };

    /** if provided, the IndexTriplet is populated with the interior ijk index for non-ghost entries.*/
    bool is_col_ghost( const int colnum, IndexTriplet* const ix=NULL ) const;
    bool is_row_ghost( const int rownum, IndexTriplet* const ix=NULL ) const;


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
     *
     *  @param opName : The name for this operator.  Must be a unique
     *  name.  Duplicate names will result in an exception.
     *
     *  @param makeDefault : [true] By default, registration of a new
     *  operator makes it the default operator.  If this flag is "false"
     *  then it will not replace the current default operator, unless
     *  one does not yet exist.
     */
    void register_new_operator( SpatialOpType * const op,
				const std::string& opName,
				const bool makeDefault = true );

    /**
     *  Reset the default operator to the one with the given name.
     */
    void set_default_operator( const std::string & opName,
			       const std::vector<int> & nxyz );

    /**
     *  Obtain the spatial operator with the given type and shape.
     *  Throws an exception if no match is found.
     *
     *  This pointer reference can change if the default operator for
     *  this type is changed via a call to
     *  <code>set_default_operator</code> or
     *  <code>register_new_operator</code>.
     */
    SpatialOpType*& retrieve_operator( const std::vector<int> & nxyz );

    /**
     *  Obtain the spatial operator with the given name and shape.
     *  Throws an exception if no match is found.
     *
     *  This returns a pointer reference that will never change.
     */
    SpatialOpType*& retrieve_operator( const std::string & opName,
					 const std::vector<int> & nxyz );

  private:

    SpatialOpDatabase();
    ~SpatialOpDatabase();

    SpatialOpDatabase(const SpatialOpDatabase&);
    SpatialOpDatabase&  operator=(const SpatialOpDatabase&);

    struct Shape
    {
      Shape( const std::vector<int> & extent );
      const std::vector<int> nxyz;
      bool operator ==( const Shape& s ) const;
      bool operator < ( const Shape& s ) const;
    };
    
    typedef std::map< Shape,       SpatialOpType* > ShapeOpMap;
    typedef std::map< std::string, Shape          > NameShapeMap;

    NameShapeMap nameMap_;
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
  register_new_operator( SpatialOpType * const op,
			 const std::string & name,
			 const bool makeDefault )
  {
    Shape s( op->get_extent() );

    shapeOpMap_[s] = op;

    typename NameShapeMap::iterator insm = nameMap_.find( name );
    if( insm == nameMap_.end() ){
      nameMap_.insert( make_pair(name,s) );
      //      nameMap_[name] = s;
    }
    else{
      std::ostringstream msg;
      msg << "ERROR!  Operator named '" << name << "' has already been registered!" << std::endl;
      throw std::runtime_error( msg.str() );
    }
  }
  //------------------------------------------------------------------
  template< class SpatialOpType >
  SpatialOpType*&
  SpatialOpDatabase<SpatialOpType>::
  retrieve_operator( const std::string & name,
		     const std::vector<int> & nxyz )
  {
    typename NameShapeMap::iterator ii = nameMap_.find( name );
    if( ii == nameMap_.end() ){
      std::ostringstream msg;
      msg << "ERROR!  No operator named '" << name << "'" << std::endl
	  << "        has been registered!" << std::endl;
      throw std::runtime_error( msg.str() );
    }

    typename ShapeOpMap::iterator iop = shapeOpMap_.find( ii->second );
    if( iop == shapeOpMap_.end() ){
      std::ostringstream msg;
      msg << "ERROR!  No operator named '" << name << "'" << std::endl
	  << "        with the requested dimensions and ghosting has been registered!" << std::endl;
      throw std::runtime_error( msg.str() );
    }
    return iop->second;
  }
  //------------------------------------------------------------------
  template< class SpatialOpType >
  SpatialOpType*&
  SpatialOpDatabase<SpatialOpType>::
  SpatialOpDatabase::retrieve_operator( const std::vector<int> & nxyz )
  {
    Shape s( nxyz );
    typename ShapeOpMap::iterator iop = shapeOpMap_.find( s );
    if( iop == shapeOpMap_.end() ){
      std::ostringstream msg;
      msg << "ERROR!  Attempted to retrieve an operator that does not exist." << std::endl
	  << "        Check the operator shape (nx,ny,nz and nghost) and ensure" << std::endl
	  << "        that an operator of this type and shape has been registered" << std::endl
	  << "        in the database." << std::endl;
      throw std::runtime_error( msg.str() );
    }
    return iop->second;
  }
  //------------------------------------------------------------------
  template< class SpatialOpType >
  void
  SpatialOpDatabase<SpatialOpType>::
  SpatialOpDatabase::set_default_operator( const std::string & opName,
					   const std::vector<int> & nxyz )
  {
    // do we have an operator with the given name?  If not, just return and do nothing.
    typename NameShapeMap::iterator ii = nameMap_.find( opName );
    if( ii == nameMap_.end() ) return;
    
    // See if we have an operator with this name AND shape
    Shape s( nxyz );
    const typename ShapeOpMap::iterator iopn = shapeOpMap_.find( s );
    
    if( iopn != shapeOpMap_.end() ){
      iopn->second = ii->second;
    }
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
  }
  //------------------------------------------------------------------
  template< class SpatialOpType >
  bool
  SpatialOpDatabase<SpatialOpType>::Shape::
  operator==( const Shape & s ) const
  {
    bool isequal = true;
    if( s.nxyz.size() != nxyz.size() ) return false;
    std::vector<int>::const_iterator is = s.nxyz.begin();
    std::vector<int>::const_iterator ii = nxyz.begin();
    for( ; ii!=nxyz.end(); ++ii, ++is ){
      if( *ii != *is ) isequal = false;
    }
    return isequal;
  }
  //------------------------------------------------------------------
  template< class SpatialOpType >
  bool
  SpatialOpDatabase<SpatialOpType>::Shape::
  operator < ( const Shape& s ) const
  {
    bool isLess = true;
    if( s.nxyz.size() != nxyz.size() ) return false;
    std::vector<int>::const_iterator is = s.nxyz.begin();
    std::vector<int>::const_iterator ii = nxyz.begin();
    for( ; ii!=nxyz.end(); ++ii, ++is ){
      if( *ii >= *is ) isLess = false;
    }
    return isLess;
  }
  //------------------------------------------------------------------


  //==================================================================


} // namespace SpatialOps


#endif
