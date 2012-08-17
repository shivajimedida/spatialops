#include <iostream>

//--- SpatialOps includes ---//
#include <spatialops/structured/FVStaggeredFieldTypes.h>
#include <spatialops/structured/FVTools.h>
#include <spatialops/structured/IntVec.h>
#include <spatialops/structured/MemoryWindow.h>
#include <spatialops/structured/stencil/FVStaggeredOperatorTypes.h>
#include <spatialops/OperatorDatabase.h>
#include <spatialops/structured/stencil/StencilBuilder.h>
#include <spatialops/structured/FVStaggeredFieldTypes.h>
#include <spatialops/FieldExpressions.h>

//-- boost includes ---//
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace po = boost::program_options;

using namespace SpatialOps;
using namespace structured;

int print_length = 10;

template<typename Field>
void print(Field const & given) {
  typename Field::const_iterator ig = given.begin();

  for(int i = 0; i < print_length; i++) {
    std::cout << *ig << std::endl;
    ++ig;
  };

  std::cout << std::endl;
};

template<typename Field>
void print_all(Field const & given) {
  typename Field::const_iterator ig = given.begin();

  structured::MemoryWindow window = given.window_with_ghost();
  for(int kk = 0; kk < window.glob_dim(2); kk++) {
      for(int jj = 0; jj < window.glob_dim(1); jj++) {
          for(int ii = 0; ii < window.glob_dim(0); ii++, ++ig) {
              std::cout << *ig << " ";
          }
          std::cout <<std::endl;
      }
      std::cout <<std::endl;
  };
};

#define INT_EQU(FIELD, FIRST, SECOND, PRINT)                            \
    {                                                                   \
        FIELD::const_iterator fi = FIRST.interior_begin();              \
        FIELD::const_iterator si = SECOND.interior_begin();             \
        FIELD::const_iterator se = SECOND.interior_end();               \
        bool flag = true;                                               \
        int count = 0;                                                  \
                                                                        \
        while( si != se ) {                                             \
            if( *fi != *si ) {                                          \
                flag = false;                                           \
                if( PRINT ) {                                           \
                    std::cout << count                                  \
                              << ": " << *fi                            \
                              << " " << *si << std::endl;               \
                };                                                      \
            };                                                          \
            ++fi;                                                       \
            ++si;                                                       \
            ++count;                                                    \
        };                                                              \
                                                                        \
        if( flag ) std::cout << "Success!";                             \
        else std::cout << "Error!";                                     \
        std::cout << std::endl;                                         \
    }


#define RUN_TEST(TEST,							\
		 TYPE)							\
  boost::posix_time::ptime start( boost::posix_time::microsec_clock::universal_time() ); \
  boost::posix_time::ptime end( boost::posix_time::microsec_clock::universal_time() ); \
  int ii = 0;								\
  start = boost::posix_time::microsec_clock::universal_time();		\
									\
  for(; ii < number_of_runs; ii++) {					\
    TEST;								\
  };									\
									\
  end = boost::posix_time::microsec_clock::universal_time();		\
  std::cout << TYPE;							\
  std::cout << " runs: ";						\
  std::cout << number_of_runs;						\
  std::cout << " result: ";						\
  std::cout << (end - start).total_microseconds()*1e-6;			\
  std::cout << std::endl;

