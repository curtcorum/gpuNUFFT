#include <limits.h>

#include "gtest/gtest.h"

#include "gpuNUFFT_operator_factory.hpp"

#define epsilon 0.01f

#define get3DC2lin(_x,_y,_z,_width) ((_x) + (_width) * ( (_y) + (_z) * (_width)))

TEST(TestDeapo, 2D)
{
  int grid_width = 256;
  DType grid_width_inv = (DType)1.0 / grid_width;
  int kernel_width = 3;
  DType osr = 1.0;
  DType beta = (DType)BETA(kernel_width,osr);

  DType test = calculateDeapodizationValue(0, grid_width_inv, kernel_width, beta);
  std::cout << "Deapo at 0: " << test << std::endl;

  test = calculateDeapodizationValue(128, grid_width_inv, kernel_width, beta);
  std::cout << "Deapo at 128: " << test << std::endl;

  test = calculateDeapodizationValue(-128, grid_width_inv, kernel_width, beta);
  std::cout << "Deapo at -128: " << test << std::endl;

  test = calculateDeapodizationValue(256, grid_width_inv, kernel_width, beta);
  std::cout << "Deapo at 256: " << test << std::endl;

  int max_ind = (int)(grid_width * osr);
  test = calculateDeapodizationValue(max_ind, grid_width_inv, kernel_width, beta);
  std::cout << "Deapo at " << max_ind << " : " << test << std::endl;
}

TEST(TestDeapo, 2Dind)
{
  int x = 0;
  int y = 0;
  int t = 0;

  int grid_width = 256;
  
  IndType3 width_offset; 
  width_offset.x = (int)(floor(grid_width / 2.0));
  width_offset.y = (int)(floor(grid_width / 2.0));

  DType3 grid_width_inv; 
  grid_width_inv.x = (DType)1.0 / grid_width;
  grid_width_inv.y = (DType)1.0 / grid_width;

  int kernel_width = 3;
  DType osr = 1.0;
  DType beta = (DType)BETA(kernel_width,osr);
  DType norm_val = calculateDeapodizationValue(0, grid_width_inv.x, kernel_width, beta);

  getCoordsFromIndex2D(t, &x, &y, grid_width,grid_width);

  DType deapo = calculateDeapodizationAt2D(x,y,width_offset,grid_width_inv,kernel_width,beta,norm_val*norm_val);
  std::cout << "Deapo at 0/0: " << deapo << std::endl;

  deapo = calculateDeapodizationAt2D(128,128,width_offset,grid_width_inv,kernel_width,beta,norm_val*norm_val);
  std::cout << "Deapo at 128/128: " << deapo << std::endl;

  
  getCoordsFromIndex2D(grid_width*grid_width-1, &x, &y, grid_width,grid_width);
  deapo = calculateDeapodizationAt2D(x,y,width_offset,grid_width_inv,kernel_width,beta,norm_val*norm_val);
  std::cout << "Deapo at " << x << "/" << y << " : " << deapo << std::endl;
}

TEST(TestGPUGpuNUFFTDeapo,KernelCall1Sector)
{
	int kernel_width = 3;
	//oversampling ratio
	float osr = 1.75;//DEFAULT_OVERSAMPLING_RATIO;

	//Image
	int im_width = 64;

	//Data
	int data_entries = 1;
    DType2* data = (DType2*) calloc(data_entries,sizeof(DType2)); //2* re + im
	data[0].x = 1;//Re
	data[0].y = 0;//Im

	//Coords
	//Scaled between -0.5 and 0.5
	//in triplets (x,y,z)
    DType* coords = (DType*) calloc(3*data_entries,sizeof(DType));//3* x,y,z
	coords[0] = 0; //should result in 7,7,7 center
	coords[1] = 0;
	coords[2] = 0;

	//sectors of data, count and start indices
	int sector_width = 8;
	
	gpuNUFFT::Array<DType> kSpaceData;
    kSpaceData.data = coords;
    kSpaceData.dim.length = data_entries;

	gpuNUFFT::Dimensions imgDims;
	imgDims.width = im_width;
	imgDims.height = im_width;
	imgDims.depth = im_width;

    gpuNUFFT::GpuNUFFTOperatorFactory factory; gpuNUFFT::GpuNUFFTOperator *gpuNUFFTOp = factory.createGpuNUFFTOperator(kSpaceData,kernel_width,sector_width,osr,imgDims);

	gpuNUFFT::Array<DType2> dataArray;
	dataArray.data = data;
	dataArray.dim.length = data_entries;

	gpuNUFFT::Array<CufftType> gdataArray;
	
	gdataArray = gpuNUFFTOp->performGpuNUFFTAdj(dataArray,gpuNUFFT::DEAPODIZATION);
	//Output Grid
	CufftType* gdata = gdataArray.data;

	if (DEBUG) printf("test %f \n",gdata[4].x);

	/*for (int y = 0; y < im_width; y++)
	{
		for (int x = 0; x < im_width; x++)
			printf(" %.3f ", gdata[get3DC2lin(x,y,16,im_width)].x,epsilon);
		printf(" \n ");
	}*/

	int index = get3DC2lin(5,5,5,im_width);
	if (DEBUG) printf("index to test %d\n",index);
	EXPECT_NEAR(0.00637,gdata[get3DC2lin(10,0,16,im_width)].x,epsilon);
	
	free(data);
	free(coords);
	free(gdata);

	delete gpuNUFFTOp;
}

