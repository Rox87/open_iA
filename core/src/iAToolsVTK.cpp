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
#include "iAToolsVTK.h"

#include <vtkImageData.h>

#include "iAConnector.h"
#include "iAITKIO.h"
#include "iAVtkDraw.h"



#include <vtkObjectFactory.h>

// declared in iAVtkDraw.h
vtkStandardNewMacro(iAvtkImageData);


void DeepCopy(vtkSmartPointer<vtkImageData> input, vtkSmartPointer<vtkImageData> output)
{
	output->DeepCopy(input);
}

vtkSmartPointer<vtkImageData> AllocateImage(int vtkType, int dimensions[3], double spacing[3], int numComponents)
{
	vtkSmartPointer<vtkImageData> result = vtkSmartPointer<vtkImageData>::New();
	result->SetDimensions(dimensions);
	result->AllocateScalars(vtkType, numComponents);
	result->SetSpacing(spacing);
	return result;
}


vtkSmartPointer<vtkImageData> AllocateImage(int vtkType, int dimensions[3], double spacing[3])
{
	return AllocateImage(vtkType, dimensions, spacing, 1);
}

vtkSmartPointer<vtkImageData> AllocateImage(vtkSmartPointer<vtkImageData> img)
{
	return AllocateImage(img->GetScalarType(), img->GetDimensions(), img->GetSpacing());
}


void StoreImage(vtkSmartPointer<vtkImageData> image, QString const & filename, bool useCompression)
{
	iAConnector con;
	con.SetImage(image);
	iAITKIO::ScalarPixelType pixelType = con.GetITKScalarPixelType();
	iAITKIO::writeFile(filename, con.GetITKImage(), pixelType, useCompression);
}

vtkSmartPointer<vtkImageData> ReadImage(QString const & filename, bool releaseFlag)
{
	iAConnector con;
	iAITKIO::ScalarPixelType pixelType;
	iAITKIO::ImagePointer img = iAITKIO::readFile(filename, pixelType, releaseFlag);
	con.SetImage(img);
	return con.GetVTKImage();
}

#include <vtkImageWriter.h>

#include <vtkBMPWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkPNGWriter.h>
#include <vtkTIFFWriter.h>

#include <QFileInfo>

void WriteSingleSliceImage(QString const & filename, vtkImageData* imageData)
{
	QFileInfo fi(filename);
	vtkSmartPointer<vtkImageWriter> writer;
	if ((QString::compare(fi.suffix(), "TIF", Qt::CaseInsensitive) == 0) || (QString::compare(fi.suffix(), "TIFF", Qt::CaseInsensitive) == 0)) {
		writer = vtkSmartPointer<vtkTIFFWriter>::New();
	}
	else if (QString::compare(fi.suffix(), "PNG", Qt::CaseInsensitive) == 0) {
		writer = vtkSmartPointer<vtkPNGWriter>::New();
	}
	else if ((QString::compare(fi.suffix(), "JPG", Qt::CaseInsensitive) == 0) || (QString::compare(fi.suffix(), "JPEG", Qt::CaseInsensitive) == 0)) {
		vtkJPEGWriter *writer = vtkJPEGWriter::New();
	}
	else if (QString::compare(fi.suffix(), "BMP", Qt::CaseInsensitive) == 0) {
		vtkBMPWriter *writer = vtkBMPWriter::New();
	}
	writer->SetFileName(filename.toLatin1());
	writer->SetInputData(imageData);
	writer->Write();
}


bool isVtkIntegerType(int type)
{
	return type == VTK_CHAR || type == VTK_UNSIGNED_CHAR ||
		type == VTK_SHORT || type == VTK_UNSIGNED_SHORT ||
		type == VTK_INT || type == VTK_UNSIGNED_INT ||
		type == VTK_LONG || type == VTK_UNSIGNED_LONG ||
		type == VTK_LONG_LONG || type == VTK_UNSIGNED_LONG_LONG;
}

int MapVTKTypeStringToInt(QString const & vtkTypeName)
{
	if (vtkTypeName == "VTK_VOID")           return VTK_VOID;
	if (vtkTypeName == "VTK_BIT")            return VTK_BIT;
	if (vtkTypeName == "VTK_UNSIGNED_CHAR")  return VTK_UNSIGNED_CHAR;
	if (vtkTypeName == "VTK_SIGNED_CHAR")    return VTK_SIGNED_CHAR;
	if (vtkTypeName == "VTK_CHAR")           return VTK_CHAR;
	if (vtkTypeName == "VTK_UNSIGNED_SHORT") return VTK_UNSIGNED_SHORT;
	if (vtkTypeName == "VTK_SHORT")          return VTK_SHORT;
	if (vtkTypeName == "VTK_UNSIGNED_INT")   return VTK_UNSIGNED_INT;
	if (vtkTypeName == "VTK_INT")            return VTK_INT;
	if (vtkTypeName == "VTK_UNSIGNED_LONG")  return VTK_UNSIGNED_LONG;
	if (vtkTypeName == "VTK_LONG")           return VTK_LONG;
	if (vtkTypeName == "VTK_FLOAT")          return VTK_FLOAT;
	if (vtkTypeName == "VTK_DOUBLE")         return VTK_DOUBLE;
	if (vtkTypeName == "VTK_ID_TYPE")        return VTK_ID_TYPE;
	return -1;
}
