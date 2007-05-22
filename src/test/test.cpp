#include <vector>

#include <FV2ndOrderTypes.h>

#include <EpetraExt_RowMatrixOut.h>
#include <EpetraExt_VectorOut.h>

using namespace std;

#define PI 3.14159265358979

void setup_geom( const std::vector<int> & dim,
		 std::vector<double> & spacing,
		 std::vector<double> & area,
		 double & volume )
{
  for( int i=0; i<3; ++i )
    spacing[i] = (dim[i]==1) ? 1.0 : PI/(dim[i]-1);

  area[0] = spacing[1]*spacing[2];
  area[1] = spacing[0]*spacing[2];
  area[2] = spacing[0]*spacing[1];

  volume = spacing[0]*spacing[1]*spacing[2];
}

//--------------------------------------------------------------------

void build_ops( const std::vector<int> & dim )
{
  using namespace SpatialOps;
  using namespace FVStaggeredUniform;

  std::vector<double> spacing(3), area(3);
  double volume;
  setup_geom( dim, spacing, area, volume );

  // build the assemblers
  InterpXC2F::Assembler rxcfasmbl( dim );
  InterpYC2F::Assembler rycfasmbl( dim );
  InterpZC2F::Assembler rzcfasmbl( dim );

  InterpXF2C::Assembler rxfcasmbl( dim );
  InterpYF2C::Assembler ryfcasmbl( dim );
  InterpZF2C::Assembler rzfcasmbl( dim );

  GradXC2F::Assembler gxcfasmbl( spacing, dim );
  GradYC2F::Assembler gycfasmbl( spacing, dim );
  GradZC2F::Assembler gzcfasmbl( spacing, dim );

  GradXF2C::Assembler gxfcasmbl( spacing, dim );
  GradYF2C::Assembler gyfcasmbl( spacing, dim );
  GradZF2C::Assembler gzfcasmbl( spacing, dim );

  DivXF2C::Assembler dxfcasmbl( dim, area, volume );
  DivYF2C::Assembler dyfcasmbl( dim, area, volume );
  DivZF2C::Assembler dzfcasmbl( dim, area, volume );

  SxCell::Assembler  sxcasmbl( dim );
  SyCell::Assembler  sycasmbl( dim );
  SzCell::Assembler  szcasmbl( dim );

  SxCellSide::Assembler  sxcfasmbl( dim );
  SyCellSide::Assembler  sycfasmbl( dim );
  SzCellSide::Assembler  szcfasmbl( dim );


  // build the operators
  InterpXC2F *rxcf = new InterpXC2F( rxcfasmbl );
  InterpYC2F *rycf = dim[1]==1 ? NULL : new InterpYC2F( rycfasmbl );
  InterpZC2F *rzcf = dim[2]==1 ? NULL : new InterpZC2F( rzcfasmbl );

  InterpXF2C *rxfc = new InterpXF2C( rxfcasmbl );
  InterpYF2C *ryfc = dim[1]==1 ? NULL : new InterpYF2C( ryfcasmbl );
  InterpZF2C *rzfc = dim[2]==1 ? NULL : new InterpZF2C( rzfcasmbl );

  GradXC2F   *gxcf = new GradXC2F( gxcfasmbl );
  GradYC2F   *gycf = dim[1]==1 ? NULL : new GradYC2F( gycfasmbl );
  GradZC2F   *gzcf = dim[2]==1 ? NULL : new GradZC2F( gzcfasmbl );

  GradXF2C   *gxfc = new GradXF2C( gxfcasmbl );
  GradYF2C   *gyfc = dim[1]==1 ? NULL : new GradYF2C( gyfcasmbl );
  GradZF2C   *gzfc = dim[2]==1 ? NULL : new GradZF2C( gzfcasmbl );

  DivXF2C    *dxfc = new DivXF2C( dxfcasmbl );
  DivYF2C    *dyfc = dim[1]==1 ? NULL : new DivYF2C( dyfcasmbl );
  DivZF2C    *dzfc = dim[2]==1 ? NULL : new DivZF2C( dzfcasmbl );

  SxCellSide *sxcf = new SxCellSide( sxcfasmbl );
  SyCellSide *sycf = dim[1]==1 ? NULL : new SyCellSide( sycfasmbl );
  SzCellSide *szcf = dim[2]==1 ? NULL : new SzCellSide( szcfasmbl );

  SxCell     *sxc  = new SxCell( sxcasmbl  );
  SyCell     *syc  = dim[1]==1 ? NULL : new SyCell( sycasmbl  );
  SzCell     *szc  = dim[2]==1 ? NULL : new SzCell( szcasmbl  );

  SpatialOpDatabase<InterpXC2F>::self().register_new_operator( rxcf );
  if( dim[1]>1 ) SpatialOpDatabase<InterpYC2F>::self().register_new_operator( rycf );
  if( dim[2]>1 ) SpatialOpDatabase<InterpZC2F>::self().register_new_operator( rzcf );

  SpatialOpDatabase<InterpXF2C>::self().register_new_operator( rxfc );
  if( dim[1]>1 ) SpatialOpDatabase<InterpYF2C>::self().register_new_operator( ryfc );
  if( dim[2]>1 ) SpatialOpDatabase<InterpZF2C>::self().register_new_operator( rzfc );

  SpatialOpDatabase<GradXC2F  >::self().register_new_operator( gxcf );
  if( dim[1]>1 ) SpatialOpDatabase<GradYC2F  >::self().register_new_operator( gycf );
  if( dim[2]>1 ) SpatialOpDatabase<GradZC2F  >::self().register_new_operator( gzcf );

  SpatialOpDatabase<GradXF2C  >::self().register_new_operator( gxfc );
  if( dim[1]>1 ) SpatialOpDatabase<GradYF2C  >::self().register_new_operator( gyfc );
  if( dim[2]>1 ) SpatialOpDatabase<GradZF2C  >::self().register_new_operator( gzfc );

  SpatialOpDatabase<DivXF2C   >::self().register_new_operator( dxfc );
  if( dim[1]>1 ) SpatialOpDatabase<DivYF2C   >::self().register_new_operator( dyfc );
  if( dim[2]>1 ) SpatialOpDatabase<DivZF2C   >::self().register_new_operator( dzfc );

  SpatialOpDatabase<SxCellSide>::self().register_new_operator( sxcf );
  if( dim[1]>1 ) SpatialOpDatabase<SyCellSide>::self().register_new_operator( sycf );
  if( dim[2]>1 ) SpatialOpDatabase<SzCellSide>::self().register_new_operator( szcf );

  SpatialOpDatabase<SxCell    >::self().register_new_operator( sxc );
  if( dim[1]>1 ) SpatialOpDatabase<SyCell    >::self().register_new_operator( syc );
  if( dim[2]>1 ) SpatialOpDatabase<SzCell    >::self().register_new_operator( szc );
}



