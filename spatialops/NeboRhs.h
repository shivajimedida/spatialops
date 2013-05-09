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

#ifndef NEBO_RHS_H
#  define NEBO_RHS_H

   namespace SpatialOps {
      template<typename CurrentMode, typename FieldType>
       struct NeboScalar;
      template<typename FieldType>
       struct NeboScalar<Initial, FieldType> {
         public:
          FieldType typedef field_type;

          typename field_type::memory_window typedef MemoryWindow;

          typename FieldType::value_type typedef AtomicType;

          NeboScalar<SeqWalk, FieldType> typedef SeqWalkType;

#         ifdef FIELD_EXPRESSION_THREADS
             NeboScalar<Resize, FieldType> typedef ResizeType;
#         endif
          /* FIELD_EXPRESSION_THREADS */

#         ifdef __CUDACC__
             NeboScalar<GPUWalk, FieldType> typedef GPUWalkType;
#         endif
          /* __CUDACC__ */

          NeboScalar<Reduction, FieldType> typedef ReductionType;

          structured::InfiniteGhostData typedef PossibleValidGhost;

          NeboScalar(AtomicType const v)
          : value_(v)
          {}

          template<typename ValidGhost, typename Shift>
           inline SeqWalkType init(void) const { return SeqWalkType(value_); }

#         ifdef FIELD_EXPRESSION_THREADS
             template<typename ValidGhost>
              inline ResizeType resize(void) const { return ResizeType(value_); }
#         endif
          /* FIELD_EXPRESSION_THREADS */

#         ifdef __CUDACC__
             inline bool gpu_ready(int const deviceIndex) const { return true; }

             template<typename ValidGhost, typename Shift>
              inline GPUWalkType gpu_init(int const deviceIndex) const {
                 return GPUWalkType(value_);
              }

#            ifdef NEBO_GPU_TEST
                inline void gpu_prep(int const deviceIndex) const {}
#            endif
             /* NEBO_GPU_TEST */
#         endif
          /* __CUDACC__ */

          template<typename ValidGhost, typename Shift>
           inline ReductionType reduce_init(void) const {
              return ReductionType(value_);
           }

         private:
          AtomicType const value_;
      };
#     ifdef FIELD_EXPRESSION_THREADS
         template<typename FieldType>
          struct NeboScalar<Resize, FieldType> {
            public:
             FieldType typedef field_type;

             typename field_type::memory_window typedef MemoryWindow;

             typename FieldType::value_type typedef AtomicType;

             NeboScalar<SeqWalk, FieldType> typedef SeqWalkType;

             NeboScalar(AtomicType const value)
             : value_(value)
             {}

             template<typename Shift>
              inline SeqWalkType init(structured::IntVec const & split,
                                      structured::IntVec const & location) const {
                 return SeqWalkType(value_);
              }

            private:
             AtomicType const value_;
         }
#     endif
      /* FIELD_EXPRESSION_THREADS */;
      template<typename FieldType>
       struct NeboScalar<SeqWalk, FieldType> {
         public:
          FieldType typedef field_type;

          typename field_type::memory_window typedef MemoryWindow;

          typename FieldType::value_type typedef AtomicType;

          NeboScalar(AtomicType const value)
          : value_(value)
          {}

          inline void next(void) {}

          inline AtomicType eval(void) const { return value_; }

         private:
          AtomicType const value_;
      };
#     ifdef __CUDACC__
         template<typename FieldType>
          struct NeboScalar<GPUWalk, FieldType> {
            public:
             FieldType typedef field_type;

             typename field_type::memory_window typedef MemoryWindow;

             typename field_type::value_type typedef AtomicType;

             NeboScalar(AtomicType const value)
             : value_(value)
             {}

             __device__ inline void start(int x, int y) {}

             __device__ inline void next(void) {}

             __device__ inline AtomicType eval(void) const { return value_; }

            private:
             AtomicType const value_;
         }
#     endif
      /* __CUDACC__ */;
      template<typename FieldType>
       struct NeboScalar<Reduction, FieldType> {
         public:
          FieldType typedef field_type;

          typename field_type::memory_window typedef MemoryWindow;

          typename FieldType::value_type typedef AtomicType;

          NeboScalar(AtomicType const value)
          : value_(value)
          {}

          inline void next(void) {}

          inline bool at_end(void) const { return false; }

          inline bool has_length(void) const { return false; }

          inline AtomicType eval(void) const { return value_; }

         private:
          AtomicType const value_;
      };

      template<typename CurrentMode, typename FieldType>
       struct NeboBoolean;
      template<typename FieldType>
       struct NeboBoolean<Initial, FieldType> {
         public:
          FieldType typedef field_type;

          typename field_type::memory_window typedef MemoryWindow;

          NeboBoolean<SeqWalk, FieldType> typedef SeqWalkType;

#         ifdef FIELD_EXPRESSION_THREADS
             NeboBoolean<Resize, FieldType> typedef ResizeType;
#         endif
          /* FIELD_EXPRESSION_THREADS */

#         ifdef __CUDACC__
             NeboBoolean<GPUWalk, FieldType> typedef GPUWalkType;
#         endif
          /* __CUDACC__ */

          NeboBoolean<Reduction, FieldType> typedef ReductionType;

          structured::InfiniteGhostData typedef PossibleValidGhost;

          NeboBoolean(bool const v)
          : value_(v)
          {}

          template<typename ValidGhost, typename Shift>
           inline SeqWalkType init(void) const { return SeqWalkType(value_); }

#         ifdef FIELD_EXPRESSION_THREADS
             template<typename ValidGhost>
              inline ResizeType resize(void) const { return ResizeType(value_); }
#         endif
          /* FIELD_EXPRESSION_THREADS */

#         ifdef __CUDACC__
             inline bool gpu_ready(int const deviceIndex) const { return true; }

             template<typename ValidGhost, typename Shift>
              inline GPUWalkType gpu_init(int const deviceIndex) const {
                 return GPUWalkType(value_);
              }

#            ifdef NEBO_GPU_TEST
                inline void gpu_prep(int const deviceIndex) const {}
#            endif
             /* NEBO_GPU_TEST */
#         endif
          /* __CUDACC__ */

          template<typename ValidGhost, typename Shift>
           inline ReductionType reduce_init(void) const {
              return ReductionType(value_);
           }

         private:
          bool const value_;
      };
#     ifdef FIELD_EXPRESSION_THREADS
         template<typename FieldType>
          struct NeboBoolean<Resize, FieldType> {
            public:
             FieldType typedef field_type;

             typename field_type::memory_window typedef MemoryWindow;

             NeboBoolean<SeqWalk, FieldType> typedef SeqWalkType;

             NeboBoolean(bool const value)
             : value_(value)
             {}

             template<typename Shift>
              inline SeqWalkType init(structured::IntVec const & split,
                                      structured::IntVec const & location) const {
                 return SeqWalkType(value_);
              }

            private:
             bool const value_;
         }
#     endif
      /* FIELD_EXPRESSION_THREADS */;
      template<typename FieldType>
       struct NeboBoolean<SeqWalk, FieldType> {
         public:
          FieldType typedef field_type;

          typename field_type::memory_window typedef MemoryWindow;

          NeboBoolean(bool const value)
          : value_(value)
          {}

          inline void next(void) {}

          inline bool eval(void) const { return value_; }

         private:
          bool const value_;
      };
#     ifdef __CUDACC__
         template<typename FieldType>
          struct NeboBoolean<GPUWalk, FieldType> {
            public:
             FieldType typedef field_type;

             typename field_type::memory_window typedef MemoryWindow;

             typename field_type::value_type typedef AtomicType;

             NeboBoolean(bool const value)
             : value_(value)
             {}

             __device__ inline void start(int x, int y) {}

             __device__ inline void next(void) {}

             __device__ inline AtomicType eval(void) const { return value_; }

            private:
             bool const value_;
         }
#     endif
      /* __CUDACC__ */;
      template<typename FieldType>
       struct NeboBoolean<Reduction, FieldType> {
         public:
          FieldType typedef field_type;

          typename field_type::memory_window typedef MemoryWindow;

          NeboBoolean(bool const value)
          : value_(value)
          {}

          inline void next(void) {}

          inline bool at_end(void) const { return false; }

          inline bool has_length(void) const { return false; }

          inline bool eval(void) const { return value_; }

         private:
          bool const value_;
      };

      template<typename CurrentMode, typename FieldType>
       struct NeboConstField;
      template<typename FieldType>
       struct NeboConstField<Initial, FieldType> {
         public:
          FieldType typedef field_type;

          typename field_type::memory_window typedef MemoryWindow;

          NeboConstField<SeqWalk, FieldType> typedef SeqWalkType;

#         ifdef FIELD_EXPRESSION_THREADS
             NeboConstField<Resize, FieldType> typedef ResizeType;
#         endif
          /* FIELD_EXPRESSION_THREADS */

#         ifdef __CUDACC__
             NeboConstField<GPUWalk, FieldType> typedef GPUWalkType;
#         endif
          /* __CUDACC__ */

          NeboConstField<Reduction, FieldType> typedef ReductionType;

          typename structured::GhostFromField<FieldType>::result typedef
          PossibleValidGhost;

          NeboConstField(FieldType const & f)
          : field_(f)
          {}

          template<typename ValidGhost, typename Shift>
           inline SeqWalkType init(void) const {
              return SeqWalkType(field_.template resize_ghost_and_shift<ValidGhost,
                                                                        Shift>());
           }

#         ifdef FIELD_EXPRESSION_THREADS
             template<typename ValidGhost>
              inline ResizeType resize(void) const {
                 return ResizeType(field_.template resize_ghost<ValidGhost>());
              }
#         endif
          /* FIELD_EXPRESSION_THREADS */

#         ifdef __CUDACC__
             inline bool gpu_ready(int const deviceIndex) const {
                return field_.find_consumer(EXTERNAL_CUDA_GPU, deviceIndex);
             }

             template<typename ValidGhost, typename Shift>
              inline GPUWalkType gpu_init(int const deviceIndex) const {
                 return GPUWalkType(deviceIndex,
                                    field_.template resize_ghost_and_shift<ValidGhost,
                                                                           Shift>());
              }

#            ifdef NEBO_GPU_TEST
                inline void gpu_prep(int const deviceIndex) const {
                   const_cast<FieldType *>(&field_)->add_consumer(EXTERNAL_CUDA_GPU,
                                                                  deviceIndex);
                }
#            endif
             /* NEBO_GPU_TEST */
#         endif
          /* __CUDACC__ */

          template<typename ValidGhost, typename Shift>
           inline ReductionType reduce_init(void) const {
              return ReductionType(field_.template resize_ghost_and_shift<ValidGhost,
                                                                          Shift>());
           }

         private:
          FieldType const field_;
      };
#     ifdef FIELD_EXPRESSION_THREADS
         template<typename FieldType>
          struct NeboConstField<Resize, FieldType> {
            public:
             FieldType typedef field_type;

             typename field_type::memory_window typedef MemoryWindow;

             NeboConstField<SeqWalk, FieldType> typedef SeqWalkType;

             NeboConstField(FieldType const & f)
             : field_(f)
             {}

             template<typename Shift>
              inline SeqWalkType init(structured::IntVec const & split,
                                      structured::IntVec const & location) const {
                 return SeqWalkType(FieldType(field_.window_with_ghost().refine(split,
                                                                                location),
                                              field_).template shift<Shift>());
              }

            private:
             FieldType const field_;
         }
#     endif
      /* FIELD_EXPRESSION_THREADS */;
      template<typename FieldType>
       struct NeboConstField<SeqWalk, FieldType> {
         public:
          FieldType typedef field_type;

          typename field_type::memory_window typedef MemoryWindow;

          typename FieldType::value_type typedef AtomicType;

          NeboConstField(FieldType const & f)
          : iter_(f.begin())
          {}

          inline void next(void) { iter_++; }

          inline AtomicType eval(void) const { return *iter_; }

         private:
          typename FieldType::const_iterator iter_;
      };
#     ifdef __CUDACC__
         template<typename FieldType>
          struct NeboConstField<GPUWalk, FieldType> {
            public:
             FieldType typedef field_type;

             typename field_type::memory_window typedef MemoryWindow;

             typename field_type::value_type typedef AtomicType;

             NeboConstField(int const deviceIndex, FieldType const & f)
             : current_(f.field_values(EXTERNAL_CUDA_GPU, deviceIndex) + f.window_with_ghost().offset(0)
               + f.window_with_ghost().glob_dim(0) * (f.window_with_ghost().offset(1)
                                                      + (f.window_with_ghost().glob_dim(1)
                                                         * f.window_with_ghost().offset(2)))),
               xLength_(f.window_with_ghost().glob_dim(0)),
               step_(xLength_ * f.window_with_ghost().glob_dim(1))
             {}

             __device__ inline void start(int x, int y) {
                current_ += x + y * xLength_;
             }

             __device__ inline void next(void) { current_ += step_; }

             __device__ inline AtomicType eval(void) const { return *current_; }

            private:
             AtomicType const * current_;

             int const xLength_;

             int const step_;
         }
#     endif
      /* __CUDACC__ */;
      template<typename FieldType>
       struct NeboConstField<Reduction, FieldType> {
         public:
          FieldType typedef field_type;

          typename field_type::memory_window typedef MemoryWindow;

          typename FieldType::value_type typedef AtomicType;

          NeboConstField(FieldType const & f)
          : iter_(f.begin()), end_(f.end())
          {}

          inline void next(void) { iter_++; }

          inline bool at_end(void) const { return iter_ == end_; }

          inline bool has_length(void) const { return true; }

          inline AtomicType eval(void) const { return *iter_; }

         private:
          typename FieldType::const_iterator iter_;

          typename FieldType::const_iterator const end_;
      };
   } /* SpatialOps */

#endif
/* NEBO_RHS_H */
