#include "OpenCL.h"
#include <fstream>
#include <string>
#include <iostream>


OpenCL::OpenCL()
{
  initCL();
}

OpenCL::OpenCL(std::string _kernel)
{
  initCL();
  loadKernelSource(_kernel);
}

OpenCL::~OpenCL()
{
  clReleaseProgram(m_program);
  clReleaseKernel(m_kernel);
  clReleaseCommandQueue(m_commands);
  clReleaseContext(m_context);
}


void OpenCL::printCLInfo()
{
  int NAMESIZE=1024;
  cl_uint plat_count;
  clGetPlatformIDs (0, NULL, &plat_count);
  cl_platform_id *platforms =  new cl_platform_id [plat_count];
  clGetPlatformIDs(plat_count, platforms, NULL);
  for (cl_uint i = 0; i < plat_count; ++i)
  {
    char buf[NAMESIZE];
    clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buf), buf, NULL);
    std::cerr<<"platform "<<i<<": vendor "<<buf<<"\n";

    // get number of devices in platform
    cl_uint dev_count;
    clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &dev_count);

    cl_device_id *devices = new cl_device_id [dev_count];

    // get list of devices in platform
    clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, dev_count, devices, NULL);

    // iterate over devices
    for (cl_uint j = 0; j < dev_count; ++j)
    {
      char buf[NAMESIZE];
      clGetDeviceInfo(devices[j], CL_DEVICE_NAME,  sizeof(buf), buf, NULL);
      std::cerr<< "device " << j <<" : "<< buf<<"\n";
    }

    delete [] devices;
  }
  delete [] platforms;

}


void OpenCL::initCL()
{
  int err;                            // error code returned from api calls

  // Connect to a compute device
  //
  int gpu = 0;
  err = clGetDeviceIDs(0, CL_DEVICE_TYPE_GPU, 1, &m_deviceID, NULL);
  if (err != CL_SUCCESS)
  {
      std::cerr<<"Error: Failed to create a device group!\n";
      printError(err);
      exit( EXIT_FAILURE);
  }

  // Create a compute context
  //
  m_context = clCreateContext(0, 1, &m_deviceID, NULL, NULL, &err);
  if (!m_context)
  {
      std::cerr<<"Error: Failed to create a compute context!\n";
      printError(err);
      exit( EXIT_FAILURE);
  }

  // Create a command commands
  //
  m_commands = clCreateCommandQueue(m_context, m_deviceID, 0, &err);
  if (!m_commands)
  {
      std::cerr<<"Error: Failed to create a command commands!\n";
      printError(err);
      exit( EXIT_FAILURE);
  }

}


void OpenCL::loadKernelSource(const std::string &_fname)
{
  std::ifstream kernelSource(_fname.c_str());
  std::string *source;
  if (!kernelSource.is_open())
  {
   std::cerr<<"File not found "<<_fname.c_str()<<"\n";
   exit(EXIT_FAILURE);
  }
  // now read in the data
  source = new std::string((std::istreambuf_iterator<char>(kernelSource)), std::istreambuf_iterator<char>());
  kernelSource.close();
  *source+="\0";

  const char* data=source->c_str();
  int err;                            // error code returned from api calls

  m_program = clCreateProgramWithSource(m_context, 1, (const char **) & data, NULL, &err);
  if (!m_program)
  {
      std::cerr<<"Error: Failed to create compute program!\n";
      printError(err);
      exit (EXIT_FAILURE);
  }
  delete source;

  // Build the program executable
  //
  err = clBuildProgram(m_program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS)
  {
    // Determine the size of the log
     size_t logSize;
     printError(err);
     clGetProgramBuildInfo(m_program, m_deviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);

     // Allocate memory for the log
     char *log = (char *) new char[logSize];

     // Get the log
     clGetProgramBuildInfo(m_program, m_deviceID, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);

    std::cerr<<log<<"\n";
    delete [] log;
    exit(EXIT_FAILURE);
  }
}

void OpenCL::createKernel(const std::string &_name)
{
  int err;
  m_kernel = clCreateKernel(m_program, _name.c_str(), &err);
  if (!m_kernel || err != CL_SUCCESS)
  {
      std::cerr<<"Error: Failed to create compute kernel!\n";
      printError(err);
      exit(EXIT_FAILURE);
  }
}

// modified from http://wiki.tiker.net/OpenCLHowTo

