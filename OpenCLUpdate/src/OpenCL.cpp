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

void OpenCL::initCL()
{
  int err;                            // error code returned from api calls

  // Connect to a compute device
  //
  int gpu = 1;
  err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &m_deviceID, NULL);
  if (err != CL_SUCCESS)
  {
      std::cerr<<"Error: Failed to create a device group!\n";
      exit( EXIT_FAILURE);
  }

  // Create a compute context
  //
  m_context = clCreateContext(0, 1, &m_deviceID, NULL, NULL, &err);
  if (!m_context)
  {
      std::cerr<<"Error: Failed to create a compute context!\n";
      exit( EXIT_FAILURE);
  }

  // Create a command commands
  //
  m_commands = clCreateCommandQueue(m_context, m_deviceID, 0, &err);
  if (!m_commands)
  {
      std::cerr<<"Error: Failed to create a command commands!\n";
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
      exit(EXIT_FAILURE);
  }
}


void OpenCL::getError(int _err)
{
  switch(_err)
  {
  case CL_INVALID_VALUE : std::cerr<<"CL_INVALID_VALUE"<<"\n"; break;
  case CL_INVALID_DEVICE_TYPE : std::cerr<<"CL_INVALID_DEVICE_TYPE"<<"\n"; break;
  case CL_INVALID_PLATFORM : std::cerr<<"CL_INVALID_PLATFORM"<<"\n"; break;
  case CL_INVALID_DEVICE : std::cerr<<"CL_INVALID_DEVICE"<<"\n"; break;
  case CL_INVALID_CONTEXT : std::cerr<<"CL_INVALID_CONTEXT"<<"\n"; break;
  case CL_INVALID_QUEUE_PROPERTIES : std::cerr<<"CL_INVALID_QUEUE_PROPERTIES"<<"\n"; break;
  case CL_INVALID_COMMAND_QUEUE : std::cerr<<"CL_INVALID_COMMAND_QUEUE"<<"\n"; break;
  case CL_INVALID_HOST_PTR : std::cerr<<"CL_INVALID_HOST_PTR"<<"\n"; break;
  case CL_INVALID_MEM_OBJECT : std::cerr<<"CL_INVALID_MEM_OBJECT"<<"\n"; break;
  case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR : std::cerr<<"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"<<"\n"; break;
  case CL_INVALID_IMAGE_SIZE : std::cerr<<"CL_INVALID_IMAGE_SIZE"<<"\n"; break;
  case CL_INVALID_SAMPLER : std::cerr<<"CL_INVALID_SAMPLER"<<"\n"; break;
  case CL_INVALID_BINARY : std::cerr<<"CL_INVALID_BINARY"<<"\n"; break;
  case CL_INVALID_BUILD_OPTIONS : std::cerr<<"CL_INVALID_BUILD_OPTIONS"<<"\n"; break;
  case CL_INVALID_PROGRAM : std::cerr<<"CL_INVALID_PROGRAM"<<"\n"; break;
  case CL_INVALID_PROGRAM_EXECUTABLE : std::cerr<<"CL_INVALID_PROGRAM_EXECUTABLE"<<"\n"; break;
  case CL_INVALID_KERNEL_NAME : std::cerr<<"CL_INVALID_KERNEL_NAME"<<"\n"; break;
  case CL_INVALID_KERNEL_DEFINITION : std::cerr<<"CL_INVALID_KERNEL_DEFINITION"<<"\n"; break;
  case CL_INVALID_KERNEL : std::cerr<<"CL_INVALID_KERNEL"<<"\n"; break;
  case CL_INVALID_ARG_INDEX : std::cerr<<"CL_INVALID_ARG_INDEX"<<"\n"; break;
  case CL_INVALID_ARG_VALUE : std::cerr<<"CL_INVALID_ARG_VALUE"<<"\n"; break;
  case CL_INVALID_ARG_SIZE : std::cerr<<"CL_INVALID_ARG_SIZE"<<"\n"; break;
  case CL_INVALID_KERNEL_ARGS : std::cerr<<"CL_INVALID_KERNEL_ARGS"<<"\n"; break;
  case CL_INVALID_WORK_DIMENSION : std::cerr<<"CL_INVALID_WORK_DIMENSION"<<"\n"; break;
  case CL_INVALID_WORK_GROUP_SIZE : std::cerr<<"CL_INVALID_WORK_GROUP_SIZE"<<"\n"; break;
  case CL_INVALID_WORK_ITEM_SIZE : std::cerr<<"CL_INVALID_WORK_ITEM_SIZE"<<"\n"; break;
  case CL_INVALID_GLOBAL_OFFSET : std::cerr<<"CL_INVALID_GLOBAL_OFFSET"<<"\n"; break;
  case CL_INVALID_EVENT_WAIT_LIST : std::cerr<<"CL_INVALID_EVENT_WAIT_LIST"<<"\n"; break;
  case CL_INVALID_EVENT : std::cerr<<"CL_INVALID_EVENT"<<"\n"; break;
  case CL_INVALID_OPERATION : std::cerr<<"CL_INVALID_OPERATION"<<"\n"; break;
  case CL_INVALID_GL_OBJECT : std::cerr<<"CL_INVALID_GL_OBJECT"<<"\n"; break;
  case CL_INVALID_BUFFER_SIZE : std::cerr<<"CL_INVALID_BUFFER_SIZE"<<"\n"; break;
  case CL_INVALID_MIP_LEVEL : std::cerr<<"CL_INVALID_MIP_LEVEL"<<"\n"; break;
  case CL_INVALID_GLOBAL_WORK_SIZE : std::cerr<<"CL_INVALID_GLOBAL_WORK_SIZE"<<"\n"; break;
  case CL_INVALID_PROPERTY : std::cerr<<"CL_INVALID_PROPERTY"<<"\n"; break;
  case CL_INVALID_IMAGE_DESCRIPTOR : std::cerr<<"CL_INVALID_IMAGE_DESCRIPTOR"<<"\n"; break;
  case CL_INVALID_COMPILER_OPTIONS : std::cerr<<"CL_INVALID_COMPILER_OPTIONS"<<"\n"; break;
  case CL_INVALID_LINKER_OPTIONS : std::cerr<<"CL_INVALID_LINKER_OPTIONS"<<"\n"; break;
  case CL_INVALID_DEVICE_PARTITION_COUNT : std::cerr<<"CL_INVALID_DEVICE_PARTITION_COUNT"<<"\n"; break;
  default : std::cerr<<"not sure what this one is "<<_err<<"\n"; break;
  }
}


