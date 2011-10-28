#include "StencilBuilder.h"
#include "FVStaggeredOperatorTypes.h"

#include <spatialops/structured/FVStaggeredFieldTypes.h>
#include <spatialops/OperatorDatabase.h>

namespace SpatialOps{
namespace structured{

#define REG_BASIC_OP_TYPES( VOL )                                       \
  {                                                                     \
    typedef BasicOpTypes<VOL>  OpTypes;                                 \
    opdb.register_new_operator( new OpTypes::InterpC2FX( 0.5, 0.5 ) );  \
    opdb.register_new_operator( new OpTypes::InterpC2FY( 0.5, 0.5 ) );  \
    opdb.register_new_operator( new OpTypes::InterpC2FZ( 0.5, 0.5 ) );  \
    opdb.register_new_operator( new OpTypes::GradX( -1.0/dx, 1.0/dx ) );        \
    opdb.register_new_operator( new OpTypes::GradY( -1.0/dy, 1.0/dy ) );        \
    opdb.register_new_operator( new OpTypes::GradZ( -1.0/dz, 1.0/dz ) );        \
    opdb.register_new_operator( new OpTypes::DivX ( -1.0/dx, 1.0/dx ) );        \
    opdb.register_new_operator( new OpTypes::DivY ( -1.0/dy, 1.0/dy ) );        \
    opdb.register_new_operator( new OpTypes::DivZ ( -1.0/dz, 1.0/dz ) );        \
  }

  //------------------------------------------------------------------

  void build_stencils( const unsigned int nx,
                       const unsigned int ny,
                       const unsigned int nz,
                       const double Lx,
                       const double Ly,
                       const double Lz,
                       OperatorDatabase& opdb )
  {
    const double dx = Lx/nx;
    const double dy = Ly/ny;
    const double dz = Lz/nz;

    //___________________________________________________________________
    // stencil2:
    //
    REG_BASIC_OP_TYPES( SVolField )  // basic operator types on a scalar volume
    REG_BASIC_OP_TYPES( XVolField )  // basic operator types on a x volume
    REG_BASIC_OP_TYPES( YVolField )  // basic operator types on a y volume
    REG_BASIC_OP_TYPES( ZVolField )  // basic operator types on a z volume

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,XVolField,YSurfXField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient,   XVolField,YSurfXField>::type( -1.0/dy, 1.0/dy ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,XVolField,ZSurfXField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient,   XVolField,ZSurfXField>::type( -1.0/dz, 1.0/dz ) );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,YVolField,XSurfYField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient,   YVolField,XSurfYField>::type( -1.0/dx, 1.0/dx ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,YVolField,ZSurfYField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient,   YVolField,ZSurfYField>::type( -1.0/dz, 1.0/dz ) );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,ZVolField,XSurfZField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient   ,ZVolField,XSurfZField>::type( -1.0/dx, 1.0/dx ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,ZVolField,YSurfZField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient   ,ZVolField,YSurfZField>::type( -1.0/dy, 1.0/dy ) );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,  XVolField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient   ,SVolField,  XVolField>::type( -1.0/dx, 1.0/dx ) );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,  YVolField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient   ,SVolField,  YVolField>::type( -1.0/dy, 1.0/dy ) );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,  ZVolField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient   ,SVolField,  ZVolField>::type( -1.0/dz, 1.0/dz ) );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,XVolField,  SVolField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient   ,XVolField,  SVolField>::type( -1.0/dx, 1.0/dx ) );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,YVolField,  SVolField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient   ,YVolField,  SVolField>::type( -1.0/dy, 1.0/dy ) );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,ZVolField,  SVolField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Gradient   ,ZVolField,  SVolField>::type( -1.0/dz, 1.0/dz ) );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,XSurfXField,XVolField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,YSurfYField,YVolField>::type( 0.5, 0.5 ) );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,ZSurfZField,ZVolField>::type( 0.5, 0.5 ) );

    //___________________________________________________________________
    // NullStencil:
    //
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,SVolField>::type() );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,XVolField,XVolField>::type() );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,YVolField,YVolField>::type() );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,ZVolField,ZVolField>::type() );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,XVolField,SSurfXField>::type() );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,YVolField,SSurfYField>::type() );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,ZVolField,SSurfZField>::type() );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,XSurfXField>::type() );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,YSurfYField>::type() );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,ZSurfZField>::type() );

    //___________________________________________________________________
    // stencil4:
    //
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,XSurfYField>::type(.25,.25,.25,.25) );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,XSurfZField>::type(.25,.25,.25,.25) );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,YSurfXField>::type(.25,.25,.25,.25) );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,YSurfZField>::type(.25,.25,.25,.25) );

    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,ZSurfXField>::type(.25,.25,.25,.25) );
    opdb.register_new_operator( new OperatorTypeBuilder<Interpolant,SVolField,ZSurfYField>::type(.25,.25,.25,.25) );
  }

  //------------------------------------------------------------------

} // namespace structured
} // namespace SpatialOps
