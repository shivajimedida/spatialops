#ifndef UT_LinearSystem_h
#define UT_LinearSystem_h

#include <vector>
#include <map>

// SpatialOps forward declarations:
namespace SpatialOps{
  class SpatialOperator;
  class SpatialField;
}


// trilinos foward declarations
class Epetra_Map;
class Epetra_Comm;
class Epetra_CrsMatrix;
class Epetra_Vector;
class Epetra_LinearProblem;
class AztecOO;


//====================================================================

/**
 *  @class  RHS
 *  @author James C. Sutherland
 *  @date   December, 2006
 *
 *  Abstraction for the RHS field.  This is intended for use in a
 *  segregated implicit scheme where the RHS is used in conjunction
 *  with a LHS operator, or in an explicit scheme where the RHS is
 *  simply used to obtain an update for the solution field directly.
 */
class RHS
{
public:

  RHS( const std::vector<int> & domainExtent );

  ~RHS();

  void reset( const double val = 0.0 );
  void add_contribution( const SpatialOps::SpatialField & localField,
			 const double scaleFac = 1.0 );

  const std::vector<double> & get_field() const{return field_;}

        double* get_ptr()      { return &field_[0]; }
  const double* get_ptr() const{ return &field_[0]; }

  const std::vector<int> & get_extent() const{return extent_;}

private:
  bool consistency_check( const SpatialOps::SpatialField & f ) const;

  const std::vector<int> extent_;
  const int npts_;

  std::vector<double> field_;

  RHS();
  RHS(const RHS&);
};

//====================================================================

/**
 *  @class  LHS
 *  @author James C. Sutherland
 *  @date   December, 2006
 *
 *  Abstraction for the LHS operator.  This is a distributed LHS
 *  operator for use in a linear system.
 */
class LHS
{
public:

  LHS( const std::vector<int> & extent,
       Epetra_CrsMatrix& A );

  ~LHS();

  int nrows() const{ return nrows_; }
  int ncols() const{ return ncols_; }

  void reset( const double val = 0 );

  /** add non-ghost elements of the local matrix to this LHS operator */
  void add_contribution( const SpatialOps::SpatialOperator & localMat,
			 const double scaleFac = 1.0 );

  /** add non-ghost elements of the local field to this LHS operator */
  void add_contribution( const SpatialOps::SpatialField & localField,
			 const double scaleFac = 1.0 );

        Epetra_CrsMatrix& epetra_mat()      { return A_; }
  const Epetra_CrsMatrix& epetra_mat() const{ return A_; }

private:

  bool compatibility_check( const SpatialOps::SpatialField& f ) const;
  bool compatibility_check( const SpatialOps::SpatialOperator& op ) const;

  Epetra_CrsMatrix & A_;
  const std::vector<int> extent_;
  const int nrows_;
  const int ncols_;

  LHS(const LHS& );
  LHS();

  std::vector<double> rowDWork_;
  std::vector<int>    rowIWork_;
};

//====================================================================

/**
 *  @class  LinearSystem
 *  @author James C. Sutherland
 *  @date   December, 2006
 *
 *  Basic support for a linear system distributed in parallel.
 */
class LinearSystem
{
public:

#ifdef HAVE_MPI
  LinearSystem( const std::vector<int> & dimExtent,
		MPI_Comm & comm );
#else
  LinearSystem( const std::vector<int> & dimExtent );
#endif

  ~LinearSystem();

  /** Zero the lhs and rhs */
  void reset();

  void solve();

        RHS & get_rhs()      { return rhs_; }
  const RHS & get_rhs() const{ return rhs_; }

        LHS & get_lhs()      { return *lhs_; }
  const LHS & get_lhs() const{ return *lhs_; }

protected:

  const std::vector<int> extent_;
  const int npts_;
  RHS   rhs_;
  LHS * lhs_;
  double * const solnFieldValues_;

  int maxIterations_;
  double solverTolerance_;

private:

  Epetra_CrsMatrix * A_;  // the distributed matrix
  Epetra_Vector    * b_;  // the distributed RHS vector
  Epetra_Vector    * x_;  // the distributed solution vector

  Epetra_Comm      * comm_;

  Epetra_LinearProblem * linProb_;
  AztecOO * aztec_;

};

//====================================================================


/**
 *  @class  LinSysMapFactory
 *  @author James C. Sutherland
 *  @date   December, 2006
 *
 *  Support for creating Epetra_Map objects for use with the
 *  LinearSystem class.
 */
class LinSysMapFactory
{
public:

  static LinSysMapFactory& self();

  Epetra_Map& get_map( const int npts,
		       Epetra_Comm & com );

private:

  std::map<int,Epetra_Map*> emap_;

  LinSysMapFactory();
  ~LinSysMapFactory();
};

//====================================================================

#endif
