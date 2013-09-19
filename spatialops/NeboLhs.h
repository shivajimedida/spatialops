/* This file was generated by fulmar version 0.8.0. */

/*
 * Copyright (c) 2013 The University of Utah
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

#ifndef NEBO_LHS_H
#  define NEBO_LHS_H

   namespace SpatialOps {
#     ifdef __CUDACC__
         template<typename LhsType, typename RhsType>
          __global__ void gpu_assign_kernel(LhsType lhs, RhsType rhs) {
             lhs.assign(rhs);
          }
#     endif
      /* __CUDACC__ */;

      template<typename CurrentMode, typename FieldType>
       struct NeboField;
      template<typename FieldType>
       struct NeboField<Initial, FieldType> {
         public:
          FieldType typedef field_type;

          typename field_type::memory_window typedef MemoryWindow;

          NeboField<SeqWalk, FieldType> typedef SeqWalkType;

#         ifdef FIELD_EXPRESSION_THREADS
             NeboField<Resize, FieldType> typedef ResizeType;
#         endif
          /* FIELD_EXPRESSION_THREADS */

#         ifdef __CUDACC__
             NeboField<GPUWalk, FieldType> typedef GPUWalkType;
#         endif
          /* __CUDACC__ */

          NeboField<Reduction, FieldType> typedef ReductionType;

          typename structured::GhostFromField<FieldType>::result typedef
          PossibleValidGhost;

          NeboField(FieldType f)
          : field_(f)
          {}

          structured::IndexTriplet<0, 0, 0> typedef Shift;

          template<typename Iterator, typename RhsType>
           inline void assign(RhsType rhs) {
#             ifdef __CUDACC__
#                ifdef NEBO_GPU_TEST
                    gpu_test_assign<Iterator, RhsType>(rhs)
#                else
                    if(gpu_ready()) {
                       if(rhs.gpu_ready(gpu_device_index())) {
                          gpu_assign<Iterator, RhsType>(rhs);
                       }
                       else {
                          std::ostringstream msg;
                          msg << "Nebo error in " << "Nebo Assignment" << ":\n";
                          msg << "Left-hand side of assignment allocated on ";
                          msg << "GPU but right-hand side is not (completely) ";
                          msg << "accessible on the same GPU";
                          msg << "\n";
                          msg << "\t - " << __FILE__ << " : " << __LINE__;
                          throw(std::runtime_error(msg.str()));;
                       };
                    }
                    else {
                       if(cpu_ready()) {
                          if(rhs.cpu_ready()) {
                             cpu_assign<Iterator, RhsType>(rhs);
                          }
                          else {
                             std::ostringstream msg;
                             msg << "Nebo error in " << "Nebo Assignment" <<
                             ":\n";
                             msg << "Left-hand side of assignment allocated on ";
                             msg << "CPU but right-hand side is not ";
                             msg << "(completely) accessible on CPU";
                             msg << "\n";
                             msg << "\t - " << __FILE__ << " : " << __LINE__;
                             throw(std::runtime_error(msg.str()));;
                          };
                       }
                       else {
                          std::ostringstream msg;
                          msg << "Nebo error in " << "Nebo Assignment" << ":\n";
                          msg << "Left-hand side of assignment allocated on ";
                          msg << "unknown device - not on CPU or GPU";
                          msg << "\n";
                          msg << "\t - " << __FILE__ << " : " << __LINE__;
                          throw(std::runtime_error(msg.str()));;
                       };
                    }
#                endif
                 /* NEBO_GPU_TEST */
#             else
                 cpu_assign<Iterator, RhsType>(rhs)
#             endif
              /* __CUDACC__ */;
           }

         private:
          template<typename Iterator, typename RhsType>
           inline void cpu_assign(RhsType rhs) {
#             ifdef FIELD_EXPRESSION_THREADS
                 if(is_thread_parallel()) {
                    thread_parallel_assign<Iterator, RhsType>(rhs);
                 }
                 else { sequential_assign<Iterator, RhsType>(rhs); }
#             else
                 sequential_assign<Iterator, RhsType>(rhs)
#             endif
              /* FIELD_EXPRESSION_THREADS */;
           }

          template<typename Iterator, typename RhsType>
           inline void sequential_assign(RhsType rhs) {
#             ifdef NEBO_REPORT_BACKEND
                 std::cout << "Starting Nebo sequential" << std::endl
#             endif
              /* NEBO_REPORT_BACKEND */;

              typename CalculateValidGhost<Iterator, RhsType, FieldType>::Result
              typedef ValidGhost;

              init<ValidGhost, Shift>().assign(rhs.template init<ValidGhost,
                                                                 Shift>());

#             ifdef NEBO_REPORT_BACKEND
                 std::cout << "Finished Nebo sequential" << std::endl
#             endif
              /* NEBO_REPORT_BACKEND */;
           }

          template<typename ValidGhost, typename Shift>
           inline SeqWalkType init(void) {
              return SeqWalkType(field_.template
                                        resize_ghost_and_shift_and_maintain_interior<ValidGhost,
                                                                                     Shift>());
           }

#         ifdef FIELD_EXPRESSION_THREADS
             template<typename Iterator, typename RhsType>
              inline void thread_parallel_assign(RhsType rhs) {
#                ifdef NEBO_REPORT_BACKEND
                    std::cout << "Starting Nebo thread parallel" << std::endl
#                endif
                 /* NEBO_REPORT_BACKEND */;

                 Semaphore semaphore(0);

                 const int thread_count = get_soft_thread_count();

                 typename CalculateValidGhost<Iterator, RhsType, FieldType>::
                 Result typedef ValidGhost;

                 typename RhsType::ResizeType typedef RhsResizeType;

                 const structured::IntVec split = nebo_find_partition(field_.template
                                                                             resize_ghost<ValidGhost>().window_with_ghost().extent(),
                                                                      thread_count);

                 const int max = nebo_partition_count(split);

                 ResizeType new_lhs = resize<ValidGhost>();

                 RhsResizeType new_rhs = rhs.template resize<ValidGhost>();

                 structured::IntVec location = structured::IntVec(0, 0, 0);

                 for(int count = 0; count < max; count++) {
                    ThreadPoolFIFO::self().schedule(boost::bind(&ResizeType::
                                                                template assign<RhsResizeType>,
                                                                new_lhs,
                                                                new_rhs,
                                                                split,
                                                                location,
                                                                &semaphore));

                    location = nebo_next_partition(location, split);
                 };

                 for(int ii = 0; ii < max; ii++) { semaphore.wait(); };

#                ifdef NEBO_REPORT_BACKEND
                    std::cout << "Finished Nebo thread parallel" << std::endl
#                endif
                 /* NEBO_REPORT_BACKEND */;
              }

             template<typename ValidGhost>
              inline ResizeType resize(void) {
                 return ResizeType(field_.template
                                          resize_ghost_and_maintain_interior<ValidGhost>());
              }
#         endif
          /* FIELD_EXPRESSION_THREADS */

#         ifdef __CUDACC__
             template<typename Iterator, typename RhsType>
              inline void gpu_assign(RhsType rhs) {
#                ifdef NEBO_REPORT_BACKEND
                    std::cout << "Starting Nebo CUDA" << std::endl
#                endif
                 /* NEBO_REPORT_BACKEND */;

                 typename CalculateValidGhost<Iterator, RhsType, FieldType>::
                 Result typedef ValidGhost;

                 typename RhsType::GPUWalkType typedef RhsGPUWalkType;

                 int extent0 = field_.window_with_ghost().extent(0);

                 int extent1 = field_.window_with_ghost().extent(1);

                 int blockDim = 16;

                 int gDimX = extent0 / blockDim + ((extent0 % blockDim) > 0 ? 1
                                                   : 0);

                 int gDimY = extent1 / blockDim + ((extent1 % blockDim) > 0 ? 1
                                                   : 0);

                 dim3 dimBlock(blockDim, blockDim);

                 dim3 dimGrid(gDimX, gDimY);

                 gpu_assign_kernel<GPUWalkType, RhsGPUWalkType><<<dimGrid,
                                                                  dimBlock,
                                                                  0,
                                                                  field_.get_stream()>>>(gpu_init<ValidGhost,
                                                                                                  Shift>(),
                                                                                         rhs.template
                                                                                             gpu_init<ValidGhost,
                                                                                                      Shift>(gpu_device_index()));

#                ifdef NEBO_REPORT_BACKEND
                    std::cout << "Finished Nebo CUDA" << std::endl
#                endif
                 /* NEBO_REPORT_BACKEND */;
              }

             inline bool cpu_ready(void) const {
                return field_.memory_device_type() == LOCAL_RAM;
             }

             inline bool gpu_ready(void) const {
                return field_.memory_device_type() == EXTERNAL_CUDA_GPU;
             }

             inline int gpu_device_index(void) const {
                return field_.device_index();
             }

             template<typename ValidGhost, typename Shift>
              inline GPUWalkType gpu_init(void) {
                 return GPUWalkType(field_.template
                                           resize_ghost_and_shift_and_maintain_interior<ValidGhost,
                                                                                        Shift>());
              }

#            ifdef NEBO_GPU_TEST
                template<typename Iterator, typename RhsType>
                 inline void gpu_test_assign(RhsType rhs) {
#                   ifdef NEBO_REPORT_BACKEND
                       std::cout << "Starting Nebo CUDA with Nebo copying" <<
                       std::endl
#                   endif
                    /* NEBO_REPORT_BACKEND */;

                    rhs.gpu_prep(0);

                    if(LOCAL_RAM == field_.memory_device_type()) {
                       FieldType gpu_field(field_.window_with_ghost(),
                                           NULL,
                                           structured::InternalStorage,
                                           EXTERNAL_CUDA_GPU,
                                           0);

                       NeboField<Initial, FieldType> gpu_lhs(gpu_field);

                       gpu_lhs.template gpu_assign<Iterator, RhsType>(rhs);

                       ema::cuda::CUDADeviceInterface & CDI = ema::cuda::
                       CUDADeviceInterface::self();

                       CDI.memcpy_from(field_.field_values(),
                                       gpu_field.field_values(EXTERNAL_CUDA_GPU,
                                                              0),
                                       field_.allocated_bytes(),
                                       0);
                    }
                    else { gpu_assign<Iterator, RhsType>(rhs); };

#                   ifdef NEBO_REPORT_BACKEND
                       std::cout << "Finished Nebo CUDA with Nebo copying" <<
                       std::endl
#                   endif
                    /* NEBO_REPORT_BACKEND */;
                 }
#            endif
             /* NEBO_GPU_TEST */
#         endif
          /* __CUDACC__ */

          FieldType field_;
      };
