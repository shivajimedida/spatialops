#ifndef SpatialOps_Structured_Stencil_h
#define SpatialOps_Structured_Stencil_h

#include <spatialops/structured/IndexTriplet.h>

namespace SpatialOps {
  namespace structured {

    /**
     *  \class Stencil2
     *  \author James C. Sutherland
     *
     *  \brief Support for implementing simple two-point stencils in
     *         one-dimension on structured meshes.
     *
     *  \tparam OpT - the type of operator (Interpolant, Gradient, Divergence)
     *  \tparam SrcT - the type of field the operator is applied to
     *  \tparam DestT - the type of field the operator produces
     *
     *  See also Stencil2Helper
     */
    template<typename OperatorT, typename SrcFieldT, typename DestFieldT>
    class Stencil2
    {
      const double coefLo_, coefHi_;
    public:

      typedef OperatorT   Type;           ///< The operator type (Interpolant, Gradient, Divergence)
      typedef SrcFieldT   SrcFieldType;   ///< The source field type
      typedef DestFieldT  DestFieldType;  ///< The destination field type

      /**
       *  \brief construct a stencil with the specified coefficients
       *  \param coefLo the coefficient to multiply the (-) side field by
       *  \param coefHi the coefficient to multiply the (+) side field by
       */
      Stencil2( const double coefLo, const double coefHi );

      ~Stencil2();

      /**
       * \brief Apply this operator to the supplied source field to produce the supplied destination field
       * \param src the field that the operator is applied to
       * \param dest the resulting field.
       */
      void apply_to_field( const SrcFieldType& src, DestFieldType& dest ) const;

      inline double get_minus_coef() const{ return coefLo_; } ///< get the (-) coefficient
      inline double  get_plus_coef() const{ return coefHi_; } ///< get the (+) coefficient
    };

    /*******************************************************************
     *
     * NOTE: all information in the s2detail namespace is meant only for
     *       use within the Stencil2Helper class and should not be used
     *       elsewhere!
     *
     ******************************************************************/
    namespace s2detail {

      /**
       *  \struct ActiveDir
       *
       *  \brief determines the direction that the operator is acting along
       *
       *  \tparam SrcT the source field type for this 2-point stencil operator
       *  \tparam DestT the destination field type for this 2-point stencil operator
       *
       *  Example:
       *  \code typedef ActiveDir<SrcT,DestT>::type OpDir; \endcode
       */
      template<typename SrcT, typename DestT>
      struct ActiveDir
      {
      private:
        typedef typename Subtract< typename  SrcT::Location::Offset,
                                   typename DestT::Location::Offset >::result  Difference;
      public:
        typedef typename GetNonzeroDir< Difference >::DirT type; ///< the direction that the operator acts in.
      };

      template<typename T> struct ActiveDir<T,T>;  ///< invalid - all Stencil2 must do something.


      /**
       * \struct ExtentsAndOffsets
       * \author James C. Sutherland
       * \brief Provides typedefs for dealing with extents and offsets for Stencil2 operators.
       */
      template<typename SrcT, typename DestT>
      struct ExtentsAndOffsets
      {
      private:
        typedef typename  SrcT::Location::Offset                SFO;            ///< Offset information for Source field
        typedef typename  SrcT::Location::BCExtra               SFBCExtra;      ///< Extra cell information for Source field
        typedef typename DestT::Location::Offset                DFO;            ///< Offset information for Destination field
        typedef typename DestT::Location::BCExtra               DFBCExtra;      ///< Extra cell information for Destination field

      public:

        typedef typename ActiveDir<SrcT,DestT>::type            Dir;            ///< The direction that this operator acts in

        typedef typename UnitTriplet<Dir>::type                 DirUnitVec;     ///< unit vector for the direction that this operator acts in.

        typedef typename DirUnitVec::Negate                     UpperLoopShift; ///< upper bounds on loop

        typedef typename Multiply<SFBCExtra,DirUnitVec>::result UpperLoopBCAug; ///< shift for upper bounds when BC is present

        typedef IndexTriplet<0,0,0>                             Src1Offset;     ///< offset for the first source field
        typedef UpperLoopShift                                  Src1Extent;     ///< extent modification for the first source field
        typedef typename Subtract<SFBCExtra,
            DFBCExtra>::result::PositiveOrZero::Negate          Src1ExtentBC;   ///< amount to augment source extents by if a BC is present

        typedef typename Add<DirUnitVec,Src1Offset>::result     Src2Offset;     ///< offset for the second source field
        typedef Src1Extent                                      Src2Extent;     ///< extent modification for the second source field
        typedef Src1ExtentBC                                    Src2ExtentBC;   ///< additional extent modification if a BC is present

        typedef typename Multiply< DirUnitVec,
            typename Subtract<SFO,DFO>::result
            >::result::PositiveOrZero                           DestOffset;     ///< the offset for the destination field
        typedef UpperLoopShift                                  DestExtent;     ///< the extent for the destination field
        typedef typename Subtract<DFBCExtra,
            SFBCExtra>::result::PositiveOrZero::Negate          DestExtentBC;   ///< amount to augment destination extents by if a BC is present

      };

    } // namespace s2detail

  }// namespace structured
} // namespace SpatialOps

#endif // SpatialOps_Structured_Stencil_h
