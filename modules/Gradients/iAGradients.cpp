/*************************************  open_iA  ************************************ *
* **********  A tool for scientific visualisation and 3D image processing  ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2017  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan,            *
*                          J. Weissenböck, Artem & Alexander Amirkhanov, B. Fröhler   *
* *********************************************************************************** *
* This program is free software: you can redistribute it and/or modify it under the   *
* terms of the GNU General Public License as published by the Free Software           *
* Foundation, either version 3 of the License, or (at your option) any later version. *
*                                                                                     *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY     *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A     *
* PARTICULAR PURPOSE.  See the GNU General Public License for more details.           *
*                                                                                     *
* You should have received a copy of the GNU General Public License along with this   *
* program.  If not, see http://www.gnu.org/licenses/                                  *
* *********************************************************************************** *
* Contact: FH OÖ Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstraße 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#include "pch.h"
#include "iAGradients.h"
#include "iAConnector.h"
#include "iAProgress.h"
#include "iATypedCallHelper.h"

#include <itkCastImageFilter.h>
#include <itkDerivativeImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkImageIOBase.h>
#include <itkHigerOrderAccurateGradient/itkHigherOrderAccurateDerivativeImageFilter.h>

#include <vtkImageData.h>

#include <QLocale>

template<class T> 
int derivative_template( unsigned int o, unsigned int d, iAProgress* p, iAConnector* image )
{
	typedef itk::Image< T, 3 >   InputImageType;
	typedef itk::Image< float, 3 >   RealImageType;

	typedef itk::CastImageFilter< InputImageType, RealImageType> CastToRealFilterType;
	typename CastToRealFilterType::Pointer toReal = CastToRealFilterType::New();
	toReal->SetInput( dynamic_cast< InputImageType * >( image->GetITKImage() ) );

	typedef itk::DerivativeImageFilter< RealImageType, RealImageType > DIFType;
	DIFType::Pointer filter = DIFType::New();

	filter->SetOrder( o );
	filter->SetDirection( d );
	filter->SetInput( toReal->GetOutput() );

	p->Observe( filter );

	filter->Update(); 

	image->SetImage(filter->GetOutput());
	image->Modified();

	filter->ReleaseDataFlagOn();

	return EXIT_SUCCESS;
}

template<class T>
int hoa_derivative_template(const HOAccGradientDerrivativeSettings &settings, iAProgress* p, iAConnector* image, T)
{
	typedef itk::Image< T, 3 >   InputImageType;
	InputImageType * inputImg = dynamic_cast<InputImageType *>(image->GetITKImage());

	typedef itk::HigherOrderAccurateDerivativeImageFilter< InputImageType, InputImageType > HOAGDFilter;
	typename HOAGDFilter::Pointer filter = HOAGDFilter::New();

	filter->SetOrder(settings.order);
	filter->SetDirection(settings.direction);
	filter->SetOrderOfAccuracy(settings.orderOfAcc);
	filter->SetInput(inputImg);

	p->Observe(filter);

	filter->Update();

	image->SetImage(filter->GetOutput());
	image->Modified();

	filter->ReleaseDataFlagOn();

	return EXIT_SUCCESS;
}

template<class T> 
int gradient_magnitude_template( iAProgress* p, iAConnector* image  )
{

	typedef itk::Image< T, 3 >   InputImageType;
	typedef itk::Image< float, 3 >   RealImageType;

	typedef itk::GradientMagnitudeImageFilter< InputImageType, InputImageType > GMFType;
	typename GMFType::Pointer filter = GMFType::New();
	filter->SetInput( dynamic_cast< InputImageType * >( image->GetITKImage() ) );

	p->Observe( filter );

	filter->Update(); 
	image->SetImage(filter->GetOutput());
	image->Modified();

	filter->ReleaseDataFlagOn();

	return EXIT_SUCCESS;
}

iAGradients::iAGradients( QString fn, iAGradientType fid, vtkImageData* i, vtkPolyData* p, iALogger* logger, QObject* parent  )
	: iAAlgorithm( fn, i, p, logger, parent ), m_type(fid)
{}


void iAGradients::performWork()
{
	iAConnector::ITKScalarPixelType itkType = getConnector()->GetITKScalarPixelType();
	switch (m_type)
	{
	case DERIVATIVE:
		ITK_TYPED_CALL(derivative_template, itkType,
			order, direction, getItkProgress(), getConnector());
		break;
	case GRADIENT_MAGNITUDE:
		ITK_TYPED_CALL(gradient_magnitude_template, itkType,
			getItkProgress(), getConnector());
		break;
	case HIGHER_ORDER_ACCURATE_DERIVATIVE:
		switch (itkType)
		{
			case itk::ImageIOBase::CHAR:
				hoa_derivative_template(m_HOAGDSettings, getItkProgress(), getConnector(), static_cast<char>(0)); break;
			case itk::ImageIOBase::SHORT:
				hoa_derivative_template(m_HOAGDSettings, getItkProgress(), getConnector(), static_cast<short>(0)); break;
			case itk::ImageIOBase::INT:
				hoa_derivative_template(m_HOAGDSettings, getItkProgress(), getConnector(), static_cast<int>(0)); break;
			case itk::ImageIOBase::LONG:
				hoa_derivative_template(m_HOAGDSettings, getItkProgress(), getConnector(), static_cast<long>(0)); break;
			case itk::ImageIOBase::FLOAT:
				hoa_derivative_template(m_HOAGDSettings, getItkProgress(), getConnector(), static_cast<float>(0)); break;
			case itk::ImageIOBase::DOUBLE:
				hoa_derivative_template(m_HOAGDSettings, getItkProgress(), getConnector(), static_cast<double>(0)); break;
			default:
				throw std::runtime_error("Unsupported pixel type!");
				return;
		}
		break;
	default:
		addMsg(tr("  unknown filter type"));
	}
}