#     ifdef FIELD_EXPRESSION_THREADS
         template<typename FieldType>
          struct NeboField<Resize, FieldType> {
            public:
             FieldType typedef field_type;

             typename field_type::memory_window typedef MemoryWindow;

             NeboField<SeqWalk, FieldType> typedef SeqWalkType;

             NeboField(FieldType f)
             : field_(f)
             {}

             structured::IndexTriplet<0, 0, 0> typedef Shift;

#            ifdef FIELD_EXPRESSION_THREADS
                template<typename RhsType>
                 inline void assign(RhsType const & rhs,
                                    structured::IntVec const & split,
                                    structured::IntVec const & location,
                                    Semaphore * semaphore) {
                    init<Shift>(split, location).assign(rhs.template init<Shift>(split,
                                                                                 location));

                    semaphore->post();
                 }
#            endif
             /* FIELD_EXPRESSION_THREADS */

            private:
             template<typename Shift>
              inline SeqWalkType init(structured::IntVec const & split,
                                      structured::IntVec const & location) {
                 return SeqWalkType(FieldType(field_.window_with_ghost().refine(split,
                                                                                location),
                                              field_).template
                                                      shift_and_maintain_interior<Shift>());
              }

             FieldType field_;
         }
#     endif
      /* FIELD_EXPRESSION_THREADS */;
      template<typename FieldType>
       struct NeboField<SeqWalk, FieldType> {
         public:
          FieldType typedef field_type;

          typename field_type::memory_window typedef MemoryWindow;

          typename FieldType::value_type typedef AtomicType;

          NeboField(FieldType f)
          : iter_(f.begin()), end_(f.end())
          {}

          template<typename RhsType>
           inline void assign(RhsType rhs) {
              while(!at_end()) { ref() = rhs.eval(); next(); rhs.next(); };
           }

         private:
          inline void next(void) { iter_++; }

          inline bool at_end(void) const { return iter_ == end_; }

          inline bool has_length(void) const { return true; }

          inline AtomicType & ref(void) { return *iter_; }

          typename FieldType::iterator iter_;

          typename FieldType::iterator const end_;
      };