TEST(TestGPUGpuNUFFTDeapo,KernelCall1Sector2Coils)
{
	int kernel_width = 3;
	//oversampling ratio
	float osr = 2.0f;//DEFAULT_OVERSAMPLING_RATIO;

	//Image
	int im_width = 32;

	//Data
	int data_entries = 1;
	int n_coils = 2;

    DType2* data = (DType2*) calloc(data_entries*n_coils,sizeof(DType2)); //2* re + im
	data[0].x = 1;//Re
	data[0].y = 0;//Im
	data[1].x = 1;//Re
	data[1].y = 0;//Im
	
	//Coords, same per coil
	//Scaled between -0.5 and 0.5
	//in triplets (x,y,z)
    DType* coords = (DType*) calloc(3*data_entries,sizeof(DType));//3* x,y,z
	coords[0] = 0; 
	coords[1] = 0;
	coords[2] = 0;

	//sectors of data, count and start indices
	int sector_width = 8;
	
	gpuNUFFT::Array<DType> kSpaceData;
    kSpaceData.data = coords;
    kSpaceData.dim.length = data_entries;

	gpuNUFFT::Dimensions imgDims;
	imgDims.width = im_width;
	imgDims.height = im_width;
	imgDims.depth = im_width;

    gpuNUFFT::GpuNUFFTOperatorFactory factory; gpuNUFFT::GpuNUFFTOperator *gpuNUFFTOp = factory.createGpuNUFFTOperator(kSpaceData,kernel_width,sector_width,osr,imgDims);

	gpuNUFFT::Array<DType2> dataArray;
	dataArray.data = data;
	dataArray.dim.length = data_entries;
	dataArray.dim.channels = n_coils;

	gpuNUFFT::Array<CufftType> gdataArray;
	
	gdataArray = gpuNUFFTOp->performGpuNUFFTAdj(dataArray,gpuNUFFT::DEAPODIZATION);
	//Output Grid
	CufftType* gdata = gdataArray.data;
	
	if (DEBUG) printf("test %f \n",gdata[4].x);
	int index = get3DC2lin(5,5,5,im_width);
	if (DEBUG) printf("index to test %d\n",index);

	int coil_offset = 1 * gpuNUFFTOp->getImageDims().count();
	if (DEBUG) printf("grid_size: %d\n",gpuNUFFTOp->getImageDims().count());
	
	/*for (int j=0; j<im_width; j++)
	{
		for (int i=0; i<im_width; i++)
			printf("A: %.2f B: %.2f",gdata[get3DC2lin(i,j,16,im_width)].x,gdata[coil_offset+get3DC2lin(i,j,16,im_width)].x);
		printf("\n");
	}*/
	EXPECT_NEAR(gdata[get3DC2lin(0,0,16,im_width)].x,0.016846,epsilon);
	//EXPECT_NEAR(gdata[coil_offset + get3DC2lin(0,0,16,im_width)].x,0.00928f,epsilon);
	EXPECT_NEAR(gdata[get3DC2lin(10,0,16,im_width)].x,gdata[coil_offset + get3DC2lin(10,0,16,im_width)].x,epsilon);
	EXPECT_NEAR(gdata[get3DC2lin(0,5,16,im_width)].x,gdata[coil_offset + get3DC2lin(0,5,16,im_width)].x,epsilon);
	EXPECT_NEAR(gdata[get3DC2lin(4,4,16,im_width)].x,gdata[coil_offset + get3DC2lin(4,4,16,im_width)].x,epsilon);
	EXPECT_NEAR(gdata[get3DC2lin(15,14,16,im_width)].x,gdata[coil_offset + get3DC2lin(15,14,16,im_width)].x,epsilon);
	
	free(data);
	free(coords);
	free(gdata);

	delete gpuNUFFTOp;

	EXPECT_EQ(1, 1);
}
