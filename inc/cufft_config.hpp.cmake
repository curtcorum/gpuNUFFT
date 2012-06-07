#ifndef CUFFT_CONFIG_H
#define CUFFT_CONFIG_H
#include "config.hpp"
#include "cufft.h"

#ifdef GPU_DOUBLE_PREC
	static cufftType_t CufftTransformType = CUFFT_Z2Z;
	
	static cufftResult (__stdcall*pt2CufftExec)(cufftHandle, cufftDoubleComplex*, cufftDoubleComplex*,int) = &cufftExecZ2Z;
#else
	static cufftType_t CufftTransformType = CUFFT_C2C;
	
	static cufftResult (__stdcall*pt2CufftExec)(cufftHandle, cufftComplex*, cufftComplex*,int) = &cufftExecC2C;
#endif

#endif // CUFFT_CONFIG_H