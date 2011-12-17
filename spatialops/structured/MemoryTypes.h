/*
 * DeviceTypes.h
 *
 *  Created on: Dec 15, 2011
 *      Author: Devin Robison
 */

#ifndef MEMORYTYPES_H_
#define MEMORYTYPES_H_
namespace SpatialOps{

  /**
   *  \enum MemoryType
   *  \brief Enumerates the supported memory management strategies.
   */
   enum MemoryType {
     LOCAL_RAM,           ///< Locally allocated memory: standard CPU attached RAM.
     LOCAL_VECTOR_INST,   ///< Future use, locally allocated memory: SSE type instruction.
     EXTERNAL_CUDA_GPU,   ///< Externally allocated memory: NVIDIA device via CUDA.
     EXTERNAL_OPENCL_GPU, ///< Future use, externally allocated memory: Generic GPU device via OpenCL.
     EXTERNAL_INTEL_MIK,  ///< Future use, externally allocated memory: Intel MIK chip
     UNKNOWN              ///< error state
   };

  /**
   *  \class DeviceTypeTools
   *  \brief Provide descriptions
   */
  namespace DeviceTypeTools {

  inline std::string get_memory_type_description( MemoryType m )
    {
      switch ( m ){
      case LOCAL_RAM:
        return std::string("Locally allocated, generic system RAM.");
        break;
      case LOCAL_VECTOR_INST:
        return std::string("Locally allocated, vector device or CPU extension.");
        break;
      case EXTERNAL_CUDA_GPU:
        return std::string("Externally allocated, CUDA GPU device.");
        break;
      case EXTERNAL_OPENCL_GPU:
        return std::string("Externally allocated, OpenCL GPU device.");
        break;
      case EXTERNAL_INTEL_MIK:
        return std::string("Externally allocated, intel MIK device.");
        break;
      default:
        return std::string("Unknown or Invalid");
        break;
      }
    }

  } // namespace DeviceTypeTools
} // namespace SpatialOps



#endif /* DEVICETYPES_H_ */