template<typename FieldType>
inline void evaluate_serial_example(FieldType & result,
				    FieldType const & phi,
				    FieldType const & dCoef,
				    IntVec const npts,
				    double const Lx,
				    double const Ly,
				    double const Lz,
				    int number_of_runs) {

    SpatialOps::OperatorDatabase opDB;
    SpatialOps::structured::build_stencils(npts[0],
                                           npts[1],
                                           npts[2],
                                           Lx,
                                           Ly,
                                           Lz,
                                           opDB);

    typename BasicOpTypes<FieldType>::GradX* const gradXOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::GradX>();
    typename BasicOpTypes<FieldType>::GradY* const gradYOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::GradY>();
    typename BasicOpTypes<FieldType>::GradZ* const gradZOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::GradZ>();
    typename BasicOpTypes<FieldType>::InterpC2FX* const interpXOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::InterpC2FX>();
    typename BasicOpTypes<FieldType>::InterpC2FY* const interpYOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::InterpC2FY>();
    typename BasicOpTypes<FieldType>::InterpC2FZ* const interpZOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::InterpC2FZ>();
    typename BasicOpTypes<FieldType>::DivX* const divXOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::DivX>();
    typename BasicOpTypes<FieldType>::DivY* const divYOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::DivY>();
    typename BasicOpTypes<FieldType>::DivZ* const divZOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::DivZ>();

    MemoryWindow const w = phi.window_with_ghost();
    typename FaceTypes<FieldType>::XFace tmpFaceX( w, NULL );
    typename FaceTypes<FieldType>::XFace tmpFaceX2( w, NULL );
    FieldType tmpX( w, NULL );
    typename FaceTypes<FieldType>::YFace tmpFaceY( w, NULL );
    typename FaceTypes<FieldType>::YFace tmpFaceY2( w, NULL );
    FieldType tmpY( w, NULL );
    typename FaceTypes<FieldType>::ZFace tmpFaceZ( w, NULL );
    typename FaceTypes<FieldType>::ZFace tmpFaceZ2( w, NULL );
    FieldType tmpZ( w, NULL );

    RUN_TEST(// X - direction
	     gradXOp_  ->apply_to_field( phi,    tmpFaceX  );
	     interpXOp_->apply_to_field( dCoef, tmpFaceX2 );
	     tmpFaceX <<= tmpFaceX * tmpFaceX2;
	     divXOp_->apply_to_field( tmpFaceX, tmpX );

	     // Y - direction
	     gradYOp_  ->apply_to_field( phi,    tmpFaceY  );
	     interpYOp_->apply_to_field( dCoef, tmpFaceY2 );
	     tmpFaceY <<= tmpFaceY * tmpFaceY2;
	     divYOp_->apply_to_field( tmpFaceY, tmpY );

	     // Z - direction
	     gradZOp_  ->apply_to_field( phi,    tmpFaceZ  );
	     interpZOp_->apply_to_field( dCoef, tmpFaceZ2 );
	     tmpFaceZ <<= tmpFaceZ * tmpFaceZ2;
	     divZOp_->apply_to_field( tmpFaceZ, tmpZ );

	     result <<= - tmpX - tmpY - tmpZ,
	     "old");

};

