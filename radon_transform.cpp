//1)Carga da imagem
//2)Geração do sinogramaa
//3)Backpropagation
#include <Windows.h>
#include "spdlog.h"
#include <itkLinearInterpolateImageFunction.h>
#include <itkRigid2DTransform.h>
#include <itkResampleImageFilter.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkImageFileWriter.h>
////Meu console
auto console = spdlog::stdout_color_mt("console");

typedef itk::Image<unsigned char, 2> ImageType;
typedef itk::Image<float, 2> FloatImageType;
typedef itk::Image<float, 1> RayAccumulatorImage;
typedef itk::ImageFileReader<ImageType> FileSourceType;
typedef itk::RescaleIntensityImageFilter<ImageType, FloatImageType> NormalizeFilterType;
typedef itk::ConstantPadImageFilter<FloatImageType, FloatImageType> PadFilterType;

FloatImageType::Pointer CreateEmptyITKImage(int width, int height);
FloatImageType::Pointer RotateImage(FloatImageType::Pointer input, float angleInDegrees);
RayAccumulatorImage::Pointer MakeRayAccumulator(FloatImageType::Pointer input);
RayAccumulatorImage::Pointer Project(FloatImageType::Pointer input);

//O método Project() é um dos glutões de processamento com 42 % e dentro dele é o InterpolateImageFunction.

//A outra parte é dominada pelo ResampleImageFilter.

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
	long T0 = GetCurrentTime();
	////A imagem está com lados = sqrt(2) * L, pra conseguir caber a maior projeção possivel da radon, na diagonal.
	const int tamanhoDaLinha = ceil(static_cast<double>(originalImage->GetLargestPossibleRegion().GetSize()[0])*sqrt(2));
	//const int tamanhoDaLinha = originalImage->GetLargestPossibleRegion().GetSize()[0];
	const int angulos = 360;
	FloatImageType::Pointer sinogramImage = CreateEmptyITKImage(angulos,tamanhoDaLinha);
	////Padding da imagem original
	PadFilterType::Pointer padFilter = PadFilterType::New();
	padFilter->SetInput(originalImage);
	PadFilterType::SizeType padValue;
	const int L = tamanhoDaLinha;
	padValue[0] = (L - originalImage->GetLargestPossibleRegion().GetSize()[0]) / 2;
	padValue[1] = (L - originalImage->GetLargestPossibleRegion().GetSize()[0]) / 2;
	padFilter->SetPadLowerBound(padValue);
	padFilter->SetPadUpperBound(padValue);
	padFilter->SetConstant(0);
	padFilter->Update();
	//O sinograma
	for (int i = 0; i < 360; i++)
	{
		FloatImageType::Pointer rotatedImage = RotateImage(padFilter->GetOutput(), i);
		RayAccumulatorImage::Pointer projection = Project(rotatedImage);
		for (unsigned int y = 0; y<projection->GetLargestPossibleRegion().GetSize()[0]; y++)
		{
			RayAccumulatorImage::IndexType srcPos;
			srcPos[0] = y;
			FloatImageType::IndexType destPos;
			destPos[0] = i;
			destPos[1] = y;
			sinogramImage->SetPixel(destPos, projection->GetPixel(srcPos));
		}
	}
	long T1 = GetCurrentTime();
	console->info("Tempo gasto = {0:d};ms",(T1-T0));
	//Salva
	itk::ImageFileWriter<FloatImageType>::Pointer writer = itk::ImageFileWriter<FloatImageType>::New();
	writer->SetInput(sinogramImage);
	writer->SetFileName("c:\\src\\sino.mha");
	writer->Write();

	return 0;
}

RayAccumulatorImage::Pointer MakeRayAccumulator(FloatImageType::Pointer input)
{
	RayAccumulatorImage::Pointer result = RayAccumulatorImage::New();
	RayAccumulatorImage::RegionType region;
	RayAccumulatorImage::RegionType::SizeType size;
	size[0] = input->GetLargestPossibleRegion().GetSize()[0];
	region.SetSize(size);
	result->SetSpacing(input->GetSpacing()[0]);
	result->SetOrigin(0.0);
	result->SetRegions(region);
	result->Allocate();
	return result;
}


RayAccumulatorImage::Pointer Project(FloatImageType::Pointer input)
{
	RayAccumulatorImage::Pointer result = MakeRayAccumulator(input);
	itk::LinearInterpolateImageFunction<FloatImageType, double>::Pointer interpolator = itk::LinearInterpolateImageFunction<FloatImageType, double>::New();
	interpolator->SetInputImage(input);
	typedef itk::LinearInterpolateImageFunction<FloatImageType, double>::PointType PointType;

	//O raio vai de baixo pra cima, da esquerda pra direita.
	for (unsigned int x = 0; x<input->GetLargestPossibleRegion().GetSize()[0]; x++)
	{
		PointType rayOrigin;
		rayOrigin[0] = input->GetOrigin()[0] + x * input->GetSpacing()[0];
		rayOrigin[1] = input->GetOrigin()[1];
		float sum = 0.0;
		for (unsigned int y = 0; y<input->GetLargestPossibleRegion().GetSize()[1]; y++)
		{
			PointType currentPoint;
			currentPoint[0] = rayOrigin[0];
			currentPoint[1] = rayOrigin[1] + y * input->GetSpacing()[1];
			float evaluation = interpolator->Evaluate(currentPoint);
			sum = sum + evaluation;
		}
		RayAccumulatorImage::IndexType resultPos;
		resultPos[0] = x;
		result->SetPixel(resultPos, sum);
		//cout << sum << endl;
	}
	return result;
}


FloatImageType::Pointer RotateImage(FloatImageType::Pointer input, float angleInDegrees)
{
	typedef itk::ResampleImageFilter<FloatImageType, FloatImageType> RotatorFilterType;
	RotatorFilterType::Pointer rotator = RotatorFilterType::New();
	rotator->SetInput(input);
	typedef itk::Rigid2DTransform<double> TransformType; //Typedef do transform
	TransformType::Pointer transform = TransformType::New();
	transform->SetAngleInDegrees(angleInDegrees);
	TransformType::InputPointType centerOfRotation;
	centerOfRotation[0] = input->GetLargestPossibleRegion().GetSize()[0] * input->GetSpacing()[0] / 2 + input->GetOrigin()[0];
	centerOfRotation[1] = input->GetLargestPossibleRegion().GetSize()[1] * input->GetSpacing()[1] / 2 + input->GetOrigin()[0];
	transform->SetCenter(centerOfRotation);
	TransformType::OutputVectorType _t;
	_t[0] = 0;
	_t[1] = 0;
	transform->SetTranslation(_t);
	rotator->SetTransform(transform);
	typedef itk::LinearInterpolateImageFunction<FloatImageType> LinearInterpolatorType;
	LinearInterpolatorType::Pointer li = LinearInterpolatorType::New();
	rotator->SetInterpolator(li);
	rotator->SetDefaultPixelValue(0);
	rotator->SetOutputSpacing(input->GetSpacing());
	rotator->SetOutputOrigin(input->GetOrigin());
	rotator->SetSize(input->GetLargestPossibleRegion().GetSize());
	rotator->Update();
	return rotator->GetOutput();
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