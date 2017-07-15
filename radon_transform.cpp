//1)Carga da imagem
//2)Geração do sinogramaa
//3)Backpropagation
#include "spdlog.h"
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
		console->log(spdlog::level::trace, "Calling GenerateData()");
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
	return 0;
}