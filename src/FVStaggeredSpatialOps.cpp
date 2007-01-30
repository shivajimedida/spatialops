#include <FVStaggeredSpatialOps.h>

#include <sstream>
#include <stdexcept>
#include <numeric>

using std::vector;

namespace SpatialOps{
namespace FVStaggeredUniform{

//====================================================================

//--------------------------------------------------------------------
int colcount( const vector<int> & dims,
	      const vector<int> & nghost )
{
  int nrows=1;
  vector<int>::const_iterator ig = nghost.begin();
  for( vector<int>::const_iterator ii=dims.begin(); ii!=dims.end(); ++ii ){
    if( *ii > 1 ){
      // add "left" side ghosts
      int nn = *ii;
      nn += *ig;
      ++ig;
      nn += *ig;
      nrows *= nn;
    }
  }
  return nrows;
}
//--------------------------------------------------------------------
int rowcount( const vector<int> & dims,
	      const vector<int> & nghost )
{
  return colcount(dims,nghost);
}
//--------------------------------------------------------------------


//====================================================================


//--------------------------------------------------------------------
ScratchOperator::ScratchOperator( const vector<int> & dimExtent,
				  const vector<int> & nghost,
				  const int entriesPerRow,
				  const Direction dir )
  : SpatialOperator( rowcount( dimExtent, nghost ),
		     colcount( dimExtent, nghost ),
		     nghost,
		     entriesPerRow,
		     dimExtent ),

