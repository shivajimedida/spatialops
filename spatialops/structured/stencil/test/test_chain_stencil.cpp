/* This file was generated by fulmar version 0.7.6.1. */
/*
 * Copyright (c) 2011 The University of Utah
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <spatialops/SpatialOpsTools.h>
#include <spatialops/structured/FVStaggeredFieldTypes.h>
#include <spatialops/OperatorDatabase.h>
#include <spatialops/structured/FVTools.h>
#include <spatialops/structured/stencil/FVStaggeredOperatorTypes.h>
#include <spatialops/structured/stencil/StencilBuilder.h>
#include <test/TestHelper.h>
#include <test/FieldHelper.h>
#include <spatialops/FieldExpressions.h>
#include <boost/program_options.hpp>
#include <string>
#include <stdexcept>

#include "ReferenceStencil.h"

using namespace SpatialOps;

using namespace structured;

using std::cout;

using std::endl;

using std::string;

namespace po = boost::program_options;

template<typename FirstOpType, typename SecondOpType, typename SrcType, typename ItmdType, typename DestType>
 inline bool test_stencil_chain(OperatorDatabase & opdb, IntVec npts, bool bc[]) {

    /* basic definitions: */
    const MemoryWindow mwSrc = get_window_with_ghost<SrcType>(npts, bc[0], bc[1], bc[2]);
    const MemoryWindow mwItmd = get_window_with_ghost<ItmdType>(npts, bc[0], bc[1], bc[2]);
    const MemoryWindow mwDest = get_window_with_ghost<DestType>(npts, bc[0], bc[1], bc[2]);
    SrcType src(mwSrc, NULL);
    ItmdType itmd(mwItmd, NULL);
    DestType ref(mwDest, NULL);
    DestType test(mwDest, NULL);

    /* initialize source field / zero out result fields: */
    initialize_field(src);
    itmd <<= 0.0;
    ref <<= 0.0;
    test <<= 0.0;

    /* get first operator */
    typename OperatorTypeBuilder<FirstOpType, SrcType, ItmdType>::type typedef FirstOp;
    const FirstOp * const firstOp = opdb.retrieve_operator<FirstOp>();

    /* get second operator: */
    typename OperatorTypeBuilder<SecondOpType, ItmdType, DestType>::type typedef SecondOp;
    const SecondOp * const secondOp = opdb.retrieve_operator<SecondOp>();

    /* run reference: */
    itmd <<= (*firstOp)(src);
    ref <<= (*secondOp)(itmd);

    /* run operator: */
    test <<= (*secondOp)((*firstOp)(src));

    return interior_display_fields_compare(ref, test, false, false);
 };

template<typename FirstOpType, typename SecondOpType, typename SrcType, typename ItmdType, typename DestType>
 inline void test_stencil_chain_with_status(TestHelper & status, OperatorDatabase & opdb, IntVec npts, bool bc[], string const & str) {
    status(test_stencil_chain<FirstOpType, SecondOpType, SrcType, ItmdType, DestType>(opdb, npts, bc), str);
 };