template<typename FieldType>
inline void evaluate_chaining_example(FieldType & result,
				      FieldType const & phi,
				      FieldType const & dCoef,
				      IntVec const npts,
				      double const Lx,
				      double const Ly,
				      double const Lz,
				      int number_of_runs) {


    SpatialOps::OperatorDatabase opDB;
    SpatialOps::structured::build_stencils(npts[0],
                                           npts[1],
                                           npts[2],
                                           Lx,
					   Ly,
					   Lz,
                                           opDB);

    typename BasicOpTypes<FieldType>::GradX* const gradXOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::GradX>();
    NeboStencilConstructor<typename BasicOpTypes<FieldType>::GradX> neboGradX(gradXOp_);
    typename BasicOpTypes<FieldType>::GradY* const gradYOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::GradY>();
    NeboStencilConstructor<typename BasicOpTypes<FieldType>::GradY> neboGradY(gradYOp_);
    typename BasicOpTypes<FieldType>::GradZ* const gradZOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::GradZ>();
    NeboStencilConstructor<typename BasicOpTypes<FieldType>::GradZ> neboGradZ(gradZOp_);
    typename BasicOpTypes<FieldType>::InterpC2FX* const interpXOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::InterpC2FX>();
    NeboStencilConstructor<typename BasicOpTypes<FieldType>::InterpC2FX> neboInterpX(interpXOp_);
    typename BasicOpTypes<FieldType>::InterpC2FY* const interpYOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::InterpC2FY>();
    NeboStencilConstructor<typename BasicOpTypes<FieldType>::InterpC2FY> neboInterpY(interpYOp_);
    typename BasicOpTypes<FieldType>::InterpC2FZ* const interpZOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::InterpC2FZ>();
    NeboStencilConstructor<typename BasicOpTypes<FieldType>::InterpC2FZ> neboInterpZ(interpZOp_);
    typename BasicOpTypes<FieldType>::DivX* const divXOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::DivX>();
    NeboStencilConstructor<typename BasicOpTypes<FieldType>::DivX> neboDivX(divXOp_);
    typename BasicOpTypes<FieldType>::DivY* const divYOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::DivY>();
    NeboStencilConstructor<typename BasicOpTypes<FieldType>::DivY> neboDivY(divYOp_);
    typename BasicOpTypes<FieldType>::DivZ* const divZOp_ = opDB.retrieve_operator<typename BasicOpTypes<FieldType>::DivZ>();
    NeboStencilConstructor<typename BasicOpTypes<FieldType>::DivZ> neboDivZ(divZOp_);

    MemoryWindow const w = phi.window_with_ghost();
    typename FaceTypes<FieldType>::XFace tmpFaceX( w, NULL );
    typename FaceTypes<FieldType>::XFace tmpFaceX2( w, NULL );
    FieldType tmpX( w, NULL );
    typename FaceTypes<FieldType>::YFace tmpFaceY( w, NULL );
    typename FaceTypes<FieldType>::YFace tmpFaceY2( w, NULL );
    FieldType tmpY( w, NULL );
    typename FaceTypes<FieldType>::ZFace tmpFaceZ( w, NULL );
    typename FaceTypes<FieldType>::ZFace tmpFaceZ2( w, NULL );
    FieldType tmpZ( w, NULL );

    typename FaceTypes<FieldType>::XFace tmpFaceX0( w, NULL );
    typename FaceTypes<FieldType>::XFace tmpFaceX20( w, NULL );
    FieldType tmpX0( w, NULL );
    typename FaceTypes<FieldType>::YFace tmpFaceY0( w, NULL );
    typename FaceTypes<FieldType>::YFace tmpFaceY20( w, NULL );
    FieldType tmpY0( w, NULL );
    typename FaceTypes<FieldType>::ZFace tmpFaceZ0( w, NULL );
    typename FaceTypes<FieldType>::ZFace tmpFaceZ20( w, NULL );
    FieldType tmpZ0( w, NULL );

    RUN_TEST(// X - direction
             tmpFaceX <<= neboGradX(phi);
// 	     gradXOp_  ->apply_to_field( phi,    tmpFaceX0  );
//              INT_EQU(typename FaceTypes<FieldType>::XFace, tmpFaceX, tmpFaceX0, false);

             tmpFaceX2 <<= neboInterpX(dCoef);
// 	     interpXOp_->apply_to_field( dCoef, tmpFaceX20 );
//              INT_EQU(typename FaceTypes<FieldType>::XFace, tmpFaceX2, tmpFaceX20, false);

	     tmpFaceX <<= tmpFaceX * tmpFaceX2;
	     tmpX <<= neboDivX(tmpFaceX);
// 	     tmpFaceX0 <<= tmpFaceX0 * tmpFaceX20;
//              divXOp_->apply_to_field( tmpFaceX0, tmpX0 );
//              INT_EQU(typename FieldType, tmpX, tmpX0, false);

	     // Y - direction
             tmpFaceY <<= neboGradY(phi);
// 	     gradYOp_  ->apply_to_field( phi,    tmpFaceY0  );
//              INT_EQU(typename FaceTypes<FieldType>::YFace, tmpFaceY, tmpFaceY0, false);

	     tmpFaceY2 <<= neboInterpY(dCoef);
//              interpYOp_->apply_to_field( dCoef, tmpFaceY20 );
//              INT_EQU(typename FaceTypes<FieldType>::YFace, tmpFaceY2, tmpFaceY20, false);

	     tmpFaceY <<= tmpFaceY * tmpFaceY2;
	     tmpY <<= neboDivY(tmpFaceY);
// 	     tmpFaceY0 <<= tmpFaceY0 * tmpFaceY20;
//              divYOp_->apply_to_field( tmpFaceY0, tmpY0 );
//              INT_EQU(typename FieldType, tmpY, tmpY0, false);

	     // Z - direction
	     tmpFaceZ <<= neboGradZ(phi);
//              gradZOp_  ->apply_to_field( phi,    tmpFaceZ0  );
//              INT_EQU(typename FaceTypes<FieldType>::ZFace, tmpFaceZ, tmpFaceZ0, false);

	     tmpFaceZ2 <<= neboInterpZ(dCoef);
//              interpZOp_->apply_to_field( dCoef, tmpFaceZ20 );
//              INT_EQU(typename FaceTypes<FieldType>::ZFace, tmpFaceZ2, tmpFaceZ20, false);

	     tmpFaceZ <<= tmpFaceZ * tmpFaceZ2;
	     tmpZ <<= neboDivZ(tmpFaceZ);
// 	     tmpFaceZ0 <<= tmpFaceZ0 * tmpFaceZ20;
//              divZOp_->apply_to_field( tmpFaceZ0, tmpZ0 );
//              INT_EQU(typename FieldType, tmpZ, tmpZ0, false);

	     result <<= - tmpX - tmpY - tmpZ,
             // r <<= (- (dXcl * ((gXcl * phi_xlxl + gXch * phi_xhxl) * (iXcl * dCoef_xlxl + iXch * dCoef_xhxl)) +
// 		       dXch * ((gXcl * phi_xlxh + gXch * phi_xhxh) * (iXcl * dCoef_xlxh + iXch * dCoef_xhxh)))
// 		    - (dYcl * ((gYcl * phi_ylyl + gYch * phi_yhyl) * (iYcl * dCoef_ylyl + iYch * dCoef_yhyl)) +
// 		       dYch * ((gYcl * phi_ylyh + gYch * phi_yhyh) * (iYcl * dCoef_ylyh + iYch * dCoef_yhyh)))
// 		    - (dZcl * ((gZcl * phi_zlzl + gZch * phi_zhzl) * (iZcl * dCoef_zlzl + iZch * dCoef_zhzl)) +
// 		       dZch * ((gZcl * phi_zlzh + gZch * phi_zhzh) * (iZcl * dCoef_zlzh + iZch * dCoef_zhzh)))),
	     "new");

};

