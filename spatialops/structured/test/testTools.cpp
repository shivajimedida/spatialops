#include <spatialops/SpatialOpsTools.h>
#include <spatialops/structured/FVTools.h>
#include <spatialops/structured/FVStaggeredFieldTypes.h>
#include <spatialops/structured/GhostData.h>

#include <test/TestHelper.h>

#include <iostream>
using std::cout;
using std::endl;

int main()
{

  using namespace SpatialOps;
  using namespace structured;

  TestHelper status(true);

  status( Abs<-2>::result == 2, "Abs(-2)" );
  status( Abs< 2>::result == 2, "Abs( 2)" );

  typedef IndexTriplet< 0, 0, 0> T000;
  typedef IndexTriplet< 1, 0, 0> T100;
  typedef IndexTriplet< 0, 1, 0> T010;
  typedef IndexTriplet< 1, 1, 0> T110;
  typedef IndexTriplet< 0, 0, 1> T001;
  typedef IndexTriplet< 1, 0, 1> T101;
  typedef IndexTriplet< 0, 1, 1> T011;
  typedef IndexTriplet< 1, 1, 1> T111;
  typedef IndexTriplet<-1, 0, 0> T200;
  typedef IndexTriplet< 0,-1, 0> T020;
  typedef IndexTriplet<-1,-1, 0> T220;
  typedef IndexTriplet< 0, 0,-1> T002;
  typedef IndexTriplet<-1, 0,-1> T202;
  typedef IndexTriplet< 0,-1,-1> T022;
  typedef IndexTriplet<-1,-1,-1> T222;

  status( IsSameType< IndexTriplet<-1,2,-3>::PositiveOrZero, IndexTriplet<0,2,0> >::result, "PositiveOrZero" );

  // jcs note that this is an incomplete set of tests.

  status( IsSameType< Add<T000,T100>::result, T100 >::result, "T000+T100" );
  status( IsSameType< Add<T222,T111>::result, T000 >::result, "T222+T111" );
  status( IsSameType< Subtract<T111,T111>::result, T000 >::result, "T111-T111" );

  status( IsSameType< LessThan<T000,T111>::result, T111 >::result, "LessThan T000 T111" );
  status( IsSameType< LessThan<T111,T000>::result, T000 >::result, "LessThan T111 T000" );
  status( IsSameType< LessThan<T101,T010>::result, T010 >::result, "LessThan T101 T010" );
  status( IsSameType< LessThan<T010,T101>::result, T101 >::result, "LessThan T010 T101" );
  status( IsSameType< LessThan<T111,T111>::result, T000 >::result, "LessThan T111 T111" );

  status( IsSameType< GreaterThan<T000,T111>::result, T000 >::result, "GreaterThan T000 T111" );
  status( IsSameType< GreaterThan<T111,T000>::result, T111 >::result, "GreaterThan T111 T000" );
  status( IsSameType< GreaterThan<T101,T010>::result, T101 >::result, "GreaterThan T101 T010" );
  status( IsSameType< GreaterThan<T010,T101>::result, T010 >::result, "GreaterThan T010 T101" );
  status( IsSameType< GreaterThan<T111,T111>::result, T000 >::result, "GreaterThan T111 T111" );

  status( IndexTripletExtract< Multiply<T111,T101>::result, XDIR >::value == 1, "T111*T101 [x] == 1" );
  status( IndexTripletExtract< Add     <T111,T101>::result, XDIR >::value == 2, "T111+T101 [x] == 2" );
  status( IndexTripletExtract< Subtract<T111,T101>::result, XDIR >::value == 0, "T111-T101 [x] == 0" );
  status( IndexTripletExtract<T101,XDIR>::value == 1 );
  status( IndexTripletExtract<T101,YDIR>::value == 0 );
  status( IndexTripletExtract<T202,ZDIR>::value == -1);

  status( IsSameType< GetNonzeroDir<T100>::DirT, XDIR >::result, "( 1, 0, 0) -> x" );
  status( IsSameType< GetNonzeroDir<T010>::DirT, YDIR >::result, "( 0, 1, 0) -> y" );
  status( IsSameType< GetNonzeroDir<T001>::DirT, ZDIR >::result, "( 0, 0, 1) -> z" );
  status( IsSameType< GetNonzeroDir<T200>::DirT, XDIR >::result, "(-1, 0, 0) -> x" );
  status( IsSameType< GetNonzeroDir<T020>::DirT, YDIR >::result, "( 0,-1, 0) -> y" );
  status( IsSameType< GetNonzeroDir<T002>::DirT, ZDIR >::result, "( 0, 0,-1) -> z" );

  status( IsSameType< UnitTriplet<XDIR>::type, T100 >::result, "X-unit" );
  status( IsSameType< UnitTriplet<YDIR>::type, T010 >::result, "Y-unit" );
  status( IsSameType< UnitTriplet<ZDIR>::type, T001 >::result, "Z-unit" );

  status( Kronecker<0,1>::value == 0, "delta(0,1)" );
  status( Kronecker<1,1>::value == 1, "delta(1,1)" );
  status( Kronecker<0,0>::value == 1, "delta(0,0)" );
  status( Kronecker<1,0>::value == 0, "delta(1,0)" );
  status( Kronecker<-1,-1>::value == 1, "delta(-1,-1)" );
  status( Kronecker<1,-1>::value == 0, "delta(1,-1)" );
  status( Kronecker<2,2>::value == 1, "delta(2,2)" );

  status( IsSameType< Invalidate<GhostData<1,1,1,1,1,1,1,1,1>,GhostData<0,0,0,0,0,0,0,0,0> >::result, GhostData<1,1,1,1,1,1,1,1,1> >::result, "GD invalidate 0");
  status( IsSameType< Invalidate<GhostData<2,2,2,2,2,2,2,2,2>,GhostData<1,1,1,1,1,1,1,1,1> >::result, GhostData<1,1,1,1,1,1,1,1,1> >::result, "Basic GD invalidate");
  status( IsSameType< Invalidate<GhostData<3,4,5,6,7,8,2,1,0>,GhostData<2,1,0,1,2,3,0,1,2> >::result, GhostData<1,3,5,5,5,5,2,0,-2> >::result, "Complex GD invalidiate");

  status( IsSameType< Invalidate<GhostData<2,3,3,4,5,5,6,7,7>,IndexTriplet<1,-1,0> >::result, GhostData<2,2,2,3,5,5,6,7,7> >::result, "GD/trip add test 1");
  status( IsSameType< Invalidate<GhostData<2,3,3,4,5,5,6,7,7>,IndexTriplet<1,0,-1> >::result, GhostData<2,2,2,4,5,5,5,7,7> >::result, "GD/trip add test 2");
  status( IsSameType< Invalidate<GhostData<2,3,3,4,5,5,6,7,7>,IndexTriplet<0,1,-1> >::result, GhostData<2,3,3,4,4,4,5,7,7> >::result, "GD/trip add test 3");
  status( IsSameType< Invalidate<GhostData<2,3,3,4,5,5,6,7,7>,IndexTriplet<0,-1,1> >::result, GhostData<2,3,3,3,5,5,6,6,6> >::result, "GD/trip add test 4");
  status( IsSameType< Invalidate<GhostData<2,3,3,4,5,5,6,7,7>,IndexTriplet<-1,0,1> >::result, GhostData<1,3,3,4,5,5,6,6,6> >::result, "GD/trip add test 5");
  status( IsSameType< Invalidate<GhostData<2,3,3,4,5,5,6,7,7>,IndexTriplet<-1,1,0> >::result, GhostData<1,3,3,4,4,4,6,7,7> >::result, "GD/trip add test 6");

  status( IsSameType< GhostFromField<TestField<NoGhost,  T000> >::result, GhostData<0,0,0,0,0,0,0,0,0> >::result, "GhostFromField test 1");
  status( IsSameType< GhostFromField<TestField<OneGhost, T000> >::result, GhostData<1,1,1,1,1,1,1,1,1> >::result, "GhostFromField test 2");
  status( IsSameType< GhostFromField<TestField<TwoGhost, T000> >::result, GhostData<2,2,2,2,2,2,2,2,2> >::result, "GhostFromField test 3");
  status( IsSameType< GhostFromField<TestField<NoGhost,  T100> >::result, GhostData<0,0,1,0,0,0,0,0,0> >::result, "GhostFromField test 4");
  status( IsSameType< GhostFromField<TestField<NoGhost,  T010> >::result, GhostData<0,0,0,0,0,1,0,0,0> >::result, "GhostFromField test 5");
  status( IsSameType< GhostFromField<TestField<NoGhost,  T111> >::result, GhostData<0,0,1,0,0,1,0,0,1> >::result, "GhostFromField test 6");
  status( IsSameType< GhostFromField<TestField<OneGhost, T100> >::result, GhostData<1,1,2,1,1,1,1,1,1> >::result, "GhostFromField test 7");
  status( IsSameType< GhostFromField<TestField<TwoGhost, T011> >::result, GhostData<2,2,2,2,2,3,2,2,3> >::result, "GhostFromField test 8");

  status( IsSameType< Minimum<GhostData<1,1,1,1,1,1,1,1,1>, GhostData<0,0,0,0,0,0,0,0,0> >::result, GhostData<0,0,0,0,0,0,0,0,0> >::result, "Minimum GD test 1");
  status( IsSameType< Minimum<GhostData<0,0,0,0,0,0,0,0,0>, GhostData<1,1,1,1,1,1,1,1,1> >::result, GhostData<0,0,0,0,0,0,0,0,0> >::result, "Minimum GD test 2");
  status( IsSameType< Minimum<GhostData<1,2,3,4,5,6,7,8,9>, GhostData<9,8,7,6,5,4,3,2,1> >::result, GhostData<1,2,3,4,5,4,3,2,1> >::result, "Minimum GD test 3");

  if( status.ok() ){
    cout << "PASS" << endl;
    return 0;
  }

  cout << "FAIL" << endl;
  return -1;
}