    entriesPerRow_( entriesPerRow ),
    dir_( dir )
{
  assert( entriesPerRow==3 );

  ndim_ = 0;
  for( vector<int>::const_iterator i=extent_.begin(); i<extent_.end(); ++i ){
    if( *i > 1 ) ++ndim_;
  }

  setup_matrix();
  finalize();
}
//--------------------------------------------------------------------
ScratchOperator::~ScratchOperator()
{
}
//--------------------------------------------------------------------
void
ScratchOperator::setup_matrix()
{
  vector<double> vals( entriesPerRow_, 0.0 );
  vector<int>    ixs ( entriesPerRow_, 0   );

  for( int i=0; i<nrows(); ++i ){
    vals.clear();
    ixs.clear();
    get_row_entries ( i, vals, ixs );
    insert_row_entry( i, vals, ixs );
  }
}
//--------------------------------------------------------------------
void
ScratchOperator::get_row_entries( const int irow,
				      vector<double> & vals,
				      vector<int> & ixs ) const
{
  const double val = 0.0;

  const int nx = extent_[0] + nghost(XDIM,MINUS) + nghost(XDIM,PLUS);
  const int ny = extent_[1] + nghost(YDIM,MINUS) + nghost(YDIM,PLUS);
  const int nz = extent_[2] + nghost(ZDIM,MINUS) + nghost(ZDIM,PLUS);

  switch( dir_ ){

  case X_DIR:{
  
    const int i = irow%nx;

    if( i==0 ){
      vals.push_back( val );  ixs.push_back( irow   );
      vals.push_back( val );  ixs.push_back( irow+1 );
      vals.push_back( 0.0 );  ixs.push_back( irow+2 );
    }
    else if( i==nx-1 ){
      vals.push_back( 0.0 );  ixs.push_back( irow-2 );
      vals.push_back( val );  ixs.push_back( irow-1 );
      vals.push_back( val );  ixs.push_back( irow   );
    }
    else{
      vals.push_back( val ); ixs.push_back( irow-1 );
      vals.push_back( val ); ixs.push_back( irow   );
      vals.push_back( val ); ixs.push_back( irow+1 );
    }
    break;
  }

  case Y_DIR:{

    assert( ndim_ > 1 );

    const int j = (irow/nx)%ny;

    if( j==0 ){
      vals.push_back( val );  ixs.push_back( irow );
      vals.push_back( val );  ixs.push_back( irow+nx );
    }
    else if( j==ny-1 ){
      vals.push_back( val );  ixs.push_back( irow-nx );
      vals.push_back( val );  ixs.push_back( irow );
    }
    else{
      vals.push_back( val );  ixs.push_back( irow-nx );
      vals.push_back( val );  ixs.push_back( irow );
      vals.push_back( val );  ixs.push_back( irow+nx );
    }
    break;
  }

  case Z_DIR:{

    assert( ndim_==3 );

    const int k = irow/(nx*ny);
    if( k==0 ){
      vals.push_back( val );  ixs.push_back( irow );
      vals.push_back( val );  ixs.push_back( irow+nx*ny );
    }
    else if( k==nz-1 ){
      vals.push_back( val );  ixs.push_back( irow-nx*ny );
      vals.push_back( val );  ixs.push_back( irow );
    }
    else{
      vals.push_back( val );  ixs.push_back( irow-nx*ny );
      vals.push_back( val );  ixs.push_back( irow );
      vals.push_back( val );  ixs.push_back( irow+nx*ny );
    }

    break;
  }

  default:{
    std::ostringstream errmsg;
    errmsg << "ERROR: Invalid dimension for interpolation." << std::endl;
    throw std::runtime_error( errmsg.str() );
  }
  }
}
//--------------------------------------------------------------------


//====================================================================


//--------------------------------------------------------------------
LinearInterpolant::LinearInterpolant( const vector<int> & dimExtent,
				      const vector<int> & nghost,
				      const Direction dir )
  : SpatialOperator( rowcount( dimExtent, nghost ),
		     colcount( dimExtent, nghost ),
		     nghost,
		     entries_per_row(),
		     dimExtent ),
    dir_( dir )
{
  ndim_ = 0;
  for( vector<int>::const_iterator i=extent_.begin(); i<extent_.end(); ++i ){
    if( *i > 1 ) ++ndim_;
  }

  setup_matrix();
  finalize();
}
//--------------------------------------------------------------------
LinearInterpolant::~LinearInterpolant()
{
}
//--------------------------------------------------------------------
void
LinearInterpolant::setup_matrix()
{
  vector<double> vals( entries_per_row(), 0.0 );
  vector<int>    ixs ( entries_per_row(), 0   );

  for( int i=0; i<nrows(); ++i ){
    vals.clear();
    ixs.clear();
    get_row_entries ( i, vals, ixs );
    insert_row_entry( i, vals, ixs );
  }
}
//--------------------------------------------------------------------
void
LinearInterpolant::get_row_entries( const int irow,
				    vector<double> & vals,
				    vector<int> & ixs ) const
{
  //
  // all nonzero entries are equal to 0.5 since we are doing midpoint
  // interpolation on a uniform mesh
  //
  const double val = 0.5;

  const int nx = extent_[0] + nghost(XDIM,MINUS) + nghost(XDIM,PLUS);
  const int ny = extent_[1] + nghost(YDIM,MINUS) + nghost(YDIM,PLUS);

  switch( dir_ ){

  case X_DIR:{
  
    const int i = irow%nx;

    // for the first row (i=0 on destination field), we don't know
    // what to do to produce a meaningful value.  An appropriate
    // boundary condition could be applied here.  We could:
    //   1. use the same interpolated value as the first interior point
    //   2. Use the value specified at the first source node.
    //   3. Zero out the value.
    // Let's choose method 1 for now...

    if( i==0 ){
      vals.push_back( val );  ixs.push_back( irow   );
      vals.push_back( val );  ixs.push_back( irow+1 );
    }
    else{
      vals.push_back( val );  ixs.push_back( irow-1 );
      vals.push_back( val );  ixs.push_back( irow   );
    }
    break;
  }

  case Y_DIR:{

    assert( ndim_ > 1 );

    const int j = (irow/nx)%ny;

    if( j==0 ){
      vals.push_back( val );  ixs.push_back( irow );
      vals.push_back( val );  ixs.push_back( irow+nx );
    }
    else{
      vals.push_back( val );  ixs.push_back( irow-nx );
      vals.push_back( val );  ixs.push_back( irow );
    }
    break;
  }

  case Z_DIR:{
    assert( ndim_==3 );

    const int k = irow/(nx*ny);
    if( k==0 ){
      vals.push_back( val );  ixs.push_back( irow );
      vals.push_back( val );  ixs.push_back( irow+nx*ny );
    }
    else{
      vals.push_back( val );  ixs.push_back( irow-nx*ny );
      vals.push_back( val );  ixs.push_back( irow );
    }

    break;
  }

  default:{
    std::ostringstream errmsg;
    errmsg << "ERROR: Invalid dimension for interpolation." << std::endl;
    throw std::runtime_error( errmsg.str() );
  }
  }
}
//--------------------------------------------------------------------


//====================================================================


//--------------------------------------------------------------------
Gradient2ndOrder::Gradient2ndOrder( const vector<double> & meshSpacing,
				    const vector<int> & dimExtent,
				    const vector<int> & nghost,
				    const Direction dir )
  : SpatialOperator( rowcount( dimExtent, nghost ),
		     colcount( dimExtent, nghost ),
		     nghost,
		     entries_per_row(),
		     dimExtent ),

