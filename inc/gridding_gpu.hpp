#ifndef GRIDDING_GPU_HPP_
#define GRIDDING_GPU_HPP_

#include "griddingFunctions.hpp"
//#include "cuda_utils.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_THREADS_PER_SECTOR 1 //16x16
#define MAX_SECTOR_WIDTH 12 // 8x8x8 + Kernel with Width 5 -> 12x12x12
#define MAX_SECTOR_DIM 1728 // 12x12x12


#if __CUDA_ARCH__ < 200
	#define THREAD_BLOCK_SIZE 128 
#else
	#define THREAD_BLOCK_SIZE 256
#endif


enum GriddingOutput
{
	CONVOLUTION,
	FFT,
	DEAPODIZATION
};

enum FFTShiftDir
{
	FORWARD,
	INVERSE
};

void gridding3D_gpu(CufftType**	data,			// kspace output data array 
					int			data_count,		// data count, samples per trajectory
					int			n_coils,		// number of coils 
					DType*		crds,			//
					DType*		imdata,			// image input data array
					int			imdata_count,	//			
					int			grid_width,		//
					DType*		kernel,			//
					int			kernel_count,	//
					int			kernel_width,	//
					int*		sectors,		//
					int			sector_count,	//
					int*		sector_centers,	//
					int			sector_width,	//
					int			im_width,		//
					DType		osr,			// oversampling ratio
					const GriddingOutput gridding_out);

void gridding3D_gpu_adj(DType*		data,			// kspace input data array
						int			data_count,		// data count, samples per trajectory
						int			n_coils,		// number of coils 
						DType*		crds,			// 
						CufftType**	imdata,			// image output data array
						int			imdata_count,	//			
						int			grid_width,		//
						DType*		kernel,			//
						int			kernel_count,	//
						int			kernel_width,	//
						int*		sectors,		//
						int			sector_count,	//
						int*		sector_centers,	//
						int			sector_width,	//
						int			im_width,		//
						DType		osr,			// oversampling ratio
						const GriddingOutput gridding_out);

struct GriddingInfo 
{
	int data_count;
	int kernel_width; 
	int kernel_widthSquared;
	DType kernel_widthInvSquared;
	int kernel_count;
	DType kernel_radius;

	int grid_width;		
	int grid_width_dim;  
	int grid_width_offset;
	DType grid_width_inv;

	int im_width;
	int im_width_dim;
	int im_width_offset;

	DType osr;
	
	int sector_count;
	int sector_width;
	int sector_dim;
	int sector_pad_width;
	int sector_pad_max;
	int sector_offset;

	DType radiusSquared;
	DType dist_multiplier;
};


#endif  // GRIDDING_GPU_HPP_*/