int main (int iarg, char* carg[]) {

   int nx, ny, nz;

   bool bc[] = {false, false, false};

   po::options_description desc("Supported Options");

   desc.add_options()
   ("help", "print help message\n")
   ("nx", po::value<int>(&nx)->default_value(11), "number of points in x-dir for base mesh")
   ("ny", po::value<int>(&ny)->default_value(11), "number of points in y-dir for base mesh")
   ("nz", po::value<int>(&nz)->default_value(11), "number of points in z-dir for base mesh")
   ("bcx", "physical boundary on +x side?")
   ("bcy", "physical boundary on +y side?")
   ("bcz", "physical boundary on +z side?");

   po::variables_map args;

   po::store(po::parse_command_line(iarg, carg, desc), args);

   po::notify(args);

   if(args.count("bcx")) bc[0] = true;

   if(args.count("bcy")) bc[1] = true;

   if(args.count("bcz")) bc[2] = true;

   if(args.count("help")) {

      cout << desc << endl << "Examples:" << endl
       << " test_stencil --nx 5 --ny 10 --nz 3 --bcx" << endl
       << " test_stencil --bcx --bcy --bcz" << endl
       << " test_stencil --nx 50 --bcz" << endl
       << endl;

      return -1;
   };

   TestHelper status(true);

   const IntVec npts(nx, ny, nz);

   cout << "Run information:" << endl
    << "  bcx    : " << (bc[0] ? "ON" : "OFF") << endl
    << "  bcy    : " << (bc[1] ? "ON" : "OFF") << endl
    << "  bcz    : " << (bc[2] ? "ON" : "OFF") << endl
    << "  domain : " << npts << endl
    << endl;

   const double length = 10.0;

   OperatorDatabase opdb;

   build_stencils(npts[0], npts[1], npts[2], length, length, length, opdb);

   try{

      /* Test for chains starting with "Interpolant (SVolField->SSurfXField)" */
      if(npts[0]>1)
       test_stencil_chain_with_status<Interpolant, Divergence, SVolField, SSurfXField, SVolField>(status, opdb, npts, bc, "Interpolant (SVolField->SSurfXField) -> Divergence (SSurfXField->SVolField)");

      /* Test for chains starting with "Interpolant (SVolField->SSurfYField)" */
      if(npts[1]>1)
       test_stencil_chain_with_status<Interpolant, Divergence, SVolField, SSurfYField, SVolField>(status, opdb, npts, bc, "Interpolant (SVolField->SSurfYField) -> Divergence (SSurfYField->SVolField)");

      /* Test for chains starting with "Interpolant (SVolField->SSurfZField)" */
      if(npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Divergence, SVolField, SSurfZField, SVolField>(status, opdb, npts, bc, "Interpolant (SVolField->SSurfZField) -> Divergence (SSurfZField->SVolField)");

      /* Test for chains starting with "Divergence (SSurfXField->SVolField)" */
      if(npts[0]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfXField, SVolField, SSurfXField>(status, opdb, npts, bc, "Divergence (SSurfXField->SVolField) -> Interpolant (SVolField->SSurfXField)");
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfXField, SVolField, SSurfYField>(status, opdb, npts, bc, "Divergence (SSurfXField->SVolField) -> Interpolant (SVolField->SSurfYField)");
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfXField, SVolField, SSurfZField>(status, opdb, npts, bc, "Divergence (SSurfXField->SVolField) -> Interpolant (SVolField->SSurfZField)");
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfXField, SVolField, XSurfYField>(status, opdb, npts, bc, "Divergence (SSurfXField->SVolField) -> Interpolant (SVolField->XSurfYField)");
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfXField, SVolField, XSurfZField>(status, opdb, npts, bc, "Divergence (SSurfXField->SVolField) -> Interpolant (SVolField->XSurfZField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfXField, SVolField, YSurfZField>(status, opdb, npts, bc, "Divergence (SSurfXField->SVolField) -> Interpolant (SVolField->YSurfZField)");
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<Divergence, InterpolantY, SSurfXField, SVolField, SVolField>(status, opdb, npts, bc, "Divergence (SSurfXField->SVolField) -> InterpolantY (SVolField->SVolField)");
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, InterpolantZ, SSurfXField, SVolField, SVolField>(status, opdb, npts, bc, "Divergence (SSurfXField->SVolField) -> InterpolantZ (SVolField->SVolField)");

      /* Test for chains starting with "Divergence (SSurfYField->SVolField)" */
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfYField, SVolField, SSurfXField>(status, opdb, npts, bc, "Divergence (SSurfYField->SVolField) -> Interpolant (SVolField->SSurfXField)");
      if(npts[1]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfYField, SVolField, SSurfYField>(status, opdb, npts, bc, "Divergence (SSurfYField->SVolField) -> Interpolant (SVolField->SSurfYField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfYField, SVolField, SSurfZField>(status, opdb, npts, bc, "Divergence (SSurfYField->SVolField) -> Interpolant (SVolField->SSurfZField)");
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfYField, SVolField, XSurfYField>(status, opdb, npts, bc, "Divergence (SSurfYField->SVolField) -> Interpolant (SVolField->XSurfYField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfYField, SVolField, XSurfZField>(status, opdb, npts, bc, "Divergence (SSurfYField->SVolField) -> Interpolant (SVolField->XSurfZField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfYField, SVolField, YSurfZField>(status, opdb, npts, bc, "Divergence (SSurfYField->SVolField) -> Interpolant (SVolField->YSurfZField)");
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<Divergence, InterpolantX, SSurfYField, SVolField, SVolField>(status, opdb, npts, bc, "Divergence (SSurfYField->SVolField) -> InterpolantX (SVolField->SVolField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, InterpolantZ, SSurfYField, SVolField, SVolField>(status, opdb, npts, bc, "Divergence (SSurfYField->SVolField) -> InterpolantZ (SVolField->SVolField)");

      /* Test for chains starting with "Divergence (SSurfZField->SVolField)" */
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfZField, SVolField, SSurfXField>(status, opdb, npts, bc, "Divergence (SSurfZField->SVolField) -> Interpolant (SVolField->SSurfXField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfZField, SVolField, SSurfYField>(status, opdb, npts, bc, "Divergence (SSurfZField->SVolField) -> Interpolant (SVolField->SSurfYField)");
      if(npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfZField, SVolField, SSurfZField>(status, opdb, npts, bc, "Divergence (SSurfZField->SVolField) -> Interpolant (SVolField->SSurfZField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfZField, SVolField, XSurfYField>(status, opdb, npts, bc, "Divergence (SSurfZField->SVolField) -> Interpolant (SVolField->XSurfYField)");
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfZField, SVolField, XSurfZField>(status, opdb, npts, bc, "Divergence (SSurfZField->SVolField) -> Interpolant (SVolField->XSurfZField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, Interpolant, SSurfZField, SVolField, YSurfZField>(status, opdb, npts, bc, "Divergence (SSurfZField->SVolField) -> Interpolant (SVolField->YSurfZField)");
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, InterpolantX, SSurfZField, SVolField, SVolField>(status, opdb, npts, bc, "Divergence (SSurfZField->SVolField) -> InterpolantX (SVolField->SVolField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Divergence, InterpolantY, SSurfZField, SVolField, SVolField>(status, opdb, npts, bc, "Divergence (SSurfZField->SVolField) -> InterpolantY (SVolField->SVolField)");

      /* Test for chains starting with "Interpolant (SVolField->XSurfYField)" */
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, SVolField, XSurfYField, SVolField>(status, opdb, npts, bc, "Interpolant (SVolField->XSurfYField) -> Interpolant (XSurfYField->SVolField)");

      /* Test for chains starting with "Interpolant (SVolField->XSurfZField)" */
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, SVolField, XSurfZField, SVolField>(status, opdb, npts, bc, "Interpolant (SVolField->XSurfZField) -> Interpolant (XSurfZField->SVolField)");

      /* Test for chains starting with "Interpolant (SVolField->YSurfZField)" */
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, SVolField, YSurfZField, SVolField>(status, opdb, npts, bc, "Interpolant (SVolField->YSurfZField) -> Interpolant (YSurfZField->SVolField)");

      /* Test for chains starting with "Interpolant (XSurfYField->SVolField)" */
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XSurfYField, SVolField, SSurfYField>(status, opdb, npts, bc, "Interpolant (XSurfYField->SVolField) -> Interpolant (SVolField->SSurfYField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XSurfYField, SVolField, SSurfZField>(status, opdb, npts, bc, "Interpolant (XSurfYField->SVolField) -> Interpolant (SVolField->SSurfZField)");
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XSurfYField, SVolField, XSurfYField>(status, opdb, npts, bc, "Interpolant (XSurfYField->SVolField) -> Interpolant (SVolField->XSurfYField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XSurfYField, SVolField, XSurfZField>(status, opdb, npts, bc, "Interpolant (XSurfYField->SVolField) -> Interpolant (SVolField->XSurfZField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XSurfYField, SVolField, YSurfZField>(status, opdb, npts, bc, "Interpolant (XSurfYField->SVolField) -> Interpolant (SVolField->YSurfZField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, InterpolantZ, XSurfYField, SVolField, SVolField>(status, opdb, npts, bc, "Interpolant (XSurfYField->SVolField) -> InterpolantZ (SVolField->SVolField)");

      /* Test for chains starting with "Interpolant (XSurfZField->SVolField)" */
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XSurfZField, SVolField, SSurfYField>(status, opdb, npts, bc, "Interpolant (XSurfZField->SVolField) -> Interpolant (SVolField->SSurfYField)");
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XSurfZField, SVolField, SSurfZField>(status, opdb, npts, bc, "Interpolant (XSurfZField->SVolField) -> Interpolant (SVolField->SSurfZField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XSurfZField, SVolField, XSurfYField>(status, opdb, npts, bc, "Interpolant (XSurfZField->SVolField) -> Interpolant (SVolField->XSurfYField)");
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XSurfZField, SVolField, XSurfZField>(status, opdb, npts, bc, "Interpolant (XSurfZField->SVolField) -> Interpolant (SVolField->XSurfZField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XSurfZField, SVolField, YSurfZField>(status, opdb, npts, bc, "Interpolant (XSurfZField->SVolField) -> Interpolant (SVolField->YSurfZField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, InterpolantY, XSurfZField, SVolField, SVolField>(status, opdb, npts, bc, "Interpolant (XSurfZField->SVolField) -> InterpolantY (SVolField->SVolField)");

      /* Test for chains starting with "Interpolant (YSurfZField->SVolField)" */
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, YSurfZField, SVolField, SSurfXField>(status, opdb, npts, bc, "Interpolant (YSurfZField->SVolField) -> Interpolant (SVolField->SSurfXField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, YSurfZField, SVolField, SSurfZField>(status, opdb, npts, bc, "Interpolant (YSurfZField->SVolField) -> Interpolant (SVolField->SSurfZField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, YSurfZField, SVolField, XSurfZField>(status, opdb, npts, bc, "Interpolant (YSurfZField->SVolField) -> Interpolant (SVolField->XSurfZField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, YSurfZField, SVolField, YSurfZField>(status, opdb, npts, bc, "Interpolant (YSurfZField->SVolField) -> Interpolant (SVolField->YSurfZField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, InterpolantX, YSurfZField, SVolField, SVolField>(status, opdb, npts, bc, "Interpolant (YSurfZField->SVolField) -> InterpolantX (SVolField->SVolField)");

      /* Test for chains starting with "Interpolant (XVolField->YVolField)" */
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XVolField, YVolField, XVolField>(status, opdb, npts, bc, "Interpolant (XVolField->YVolField) -> Interpolant (YVolField->XVolField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XVolField, YVolField, ZVolField>(status, opdb, npts, bc, "Interpolant (XVolField->YVolField) -> Interpolant (YVolField->ZVolField)");

      /* Test for chains starting with "Interpolant (XVolField->ZVolField)" */
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XVolField, ZVolField, XVolField>(status, opdb, npts, bc, "Interpolant (XVolField->ZVolField) -> Interpolant (ZVolField->XVolField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, XVolField, ZVolField, YVolField>(status, opdb, npts, bc, "Interpolant (XVolField->ZVolField) -> Interpolant (ZVolField->YVolField)");

      /* Test for chains starting with "Interpolant (YVolField->XVolField)" */
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, YVolField, XVolField, YVolField>(status, opdb, npts, bc, "Interpolant (YVolField->XVolField) -> Interpolant (XVolField->YVolField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, YVolField, XVolField, ZVolField>(status, opdb, npts, bc, "Interpolant (YVolField->XVolField) -> Interpolant (XVolField->ZVolField)");

      /* Test for chains starting with "Interpolant (YVolField->ZVolField)" */
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, YVolField, ZVolField, XVolField>(status, opdb, npts, bc, "Interpolant (YVolField->ZVolField) -> Interpolant (ZVolField->XVolField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, YVolField, ZVolField, YVolField>(status, opdb, npts, bc, "Interpolant (YVolField->ZVolField) -> Interpolant (ZVolField->YVolField)");

      /* Test for chains starting with "Interpolant (ZVolField->XVolField)" */
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, ZVolField, XVolField, YVolField>(status, opdb, npts, bc, "Interpolant (ZVolField->XVolField) -> Interpolant (XVolField->YVolField)");
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, ZVolField, XVolField, ZVolField>(status, opdb, npts, bc, "Interpolant (ZVolField->XVolField) -> Interpolant (XVolField->ZVolField)");

      /* Test for chains starting with "Interpolant (ZVolField->YVolField)" */
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, ZVolField, YVolField, XVolField>(status, opdb, npts, bc, "Interpolant (ZVolField->YVolField) -> Interpolant (YVolField->XVolField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<Interpolant, Interpolant, ZVolField, YVolField, ZVolField>(status, opdb, npts, bc, "Interpolant (ZVolField->YVolField) -> Interpolant (YVolField->ZVolField)");

      /* Test for chains starting with "InterpolantX (SVolField->SVolField)" */
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<InterpolantX, Interpolant, SVolField, SVolField, SSurfYField>(status, opdb, npts, bc, "InterpolantX (SVolField->SVolField) -> Interpolant (SVolField->SSurfYField)");
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<InterpolantX, Interpolant, SVolField, SVolField, SSurfZField>(status, opdb, npts, bc, "InterpolantX (SVolField->SVolField) -> Interpolant (SVolField->SSurfZField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<InterpolantX, Interpolant, SVolField, SVolField, YSurfZField>(status, opdb, npts, bc, "InterpolantX (SVolField->SVolField) -> Interpolant (SVolField->YSurfZField)");
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<InterpolantX, InterpolantY, SVolField, SVolField, SVolField>(status, opdb, npts, bc, "InterpolantX (SVolField->SVolField) -> InterpolantY (SVolField->SVolField)");
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<InterpolantX, InterpolantZ, SVolField, SVolField, SVolField>(status, opdb, npts, bc, "InterpolantX (SVolField->SVolField) -> InterpolantZ (SVolField->SVolField)");

      /* Test for chains starting with "InterpolantY (SVolField->SVolField)" */
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<InterpolantY, Interpolant, SVolField, SVolField, SSurfXField>(status, opdb, npts, bc, "InterpolantY (SVolField->SVolField) -> Interpolant (SVolField->SSurfXField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<InterpolantY, Interpolant, SVolField, SVolField, SSurfZField>(status, opdb, npts, bc, "InterpolantY (SVolField->SVolField) -> Interpolant (SVolField->SSurfZField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<InterpolantY, Interpolant, SVolField, SVolField, XSurfZField>(status, opdb, npts, bc, "InterpolantY (SVolField->SVolField) -> Interpolant (SVolField->XSurfZField)");
      if(npts[0]>1 & npts[1]>1)
       test_stencil_chain_with_status<InterpolantY, InterpolantX, SVolField, SVolField, SVolField>(status, opdb, npts, bc, "InterpolantY (SVolField->SVolField) -> InterpolantX (SVolField->SVolField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<InterpolantY, InterpolantZ, SVolField, SVolField, SVolField>(status, opdb, npts, bc, "InterpolantY (SVolField->SVolField) -> InterpolantZ (SVolField->SVolField)");

      /* Test for chains starting with "InterpolantZ (SVolField->SVolField)" */
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<InterpolantZ, Interpolant, SVolField, SVolField, SSurfXField>(status, opdb, npts, bc, "InterpolantZ (SVolField->SVolField) -> Interpolant (SVolField->SSurfXField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<InterpolantZ, Interpolant, SVolField, SVolField, SSurfYField>(status, opdb, npts, bc, "InterpolantZ (SVolField->SVolField) -> Interpolant (SVolField->SSurfYField)");
      if(npts[0]>1 & npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<InterpolantZ, Interpolant, SVolField, SVolField, XSurfYField>(status, opdb, npts, bc, "InterpolantZ (SVolField->SVolField) -> Interpolant (SVolField->XSurfYField)");
      if(npts[0]>1 & npts[2]>1)
       test_stencil_chain_with_status<InterpolantZ, InterpolantX, SVolField, SVolField, SVolField>(status, opdb, npts, bc, "InterpolantZ (SVolField->SVolField) -> InterpolantX (SVolField->SVolField)");
      if(npts[1]>1 & npts[2]>1)
       test_stencil_chain_with_status<InterpolantZ, InterpolantY, SVolField, SVolField, SVolField>(status, opdb, npts, bc, "InterpolantZ (SVolField->SVolField) -> InterpolantY (SVolField->SVolField)");
   }
   catch(std::runtime_error & e){ cout << e.what() << endl; };

   if(status.ok()) cout << "ALL TESTS PASSED :)" << endl;
   else cout << "******************************" << endl
    << " At least one test FAILED :(" << endl
    << "******************************" << endl;

   return status.ok() ? 0 : -1;
};