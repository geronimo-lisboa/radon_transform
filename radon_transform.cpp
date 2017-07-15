//1)Carga da imagem
//2)Geração do sinogramaa
//3)Backpropagation
#include "spdlog.h"
#include <itkResampleImageFilter.hxx>
#include <itkImage.h>
#include <itkImageToImageFilter.h>
#include <itkImageFileReader.h>
////Meu console
auto console = spdlog::stdout_color_mt("console");
////A classe
namespace itk
{
	template<class TImage>
	class myRadonTransform : public ImageToImageFilter<TImage, TImage>
	{
	public:
		typedef myRadonTransform Self;
		typedef ImageToImageFilter<TImage, TImage> Superclass;
		typedef SmartPointer<Self> Pointer;
		itkNewMacro(Self);
		itkTypeMacro(myRadonTransform, ImageToImageFilter);
	protected:
		myRadonTransform(){}
		~myRadonTransform(){}
		virtual void GenerateData();
	private:
		myRadonTransform(const Self&);
		void operator=(const Self&);
	};
}
////A implementação da classe
namespace itk
{
	template <class TImage>
	void myRadonTransform<TImage>::GenerateData()
	{
		Image<unsigned char, 2>::Pointer teste;
		typename TImage::ConstPointer input = this->GetInput();
		typename TImage::Pointer output = this->GetOutput();
		//A imagem de entrada tem que ser 2d e quadrada
		if (input->GetImageDimension() != 2)
			throw itk::ExceptionObject("Input tem que ser obrigatoriamente de dimensão = 2");
		if (input->GetLargestPossibleRegion().GetSize()[0]!= input->GetLargestPossibleRegion().GetSize()[0])
			throw itk::ExceptionObject("Lados tem que ser iguais");
		const int L = input->GetLargestPossibleRegion().GetSize()[0];
		//O tamanho correto do output é l * 2^(1/2)
		typename TImage::SizeType newOutputSize;
		newOutputSize[0] = ceil( (double)L * sqrt(2));
		newOutputSize[1] = ceil( (double)L * sqrt(2));
		typename TImage::RegionType newOutputRegion;
		newOutputRegion.SetSize(newOutputSize);
		output->SetRegions(newOutputRegion);
		output->Allocate();
		//Agora o output tá com o tamanho correto
		console->log(spdlog::level::info, "Calling GenerateData()");
	}

}

typedef itk::Image<unsigned char, 2> ImageType;
typedef itk::ImageFileReader<ImageType> FileSourceType;
typedef itk::myRadonTransform<ImageType> RadonFilterType;

int main(int argc, char** argv)
{
	FileSourceType::Pointer imageReader = FileSourceType::New();
	imageReader->SetFileName("C:\\src\\radon_transform\\black_dot_1_channel.png");
	RadonFilterType::Pointer radonFilter = RadonFilterType::New();
	radonFilter->SetInput(imageReader->GetOutput());
	radonFilter->Update();
	radonFilter->GetOutput()->Print(std::cout);
	return 0;
}