int main(int iarg, char* carg[]) {
    typedef SVolField Field;

    int nx, ny, nz;
    int number_of_runs;
    double Lx, Ly, Lz;

    // parse the command line options input describing the problem
    {
        po::options_description desc("Supported Options");
	desc.add_options()
	  ( "help", "print help message" )
	  ( "nx", po::value<int>(&nx)->default_value(10), "Grid in x" )
	  ( "ny", po::value<int>(&ny)->default_value(10), "Grid in y" )
	  ( "nz", po::value<int>(&nz)->default_value(10), "Grid in z" )
	  ( "Lx", po::value<double>(&Lx)->default_value(1.0),"Length in x")
	  ( "Ly", po::value<double>(&Ly)->default_value(1.0),"Length in y")
	  ( "Lz", po::value<double>(&Lz)->default_value(1.0),"Length in z")
	  ( "runs", po::value<int>(&number_of_runs)->default_value(1), "Number of iterations of each test");

	po::variables_map args;
	po::store( po::parse_command_line(iarg,carg,desc), args );
	po::notify(args);

	if (args.count("help")) {
	    std::cout << desc << "\n";
	    return 1;
	}
    }

    const MemoryWindow window( get_window_with_ghost<Field>(IntVec(nx,ny,nz),false,false,false) );

    Field a( window, NULL );
    Field b( window, NULL );
    Field cr( window, NULL );
    Field sr( window, NULL );

    Field::iterator ia = a.begin();
    Field::iterator ib = b.begin();
    for(int kk = 0; kk < window.glob_dim(2); kk++) {
        for(int jj = 0; jj < window.glob_dim(1); jj++) {
            for(int ii = 0; ii < window.glob_dim(0); ii++, ++ia, ++ib) {
	      *ia = ii + jj * 2 + kk * 4;
	      *ib = ii + jj * 3 + kk * 5;
            }
        }
    };

    evaluate_serial_example(sr,
			    a,
			    b,
			    IntVec(nx,ny,nz),
			    Lx,
			    Ly,
			    Lz,
			    number_of_runs);

    evaluate_chaining_example(cr,
			      a,
			      b,
			      IntVec(nx,ny,nz),
			      Lx,
			      Ly,
			      Lz,
			      number_of_runs);

    INT_EQU(Field, cr, sr, false);

    return 0;
};