/* This file was generated by fulmar version 0.7.6. */
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

#ifndef SpatialOps_FieldExpressionsFDStencil_2_h
#  define SpatialOps_FieldExpressionsFDStencil_2_h

#  include <spatialops/structured/FVStaggeredFieldTypes.h>
#  include <spatialops/FieldExpressions.h>
#  include <spatialops/structured/stencil/FDStencil2.h>

   /*#include <iostream> */

#  ifdef STENCIL_THREADS
#     include <vector>
#     include <boost/bind.hpp>
#     include <boost/ref.hpp>
#     include <spatialops/ThreadPool.h>
#     include <spatialops/structured/IntVec.h>
#     include <boost/interprocess/sync/interprocess_semaphore.hpp>
      namespace BI = boost::interprocess;
#  endif
   /* STENCIL_THREADS */

   namespace SpatialOps {

      namespace structured {

         template<typename OperatorType, typename FieldType, typename DirVec>
          inline void fd_stencil_2_apply_to_field_sequential_execute_internal(FieldType const & src,
                                                                              FieldType & dest,
                                                                              double const low,
                                                                              double const high) {

             const MemoryWindow & w = src.window_with_ghost();

             const IntVec shift = DirVec::int_vec() + DirVec::int_vec();

             const MemoryWindow ws1(w.glob_dim(),
                                    w.offset(),
                                    w.extent() - shift,
                                    w.has_bc(0),
                                    w.has_bc(1),
                                    w.has_bc(2));

             const MemoryWindow ws2(w.glob_dim(),
                                    w.offset() + shift,
                                    w.extent() - shift,
                                    w.has_bc(0),
                                    w.has_bc(1),
                                    w.has_bc(2));

             const MemoryWindow wd(w.glob_dim(),
                                   w.offset() + DirVec::int_vec(),
                                   w.extent() - shift,
                                   w.has_bc(0),
                                   w.has_bc(1),
                                   w.has_bc(2));

             FieldType d(wd, dest.field_values(), ExternalStorage);

             FieldType s1(ws1, src.field_values(), ExternalStorage);

             FieldType s2(ws2, src.field_values(), ExternalStorage);

             typename FieldType::iterator id = d.begin();

             typename FieldType::iterator ide = d.end();

             typename FieldType::const_iterator is1 = s1.begin();

             typename FieldType::const_iterator is2 = s2.begin();

             for(; id != ide; ++id, ++is1, ++is2){ *id = *is1 * low + *is2 * high; };
          };

         template<typename OperatorType, typename FieldType, typename DirVec>
          inline void fd_stencil_2_apply_to_field_sequential_execute(FieldType const & src,
                                                                     FieldType & dest,
                                                                     double const low,
                                                                     double const high) {

             fd_stencil_2_apply_to_field_sequential_execute_internal<OperatorType, FieldType, DirVec>(src,
                                                                                                      dest,
                                                                                                      low,
                                                                                                      high);
          };

#        ifdef STENCIL_THREADS
            template<typename OperatorType, typename SrcType, typename DestType, typename DirVec>
             inline void fd_stencil_2_apply_to_field_thread_parallel_execute_internal(SrcType const
                                                                                      & src,
                                                                                      DestType &
                                                                                      dest,
                                                                                      double const
                                                                                      low,
                                                                                      double const
                                                                                      high,
                                                                                      MemoryWindow
                                                                                      const & sw,
                                                                                      MemoryWindow
                                                                                      const & dw,
                                                                                      BI::
                                                                                      interprocess_semaphore
                                                                                      * sem) {

                fd_stencil_2_apply_to_field_sequential_execute_internal<OperatorType,
                                                                        typename DestType::
                                                                        field_type,
                                                                        DirVec>(src.resize(sw).field(),
                                                                                dest.resize(dw).field(),
                                                                                low,
                                                                                high);

                sem->post();
             }
#        endif
         /* STENCIL_THREADS */;

#        ifdef STENCIL_THREADS
            template<typename OperatorType, typename FieldType, typename DirVec>
             inline void fd_stencil_2_apply_to_field_thread_parallel_execute(FieldType const & src,
                                                                             FieldType & dest,
                                                                             double const low,
                                                                             double const high,
                                                                             int const
                                                                             number_of_partitions) {

                typename NeboConstField<Initial, FieldType>::template Iterator<UseWholeIterator>::
                ResizePrepType typedef SrcPmtrType;

                typename NeboField<Initial, FieldType>::template Iterator<UseWholeIterator>::
                ResizePrepType typedef DestPmtrType;

                MemoryWindow sw = src.window_with_ghost();

                MemoryWindow dw = dest.window_with_ghost();

                int x = 1;
                int y = 1;
                int z = 1;

                if(number_of_partitions <= sw.extent(2)){ z = number_of_partitions; }
                else if(number_of_partitions <= sw.extent(1)){ y = number_of_partitions; }
                else if(number_of_partitions <= sw.extent(0)){ x = number_of_partitions; };

                typename FieldType::field_type::Location::BCExtra typedef SrcBCExtra;
                typename FieldType::field_type::Location::BCExtra typedef DestBCExtra;
                structured::IntVec sBC = sw.has_bc() * SrcBCExtra::int_vec();
                structured::IntVec dBC = dw.has_bc() * DestBCExtra::int_vec();

                std::vector<MemoryWindow> vec_sw = sw.split(structured::IntVec(x, y, z),
                                                            FieldType::Ghost::NGhostMinus::int_vec(),
                                                            FieldType::Ghost::NGhostPlus::int_vec(),
                                                            sBC);

                std::vector<MemoryWindow> vec_dw = dw.split(structured::IntVec(x, y, z),
                                                            FieldType::Ghost::NGhostMinus::int_vec(),
                                                            FieldType::Ghost::NGhostPlus::int_vec(),
                                                            dBC);

                BI::interprocess_semaphore semaphore(0);

                typename std::vector<MemoryWindow>::const_iterator ivec_sw = vec_sw.begin();

                typename std::vector<MemoryWindow>::const_iterator ivec_dw = vec_dw.begin();

                typename std::vector<MemoryWindow>::const_iterator evec_sw = vec_sw.end();

                for(; ivec_sw != evec_sw; ++ivec_sw, ++ivec_dw){

                   ThreadPoolFIFO::self().schedule(boost::bind(&
                                                               fd_stencil_2_apply_to_field_thread_parallel_execute_internal<OperatorType,
                                                                                                                            SrcPmtrType,
                                                                                                                            DestPmtrType,
                                                                                                                            DirVec>,
                                                               NeboConstField<Initial, FieldType>(src).template
                                                                                                       resize_prep<UseWholeIterator>(),
                                                               NeboField<Initial, FieldType>(dest).template
                                                                                                   resize_prep<UseWholeIterator>(),
                                                               low,
                                                               high,
                                                               *ivec_sw,
                                                               *ivec_dw,
                                                               &semaphore));
                };

                for(int ii = 0; ii < vec_sw.size(); ii++){ semaphore.wait(); };
             }
#        endif
         /* STENCIL_THREADS */;

         template<typename OperatorType, typename FieldType, typename DirVec>
          inline void fd_stencil_2_apply_to_field_general_execute(FieldType const & src,
                                                                  FieldType & dest,
                                                                  double const low,
                                                                  double const high) {

#            ifdef STENCIL_THREADS
                (is_nebo_thread_parallel() ? fd_stencil_2_apply_to_field_thread_parallel_execute<OperatorType,
                                                                                                 FieldType,
                                                                                                 DirVec>(src,
                                                                                                         dest,
                                                                                                         low,
                                                                                                         high,
                                                                                                         get_nebo_thread_count())
                 : fd_stencil_2_apply_to_field_sequential_execute<OperatorType, FieldType, DirVec>(src,
                                                                                                   dest,
                                                                                                   low,
                                                                                                   high))
#            else
                fd_stencil_2_apply_to_field_sequential_execute<OperatorType, FieldType, DirVec>(src,
                                                                                                dest,
                                                                                                low,
                                                                                                high)
#            endif
             /* STENCIL_THREADS */
             ;
          };
      } /* structured */;
   } /* SpatialOps */;

#endif
/* SpatialOps_FieldExpressionsFDStencil_2_h */