void OpenCL::printError(int _err) const
{
  switch(_err)
  {
  case CL_SUCCESS: std::cerr<< "success\n";
  case CL_DEVICE_NOT_FOUND: std::cerr<< "device not found\n";
  case CL_DEVICE_NOT_AVAILABLE: std::cerr<< "device not available\n";
#if !(defined(CL_PLATFORM_NVIDIA) && CL_PLATFORM_NVIDIA == 0x3001)
  case CL_COMPILER_NOT_AVAILABLE: std::cerr<< "device compiler not available\n";
#endif
  case CL_MEM_OBJECT_ALLOCATION_FAILURE: std::cerr<< "mem object allocation failure\n";
  case CL_OUT_OF_RESOURCES: std::cerr<< "out of resources\n";
  case CL_OUT_OF_HOST_MEMORY: std::cerr<< "out of host memory\n";
  case CL_PROFILING_INFO_NOT_AVAILABLE: std::cerr<< "profiling info not available\n";
  case CL_MEM_COPY_OVERLAP: std::cerr<< "mem copy overlap\n";
  case CL_IMAGE_FORMAT_MISMATCH: std::cerr<< "image format mismatch\n";
  case CL_IMAGE_FORMAT_NOT_SUPPORTED: std::cerr<< "image format not supported\n";
  case CL_BUILD_PROGRAM_FAILURE: std::cerr<< "build program failure\n";
  case CL_MAP_FAILURE: std::cerr<< "map failure\n";

  case CL_INVALID_VALUE: std::cerr<< "invalid value\n";
  case CL_INVALID_DEVICE_TYPE: std::cerr<< "invalid device type\n";
  case CL_INVALID_PLATFORM: std::cerr<< "invalid platform\n";
  case CL_INVALID_DEVICE: std::cerr<< "invalid device\n";
  case CL_INVALID_CONTEXT: std::cerr<< "invalid context\n";
  case CL_INVALID_QUEUE_PROPERTIES: std::cerr<< "invalid queue properties\n";
  case CL_INVALID_COMMAND_QUEUE: std::cerr<< "invalid command queue\n";
  case CL_INVALID_HOST_PTR: std::cerr<< "invalid host ptr\n";
  case CL_INVALID_MEM_OBJECT: std::cerr<< "invalid mem object\n";
  case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: std::cerr<< "invalid image format descriptor\n";
  case CL_INVALID_IMAGE_SIZE: std::cerr<< "invalid image size\n";
  case CL_INVALID_SAMPLER: std::cerr<< "invalid sampler\n";
  case CL_INVALID_BINARY: std::cerr<< "invalid binary\n";
  case CL_INVALID_BUILD_OPTIONS: std::cerr<< "invalid build options\n";
  case CL_INVALID_PROGRAM: std::cerr<< "invalid program\n";
  case CL_INVALID_PROGRAM_EXECUTABLE: std::cerr<< "invalid program executable\n";
  case CL_INVALID_KERNEL_NAME: std::cerr<< "invalid kernel name\n";
  case CL_INVALID_KERNEL_DEFINITION: std::cerr<< "invalid kernel definition\n";
  case CL_INVALID_KERNEL: std::cerr<< "invalid kernel\n";
  case CL_INVALID_ARG_INDEX: std::cerr<< "invalid arg index\n";
  case CL_INVALID_ARG_VALUE: std::cerr<< "invalid arg value\n";
  case CL_INVALID_ARG_SIZE: std::cerr<< "invalid arg size\n";
  case CL_INVALID_KERNEL_ARGS: std::cerr<< "invalid kernel args\n";
  case CL_INVALID_WORK_DIMENSION: std::cerr<< "invalid work dimension\n";
  case CL_INVALID_WORK_GROUP_SIZE: std::cerr<< "invalid work group size\n";
  case CL_INVALID_WORK_ITEM_SIZE: std::cerr<< "invalid work item size\n";
  case CL_INVALID_GLOBAL_OFFSET: std::cerr<< "invalid global offset\n";
  case CL_INVALID_EVENT_WAIT_LIST: std::cerr<< "invalid event wait list\n";
  case CL_INVALID_EVENT: std::cerr<< "invalid event\n";
  case CL_INVALID_OPERATION: std::cerr<< "invalid operation\n";
  case CL_INVALID_GL_OBJECT: std::cerr<< "invalid gl object\n";
  case CL_INVALID_BUFFER_SIZE: std::cerr<< "invalid buffer size\n";
  case CL_INVALID_MIP_LEVEL: std::cerr<< "invalid mip level\n";

#if defined(cl_khr_gl_sharing) && (cl_khr_gl_sharing >= 1)
  case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR: std::cerr<< "invalid gl sharegroup reference number\n";
#endif

#ifdef CL_VERSION_1_1
  case CL_MISALIGNED_SUB_BUFFER_OFFSET: std::cerr<< "misaligned sub-buffer offset\n";
  case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: std::cerr<< "exec status error for events in wait list\n";
  case CL_INVALID_GLOBAL_WORK_SIZE: std::cerr<< "invalid global work size\n";
#endif

  default : std::cerr<<"not sure what this one is "<<_err<<"\n"; break;
  }
}