#     ifdef __CUDACC__
         template<typename FieldType>
          struct NeboField<GPUWalk, FieldType> {
            public:
             FieldType typedef field_type;

             typename field_type::memory_window typedef MemoryWindow;

             typename field_type::value_type typedef AtomicType;

             NeboField(FieldType f)
             : current_(f.field_values(EXTERNAL_CUDA_GPU, f.device_index()) + f.window_with_ghost().offset(0)
               + f.window_with_ghost().glob_dim(0) * (f.window_with_ghost().offset(1)
                                                      + (f.window_with_ghost().glob_dim(1)
                                                         * f.window_with_ghost().offset(2)))),
               location_(0),
               valid_(false),
               xLength_(f.window_with_ghost().glob_dim(0)),
               xExtent_(f.window_with_ghost().extent(0)),
               yExtent_(f.window_with_ghost().extent(1)),
               zExtent_(f.window_with_ghost().extent(2)),
               step_(xLength_ * f.window_with_ghost().glob_dim(1))
             {}

             template<typename RhsType>
              __device__ inline void assign(RhsType rhs) {
                 const int ii = blockIdx.x * blockDim.x + threadIdx.x;

                 const int jj = blockIdx.y * blockDim.y + threadIdx.y;

                 start(ii, jj);

                 rhs.start(ii, jj);

                 while(!at_end()) {
                    if(valid()) { ref() = rhs.eval(); };

                    next();

                    rhs.next();
                 };
              }

            private:
             __device__ inline bool valid(void) { return valid_; }

             __device__ inline void start(int x, int y) {
                valid_ = (x < xExtent_ && x >= 0 && y < yExtent_ && y >= 0);

                if(valid()) { location_ = 0; current_ += x + y * xLength_; };
             }

             __device__ inline void next(void) {
                current_ += step_;

                location_++;
             }

             __device__ inline bool at_end(void) {
                return location_ >= zExtent_;
             }

             __device__ inline AtomicType & ref(void) { return *current_; }

             AtomicType * current_;

             int location_;

             int valid_;

             int const xLength_;

             int const xExtent_;

             int const yExtent_;

             int const zExtent_;

             int const step_;
         }
#     endif
      /* __CUDACC__ */;
   } /* SpatialOps */

#endif
/* NEBO_LHS_H */