    spacing_( meshSpacing ),
    dir_( dir )
{
  ndim_ = 0;
  for( vector<int>::const_iterator i=extent_.begin(); i<extent_.end(); ++i ){
    if( *i > 1 ) ++ndim_;
  }

  setup_matrix();
  finalize();
}
//--------------------------------------------------------------------
Gradient2ndOrder::~Gradient2ndOrder()
{
}
//--------------------------------------------------------------------
void
Gradient2ndOrder::setup_matrix()
{
  vector<double> vals( entries_per_row(), 0.0 );
  vector<int>    ixs ( entries_per_row(), 0   );

  for( int i=0; i<nrows(); ++i ){
    vals.clear();
    ixs.clear();
    get_row_entries ( i, vals, ixs );
    insert_row_entry( i, vals, ixs );
  }
}
//--------------------------------------------------------------------
void
Gradient2ndOrder::get_row_entries( const int irow,
				       vector<double> & vals,
				       vector<int> & ixs ) const
{
  const int nx  = extent_[0] + nghost(XDIM,MINUS) + nghost(XDIM,PLUS);
  const int ny  = extent_[1] + nghost(YDIM,MINUS) + nghost(YDIM,PLUS);

  switch( dir_ ){

  case X_DIR:{

    const double fac = 1.0/spacing_[0];
    const int i = irow%nx;

    if( i==0 ){
      vals.push_back( 0.0 );  ixs.push_back( irow   );
      vals.push_back( 0.0 );  ixs.push_back( irow+1 );
//       vals.push_back( -fac );  ixs.push_back( irow   );
//       vals.push_back(  fac );  ixs.push_back( irow+1 );
    }
    else{
      vals.push_back( -fac );  ixs.push_back( irow-1 );
      vals.push_back(  fac );  ixs.push_back( irow   );
    }
    break;
  }

  case Y_DIR:{

    assert( ndim_ > 1 );

    const int j = (irow/nx)%ny;
    const double fac = 1.0/spacing_[1];

    if( j==0 ){
      vals.push_back( 0.0 );  ixs.push_back( irow    );
      vals.push_back( 0.0 );  ixs.push_back( irow+nx );
//       vals.push_back( -fac );  ixs.push_back( irow    );
//       vals.push_back(  fac );  ixs.push_back( irow+nx );
    }
    else{
      vals.push_back( -fac );  ixs.push_back( irow-nx );
      vals.push_back(  fac );  ixs.push_back( irow    );
    }
    break;
  }

  case Z_DIR:{
    assert( ndim_ == 3 );

    const double fac = 1.0/spacing_[2];
    const int k = irow/(nx*ny);

    if( k==0 ){
      vals.push_back( 0.0 );  ixs.push_back( irow );
      vals.push_back( 0.0 );  ixs.push_back( irow+nx*ny );
//       vals.push_back( -fac );  ixs.push_back( irow );
//       vals.push_back(  fac );  ixs.push_back( irow+nx*ny );
    }
    else{
      vals.push_back( -fac );  ixs.push_back( irow-nx*ny );
      vals.push_back(  fac );  ixs.push_back( irow );
    }
    break;
  }

  default:{
    std::ostringstream errmsg;
    errmsg << "ERROR: Incompatible dimension for gradient operator." << std::endl;
    throw std::runtime_error( errmsg.str() );
  }
  } // end switch
}
//--------------------------------------------------------------------


//====================================================================


//--------------------------------------------------------------------
Divergence2ndOrder::Divergence2ndOrder( const vector<double> & cellFaceArea,
					const double cellVolume,
					const vector<int> & dimExtent,
					const vector<int> & nghost,
					const Direction dir )
  : SpatialOperator( rowcount( dimExtent, nghost ),
		     colcount( dimExtent, nghost ),
		     nghost,
		     entries_per_row(), 
		     dimExtent ),

