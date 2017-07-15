//1)Carga da imagem
//2)Geração do sinogramaa
//3)Backpropagation
#include "spdlog.h"
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkRescaleIntensityImageFilter.h>
////Meu console
auto console = spdlog::stdout_color_mt("console");

typedef itk::Image<unsigned char, 2> ImageType;
typedef itk::Image<float, 2> FloatImageType;
typedef itk::ImageFileReader<ImageType> FileSourceType;
typedef itk::RescaleIntensityImageFilter<ImageType, FloatImageType> NormalizeFilterType;

FloatImageType::Pointer CreateEmptyITKImage(int width, int height);

int main(int argc, char** argv)
{
	FileSourceType::Pointer imageReader = FileSourceType::New();
	imageReader->SetFileName("C:\\src\\radon_transform\\black_dot_1_channel.png");//Carga
	NormalizeFilterType::Pointer normalizeFilter = NormalizeFilterType::New();
	normalizeFilter->SetOutputMaximum(1.0);
	normalizeFilter->SetOutputMinimum(0.0);
	normalizeFilter->SetInput(imageReader->GetOutput());//Normalização + conversão pra float
	normalizeFilter->Update();
	FloatImageType::Pointer originalImage = normalizeFilter->GetOutput();//To com a imagem normalizada.
	FloatImageType::Pointer sinogramImage = CreateEmptyITKImage(ceil(static_cast<double>(originalImage->GetLargestPossibleRegion().GetSize()[0])*sqrt(2)),
																ceil(static_cast<double>(originalImage->GetLargestPossibleRegion().GetSize()[0])*sqrt(2)));
	return 0;
}

FloatImageType::Pointer CreateEmptyITKImage(int width, int height)
{
	FloatImageType::Pointer image = FloatImageType::New();
	FloatImageType::RegionType region;
	FloatImageType::IndexType start;
	start[0] = 0;
	start[1] = 0;
	FloatImageType::SizeType size;
	size[0] = width;
	size[1] = height;
	region.SetSize(size);
	region.SetIndex(start);
	image->SetRegions(region);
	image->Allocate();
	memset(
		image->GetBufferPointer(),
		0,
		width * height * sizeof(float));
	return image;
}