bool test( const std::vector<int> & dim )
{
  using namespace SpatialOps;
  using namespace FVStaggeredUniform;

  std::vector<double> spacing(3), area(3);
  double volume;
  setup_geom( dim, spacing, area, volume );

  // get the operators
  InterpXC2F & rxcf = *SpatialOpDatabase<InterpXC2F>::self().retrieve_operator( dim );
  InterpXF2C & rxfc = *SpatialOpDatabase<InterpXF2C>::self().retrieve_operator( dim );
  GradXC2F   & gxcf = *SpatialOpDatabase<GradXC2F  >::self().retrieve_operator( dim );
  DivXF2C    & dxfc = *SpatialOpDatabase<DivXF2C   >::self().retrieve_operator( dim );
  SxCellSide & sxcf = *SpatialOpDatabase<SxCellSide>::self().retrieve_operator( dim );
  SxCell     & sxc  = *SpatialOpDatabase<SxCell    >::self().retrieve_operator( dim );

  EpetraExt::RowMatrixToMatrixMarketFile( "Dx.mm", dxfc.get_linalg_mat(), "", "" );
  EpetraExt::RowMatrixToMatrixMarketFile( "Gx.mm", gxcf.get_linalg_mat(), "", "" );
  EpetraExt::RowMatrixToMatrixMarketFile( "Rx.mm", rxcf.get_linalg_mat(), "", "" );
  EpetraExt::RowMatrixToMatrixMarketFile( "Rxfc.mm", rxfc.get_linalg_mat(), "", "" );
  EpetraExt::RowMatrixToMatrixMarketFile( "Sx.mm", sxc .get_linalg_mat(), "", "" );


  // build the spatial fields
  CellField      x( dim, NULL, InternalStorage );
  CellField      f( dim, NULL, InternalStorage );
  XSideField  xint( dim, NULL, InternalStorage );
  XSideField  fint( dim, NULL, InternalStorage );
  XSideField gradF( dim, NULL, InternalStorage );
  CellField    d2f( dim, NULL, InternalStorage );
  CellField  fint2( dim, NULL, InternalStorage );


  const double dx = spacing[0]; //1.0/double(dim[0]-1);
  const int ngxlo = CellFieldTraits::GhostTraits::get<XDIR,SideMinus>();
  const int khi = dim[2]==1 ? 1 : dim[2]+CellFieldTraits::GhostTraits::get<ZDIR,SideMinus>() + CellFieldTraits::GhostTraits::get<ZDIR,SidePlus>();
  const int jhi = dim[1]==1 ? 1 : dim[1]+CellFieldTraits::GhostTraits::get<YDIR,SideMinus>() + CellFieldTraits::GhostTraits::get<YDIR,SidePlus>();
  const int ihi = dim[0]+CellFieldTraits::GhostTraits::get<XDIR,SideMinus>() + CellFieldTraits::GhostTraits::get<XDIR,SidePlus>();
  const int ilo=0, jlo=0, klo=0;
  int ix = 0;
  for( int k=klo; k<khi ; ++k ){
    for( int j=jlo; j<jhi; ++j ){
      for( int i=ilo; i<ihi; ++i ){
	x[ix] = (i-ngxlo)*dx;
	f[ix] = std::sin(x[ix]) + 2.0;
	++ix;
      }
    }
  }

  // apply the operators to the fields
  gxcf.apply_to_field( f, gradF );
  rxcf.apply_to_field( x, xint  );
  rxcf.apply_to_field( f, fint  );

  rxfc.apply_to_field( fint, fint2 );

  dxfc.apply_to_op( gxcf, sxc );
  EpetraExt::RowMatrixToMatrixMarketFile( "Lx.mm", sxc.get_linalg_mat(), "", "" );

  sxc.apply_to_field( f, d2f );

  //gxfc.apply_to_field( gradF, d2f );


  sxcf  = gxcf;
  sxcf += gxcf;
  sxcf -= gxcf;
  sxcf += fint;


  /*
  EpetraExt::VectorToMatrixMarketFile( "x.mm",   x.get_linalg_vec(), "", "" );
  EpetraExt::VectorToMatrixMarketFile( "fx.mm",   f.get_linalg_vec(), "", "" );
  EpetraExt::VectorToMatrixMarketFile( "xx.mm", xint.get_linalg_vec(), "", "" );
  EpetraExt::VectorToMatrixMarketFile( "fintx.mm", fint.get_linalg_vec(), "", "" );
  EpetraExt::VectorToMatrixMarketFile( "dfdx.mm", gradF.get_linalg_vec(), "", "" );
  EpetraExt::VectorToMatrixMarketFile( "d2fdx2.mm", d2f.get_linalg_vec(), "", "" );
  */

  // build a field without any ghost information in it.
  CellFieldNoGhost tmpField( dim, NULL, InternalStorage );
  RHS tmp(dim);
  tmp.reset();
  tmp.add_field_contribution( x );
  tmpField = tmp;

  // this is really a hack, but it is a way to eliminate ghost values from the output...
  tmp.reset(); tmp.add_field_contribution(x    ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "x.mm", tmpField.get_linalg_vec(), "", "" );
  tmp.reset(); tmp.add_field_contribution(f    ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "fx.mm", tmpField.get_linalg_vec(), "", "" );
  tmp.reset(); tmp.add_field_contribution(xint ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "xx.mm", tmpField.get_linalg_vec(), "", "" );
  tmp.reset(); tmp.add_field_contribution(fint ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "fintx.mm", tmpField.get_linalg_vec(), "", "" );
  tmp.reset(); tmp.add_field_contribution(gradF); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "dfdx.mm", tmpField.get_linalg_vec(), "", "" );
  tmp.reset(); tmp.add_field_contribution(d2f  ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "d2fdx2.mm", tmpField.get_linalg_vec(), "", "" );
  tmp.reset(); tmp.add_field_contribution(fint2); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "fintx2.mm", tmpField.get_linalg_vec(), "", "" );

  tmpField += tmp;  // test += operator
  tmpField -= tmp;  // test -= operator
  CellFieldNoGhost tmp2( dim,NULL,InternalStorage );
  tmp2 = tmp;
  cout << "SpatialField operators -=, +=, ==, = ... ";
  if( tmp2!=tmpField ) cout << "FAIL!" << std::endl;
  else cout << "PASS" << std::endl;

  if( dim[1] > 1 ){

    // get the operators
    InterpYC2F & rycf = *SpatialOpDatabase<InterpYC2F>::self().retrieve_operator( dim );
    InterpYF2C & ryfc = *SpatialOpDatabase<InterpYF2C>::self().retrieve_operator( dim );
    GradYC2F   & gycf = *SpatialOpDatabase<GradYC2F  >::self().retrieve_operator( dim );
    DivYF2C    & dyfc = *SpatialOpDatabase<DivYF2C   >::self().retrieve_operator( dim );
    SyCell     & syc  = *SpatialOpDatabase<SyCell    >::self().retrieve_operator( dim );

    EpetraExt::RowMatrixToMatrixMarketFile( "Dy.mm", dyfc.get_linalg_mat(), "", "" );
    EpetraExt::RowMatrixToMatrixMarketFile( "Gy.mm", gycf.get_linalg_mat(), "", "" );
    EpetraExt::RowMatrixToMatrixMarketFile( "Ry.mm", rycf.get_linalg_mat(), "", "" );
    EpetraExt::RowMatrixToMatrixMarketFile( "Ryfc.mm", ryfc.get_linalg_mat(), "", "" );
    EpetraExt::RowMatrixToMatrixMarketFile( "Sy.mm", syc.get_linalg_mat(), "", "" );


    CellField     y( dim, NULL, InternalStorage );
    YSideField yint( dim, NULL, InternalStorage );
    YSideField   fy( dim, NULL, InternalStorage );
    YSideField dfdy( dim, NULL, InternalStorage );
    CellField   fy2( dim, NULL, InternalStorage );

    const double dy = spacing[1];
    const int ngylo = CellFieldTraits::GhostTraits::get<YDIR,SideMinus>();
    int ix=0;
    for( int k=klo; k<khi ; ++k ){
      for( int j=jlo; j<jhi; ++j ){
	for( int i=ilo; i<ihi; ++i ){
	  y[ix] = (j-ngylo)*dy;
	  f[ix] = std::sin(y[ix]) + 2.0;
	  ++ix;
	}
      }
    }

    rycf.apply_to_field( f, fy );
    rycf.apply_to_field( y, yint );
    ryfc.apply_to_field( fy, fy2 );
    gycf.apply_to_field( f, dfdy );
    dyfc.apply_to_field( dfdy, d2f );

    dyfc.apply_to_op( gycf, syc );

    syc.apply_to_field( f, d2f );
    
    EpetraExt::RowMatrixToMatrixMarketFile( "Ly.mm", syc.get_linalg_mat(), "", "" );

   // this is really a hack, but it is a way to eliminate ghost values from the output...
    tmp.reset(); tmp.add_field_contribution(y    ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "y.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(f    ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "fy.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(yint ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "yy.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(fy   ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "finty.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(fy2  ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "finty2.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(dfdy ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "dfdy.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(d2f  ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "d2fdy2.mm", tmpField.get_linalg_vec(), "", "" );
 
  }


  if( dim[2] > 1 ){

    // get the operators
    InterpZC2F & rzcf = *SpatialOpDatabase<InterpZC2F>::self().retrieve_operator( dim );
    InterpZF2C & rzfc = *SpatialOpDatabase<InterpZF2C>::self().retrieve_operator( dim );
    GradZC2F   & gzcf = *SpatialOpDatabase<GradZC2F  >::self().retrieve_operator( dim );
    DivZF2C    & dzfc = *SpatialOpDatabase<DivZF2C   >::self().retrieve_operator( dim );
    SzCell     & szc  = *SpatialOpDatabase<SzCell    >::self().retrieve_operator( dim );

    EpetraExt::RowMatrixToMatrixMarketFile( "Dz.mm", dzfc.get_linalg_mat(), "", "" );
    EpetraExt::RowMatrixToMatrixMarketFile( "Gz.mm", gzcf.get_linalg_mat(), "", "" );
    EpetraExt::RowMatrixToMatrixMarketFile( "Rz.mm", rzcf.get_linalg_mat(), "", "" );
    EpetraExt::RowMatrixToMatrixMarketFile( "Sz.mm", szc .get_linalg_mat(), "", "" );


    CellField     z( dim, NULL, InternalStorage );
    ZSideField zint( dim, NULL, InternalStorage );
    ZSideField   fz( dim, NULL, InternalStorage );
    ZSideField dfdz( dim, NULL, InternalStorage );
    CellField   fz2( dim, NULL, InternalStorage );
    CellField  d2fa( dim, NULL, InternalStorage );

    const double dz = spacing[2];
    const int ngzlo = CellFieldTraits::GhostTraits::get<ZDIR,SideMinus>();
    int ix=0;
    for( int k=klo; k<khi ; ++k ){
      for( int j=jlo; j<jhi; ++j ){
	for( int i=ilo; i<ihi; ++i ){
	  z[ix] = (k-ngzlo)*dz;
	  f[ix] = std::sin(z[ix]) + 2.0;
	  ++ix;
	}
      }
    }

    rzcf.apply_to_field( f, fz );
    rzfc.apply_to_field( fz, fz2 );
    rzcf.apply_to_field( z, zint );
    gzcf.apply_to_field( f, dfdz );
    dzfc.apply_to_field( dfdz, d2fa );

    dzfc.apply_to_op( gzcf, szc );

    szc.apply_to_field( f, d2f );

    cout << "test on grad/div consistency ... ";
    ix=0;
    bool trouble = false;
    for( int k=klo; k<khi ; ++k ){
      for( int j=jlo; j<jhi; ++j ){
	for( int i=ilo; i<ihi; ++i ){
	  const double err = std::abs( d2fa[ix] - d2f[ix] );
	  if( err > 1.0e-11 ){
	    std::cout << "("<<i<<","<<j<<","<<k<<") err: " << err << std::endl;
	    trouble = true;
	  }
	  assert( err < 1.0e-10 );
	  ++ix;
	}
      }
    }
    if( trouble ) cout << "FAIL" << std::endl;
    else cout << "PASS" << std::endl;
    
    EpetraExt::RowMatrixToMatrixMarketFile( "Lz.mm", szc.get_linalg_mat(), "", "" );

   // this is really a hack, but it is a way to eliminate ghost values from the output...
    tmp.reset(); tmp.add_field_contribution(z    ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "z.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(f    ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "fz.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(zint ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "zz.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(fz   ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "fintz.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(fz2  ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "fintz2.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(dfdz ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "dfdz.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(d2f  ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "d2fdz2.mm", tmpField.get_linalg_vec(), "", "" );
    tmp.reset(); tmp.add_field_contribution(d2fa ); tmpField=tmp;  EpetraExt::VectorToMatrixMarketFile( "d2fdz2a.mm", tmpField.get_linalg_vec(), "", "" );
 
  } // z-dir

  return true;
}

//====================================================================

bool test_linsys( const std::vector<int> & dim )
{
  using namespace SpatialOps;
  using namespace FVStaggeredUniform;

  std::vector<double> spacing(3), area(3);
  double volume;
  setup_geom( dim, spacing, area, volume );

  LinearSystem & linSys = LinSysFactory::self().get_linsys( LinSysInfo(dim) );
  LHS & A = linSys.get_lhs();
  A.reset();


  GradXC2F   & gradX = *SpatialOpDatabase<GradXC2F  >::self().retrieve_operator( dim );
  DivXF2C    & divX = *SpatialOpDatabase<DivXF2C   >::self().retrieve_operator( dim );
  SxCell     & sxc  = *SpatialOpDatabase<SxCell    >::self().retrieve_operator( dim );
  divX.apply_to_op( gradX, sxc );
  A.add_op_contribution( sxc );

  if( dim[1]>1 ){
    GradYC2F   & gradY = *SpatialOpDatabase<GradYC2F  >::self().retrieve_operator( dim );
    DivYF2C    & divY = *SpatialOpDatabase<DivYF2C   >::self().retrieve_operator( dim );
    SyCell     & syc  = *SpatialOpDatabase<SyCell    >::self().retrieve_operator( dim );
    divY.apply_to_op( gradY, syc );
    A.add_op_contribution( syc );
  }
  if( dim[2]>1 ){
    GradZC2F   & gradZ = *SpatialOpDatabase<GradZC2F  >::self().retrieve_operator( dim );
    DivZF2C    & divZ = *SpatialOpDatabase<DivZF2C   >::self().retrieve_operator( dim );
    SzCell     & szc  = *SpatialOpDatabase<SzCell    >::self().retrieve_operator( dim );
    divZ.apply_to_op( gradZ, szc );
    A.add_op_contribution( szc );
  }

  CellField d(dim,NULL,InternalStorage);
  d = 1.0;
  A.add_field_contribution( d );

  RHS & b = linSys.get_rhs();
  b.reset( 1.0 );

  linSys.solve();

  CellFieldNoGhost tmpField( dim, NULL, InternalStorage );
  tmpField = linSys.get_soln_field();

  EpetraExt::VectorToMatrixMarketFile( "soln.mm", tmpField.get_linalg_vec(), "", "" );

  tmpField = b;
  EpetraExt::VectorToMatrixMarketFile( "rhs.mm", tmpField.get_linalg_vec(), "", "" );

  EpetraExt::RowMatrixToMatrixMarketFile( "A.mm", A.epetra_mat(), "", "" );

  return true;
}

//====================================================================

void test_daixt( vector<int>& dim )
{
  using namespace SpatialOps;
  using namespace FVStaggeredUniform;

  cout << "SpatialField operators +, -, /, *, =  ... ";

  vector<double> d1, d2;
  const int n = dim[0]*dim[1]*dim[2];
  for( int i=0; i<n; ++i){
    d1.push_back(i);
    d2.push_back(-i+1.234);
  }

  CellFieldNoGhost f1( dim, &d1[0], ExternalStorage );
  CellFieldNoGhost f2( dim, &d2[0], ExternalStorage );
  CellFieldNoGhost f3( dim, NULL,   InternalStorage );

  f3 = f1+f2;
  for( int i=0; i<n; ++i ){
    if( std::abs(f3[i] - 1.234)>1.0e-10 ){
      cout << "FAIL" << endl;
    }
  }

  f3 = f3-(f1+f2);
  for( int i=0; i<n; ++i ){
    if( std::abs(f3[i])>1.0e-10 ){
      cout << "FAIL" << endl;
    }
  }


  f3 = (f1+f2)*f1;
  for( int i=0; i<n; ++i ){
    if( std::abs(f3[i] - f1[i]*1.234)>1.0e-10 ){
      cout << "FAIL" << endl;
    }
  }


  f3 = f1*f2+f1/f2*f3+f2/f1+f2*f1*f2;  // this ends up using two temporaries.

  // THIS SHOULD NEVER BE DONE, because it is very volatile, as shown below.
  CellFieldNoGhost& f4 = f1+f2;  // f4 is computed from a temporary.
  f3 = f1-f2;       // the same temporary is used here, and now f4 is dead.

  cout << "PASS" << endl;
}

//====================================================================

int main()
{
  vector<int> dim(3,1);
  dim[0]= 14;
  dim[1]=  1;
  dim[2]=  1;

  test_daixt( dim );

  build_ops( dim );

  test( dim );
  test_linsys( dim );

  return 0;
}