    faceArea_( cellFaceArea ),
    cellVol_( cellVolume ),
    dir_( dir )
{
  ndim_ = 0;
  for( vector<int>::const_iterator i=extent_.begin(); i<extent_.end(); ++i ){
    if( *i > 1 ) ++ndim_;
  }

  setup_matrix();
  finalize();
}
//--------------------------------------------------------------------
Divergence2ndOrder::~Divergence2ndOrder()
{
}
//--------------------------------------------------------------------
void
Divergence2ndOrder::setup_matrix()
{
  vector<double> vals( entries_per_row(), 0.0 );
  vector<int>    ixs ( entries_per_row(), 0   );

  for( int i=0; i<nrows(); ++i ){
    vals.clear();
    ixs.clear();
    get_row_entries ( i, vals, ixs );
    insert_row_entry( i, vals, ixs );
  }
}
//--------------------------------------------------------------------
void
Divergence2ndOrder::get_row_entries( const int irow,
				     vector<double> & vals,
				     vector<int> & ixs ) const
{
  switch( dir_ ){

  case X_DIR:{

    const int nx = extent_[0] + nghost(XDIM,MINUS) + nghost(XDIM,PLUS);

    const double fac = faceArea_[0]/cellVol_;

    const int i = irow%nx;
    if( i==nx-1 ){
      vals.push_back( 0.0 );  ixs.push_back( irow-1 );
      vals.push_back( 0.0 );  ixs.push_back( irow   );
//       vals.push_back( -fac );  ixs.push_back( irow-1 );
//       vals.push_back(  fac );  ixs.push_back( irow   );
    }
    else{
      vals.push_back( -fac );  ixs.push_back( irow   );
      vals.push_back(  fac );  ixs.push_back( irow+1 );
    }
    break;
  }

  case Y_DIR:{

    assert( ndim_ > 1 );
    
    const int nx = extent_[0] + nghost(XDIM,MINUS) + nghost(XDIM,PLUS);
    const int ny = extent_[1] + nghost(YDIM,MINUS) + nghost(YDIM,PLUS);

    const double fac = faceArea_[1]/cellVol_;
    const int j = (irow/nx)%ny;

    if( j==ny-1 ){
      vals.push_back( 0.0 );  ixs.push_back( irow-nx );
      vals.push_back( 0.0 );  ixs.push_back( irow    );
//       vals.push_back( -fac );  ixs.push_back( irow-nx );
//       vals.push_back(  fac );  ixs.push_back( irow    );
    }
    else{
      vals.push_back( -fac );  ixs.push_back( irow );
      vals.push_back(  fac );  ixs.push_back( irow+nx );
    }
    break;
  }

  case Z_DIR:{
    assert( ndim_ == 3 );

    const int nx = extent_[0] + nghost(XDIM,MINUS) + nghost(XDIM,PLUS);
    const int ny = extent_[1] + nghost(YDIM,MINUS) + nghost(YDIM,PLUS);
    const int nz = extent_[2] + nghost(ZDIM,MINUS) + nghost(ZDIM,PLUS);

    const double fac = faceArea_[2]/cellVol_;
    const int k = irow/(nx*ny);
    if( k==nz-1 ){
      vals.push_back( 0.0 );  ixs.push_back( irow-nx*ny );
      vals.push_back( 0.0 );  ixs.push_back( irow       );
//       vals.push_back( -fac );  ixs.push_back( irow-nx*ny );
//       vals.push_back(  fac );  ixs.push_back( irow       );
    }
    else{
      vals.push_back( -fac );  ixs.push_back( irow );
      vals.push_back(  fac );  ixs.push_back( irow+nx*ny );
    }
    break;
  }

  default:{
    std::ostringstream errmsg;
    errmsg << "ERROR: Incompatible dimension for gradient operator." << std::endl;
    throw std::runtime_error( errmsg.str() );
  }
  } // end switch

}
//--------------------------------------------------------------------


//====================================================================


} // namespace FVStaggeredUniform
} // namespace SpatialOps
